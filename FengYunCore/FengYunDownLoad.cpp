#include "FengYunDownLoad.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <chrono>
#include <thread>
#include <QByteArray>
#include <QFile>
#include <QEventLoop>
#include <QAbstractEventDispatcher>
#include <QTextCodec>
#include "ChapterModel.hpp"
#include <map>
#include <QRegExp>
#include <QTextStream>
#include <QDataStream>
#include <QThreadPool>
#include <QRunnable>
#include <QEvent>
#include <QApplication>
#include <QRegularExpression>
namespace 
{
	class FengYunDownLoadEvent :
		public QEvent {
	public:
		int index;
		FengYunDownLoadEvent(int i):
			QEvent(QEvent::MaxUser),
		    index(i){
		}
		~FengYunDownLoadEvent() {
		}
	};
}

void FengYunDownLoad::customEvent(QEvent * e ) {
	auto * event = dynamic_cast<FengYunDownLoadEvent *>(e);
	if ( event ) {
		this->downLoadAChapter(event->index );
		return;
	}
	QObject::customEvent(e );
}

FengYunDownLoad::FengYunDownLoad(const QString & lp)
    :localPath(lp)
{
	isOnDestory.store(false );
    thread_ = new Thread;
    this->moveToThread(thread_);
    thread_->start();

    connect(this, &FengYunDownLoad::startDownLoad,
        this,&FengYunDownLoad::downLoad,
        Qt::QueuedConnection
        );

	connect(this, &FengYunDownLoad::downLoadHeaders,
		this, &FengYunDownLoad::_downLoadHeaders,
		Qt::QueuedConnection
		);

}

FengYunDownLoad::~FengYunDownLoad(){
	isOnDestory.store( true );
    thread_->quit();
    thread_->wait( 10000 );
    delete thread_;
}

namespace {
    const QString _index_page = u8R"(http://www.baoliny.com/${index}/index.html)";
    const QString _data_page  = u8R"(http://www.baoliny.com/modules/article/packshow.php?id=${index}&type=txtchapter)";

    QString get_out_file(
        const QByteArray & i,
        const QByteArray & d,
		std::vector<ChapterHeaderData> & ans_chapters
        ) {
		ans_chapters.clear();
		auto codec = QTextCodec::codecForName("GBK");

		const QString indexFile = codec->toUnicode( i );
		const QString dataFile = codec->toUnicode( d );

		std::map<QString, ChapterHeaderData >  allItems;

		{//获得章节名
			QRegExp regex(
				u8R"(html">(.*)</a>)" , 
				Qt::CaseSensitive,
				QRegExp::RegExp2
				);

			{
				regex.setMinimal(true);
				int index_ = 0;
				int pos = indexFile.indexOf(u8R"(<td class="ccss">)"  );
				while ((pos = regex.indexIn(indexFile, pos)) != -1) {
					QString cp = regex.cap(1).trimmed() ;
					pos += regex.matchedLength();
					if (allItems.count(cp)) { 
						qDebug().noquote() << cp << "find again";
						continue;
					}
					ChapterHeaderData item;
					item.getChapterIndex() = index_++;
					item.getChapterName() = cp;
					allItems[cp] = item;
					
				}
			}
		}

		{//获得章节网址
			int pos = dataFile.indexOf(u8R"(<div class="main">)");
			 
			ChapterHeaderData item( (ChapterHeaderData::Undefine *)(0) );
			QRegExp regex0(
				u8R"(<td class="odd">(.*)</td>)",
				Qt::CaseSensitive,
				QRegExp::RegExp2
				);
			QRegExp regex1(
				u8R"_(<a href="(.*)">)_",
				Qt::CaseSensitive,
				QRegExp::RegExp2
				);

			regex0.setMinimal(true);
			regex1.setMinimal(true);

			while ((pos = regex0.indexIn(dataFile, pos)) != -1) {
				QString cp = regex0.cap(1).trimmed();
				pos += regex0.matchedLength();

				{
					auto it_ = allItems.find(cp);
					if (it_!=allItems.end()) {
						item = it_->second;
					}
					else {
						const QString cp1 = cp.mid( cp.indexOf(" "),-1 ).trimmed() ;
						auto it_1 = allItems.find(cp1);
						if ( 
							(it_1 != allItems.end())&&
							(it_1->second.getChapterURL().isEmpty()) 
							) {
							item = it_1->second;
						}
						else {
							qDebug().noquote() << "can not find" << cp ;
							continue;
						}
					}
				}
				
				if ( (pos = regex1.indexIn(dataFile, pos)) != -1 ) {
					cp = regex1.cap(1).trimmed();
					pos += regex1.matchedLength();
					item.getChapterURL() = cp;
				}
				else {
					qDebug().noquote() << "can not find" << cp <<"url";
				}
			}

		}

		{//获得书名
			ChapterHeaderData item((ChapterHeaderData::Undefine *)(0));
			ans_chapters.resize( allItems.size() , item );
			for (const auto & ic : allItems) {
				ans_chapters[ ic.second.getChapterIndex() ] = ic.second;
			}
		}

		{
			QRegExp regex(
				u8R"(html">(.*)</a>)",
				Qt::CaseSensitive,
				QRegExp::RegExp2
				);
			regex.setMinimal(true);
			int pos = dataFile.indexOf(u8R"(<caption>)");

			QString ans;
			if ( (pos = regex.indexIn(dataFile, pos) ) != -1 ) {
				ans = regex.cap(1).trimmed();
				return ans;
			}

		}

        return  "";
    }

}

void FengYunDownLoad::downLoad( int i ){

    Q_ASSERT_X(
            QThread::currentThread() == this->thread(),
            "FengYunDownLoad::downLoad",
            "the function must run in this->thread()"
    );

	{
		QFile file(this->localPath + "/_0_header.txt");
		if ( file.exists() && file.open( QIODevice::ReadOnly )  ) {
			FengYunDownLoadHeader headers_;
			headers_.allDatas = std::make_shared< std::vector<ChapterHeaderData> >();
			QTextStream stream( &file );
			stream.setCodec(QTextCodec::codecForName("UTF-8"));
			headers_.bookName = stream.readLine();
			std::list<ChapterHeaderData> temp;
			while ( stream.atEnd() == false ) {
				ChapterHeaderData item;
				item.getChapterIndex() = stream.readLine().toInt();
				item.getChapterName() = stream.readLine();
				item.getChapterURL() = stream.readLine();
				temp.push_back( item );
			}
			headers_.allDatas->reserve( temp.size() );
			for (const auto & i:temp) {
				headers_.allDatas->push_back( i );
			}
			emit downLoadHeaders(headers_);
			return;
		}
	}

    const QString index_ = QString::asprintf("%d",i);
	std::vector<  ChapterHeaderData > allItems;

    {
        QNetworkAccessManager downloadManager;
        QByteArray indexPage;
        QByteArray dataPage;

        {
            QString index_page_url = _index_page;
            index_page_url.replace(u8R"(${index})", index_);
            QNetworkRequest r(index_page_url);
            QNetworkReply * reply = downloadManager.get(r);
            while (reply->isFinished() == false) {
                thread_->eventDispatcher()->processEvents( QEventLoop::EventLoopExec );
            }

            if (reply->error() != QNetworkReply::NoError) {
                delete reply;
                return;
            }

            indexPage = reply->readAll();
            delete reply;
        }

        {
            QString data_page_url = _data_page;
            data_page_url.replace(u8R"(${index})", index_);
            QNetworkRequest r(data_page_url);
            QNetworkReply * reply = downloadManager.get(r);
            while (reply->isFinished() == false) {
                thread_->eventDispatcher()->processEvents( QEventLoop::EventLoopExec );
            }

            if (reply->error() != QNetworkReply::NoError) {
                delete reply;
                return;
            }

            dataPage = reply->readAll();
            delete reply;
        }

        QString bookName = get_out_file( indexPage , dataPage , allItems );

		FengYunDownLoadHeader headers_;
		{
			headers_.allDatas  = std::make_shared< std::vector<ChapterHeaderData> >();
			*headers_.allDatas = std::move( allItems );
			headers_.bookName  = bookName;
			emit downLoadHeaders( headers_ );
		}

		if ( bookName.isEmpty() == false ) {
			QFile file( this->localPath + "/_0_header.txt" );
			file.open( QIODevice::WriteOnly );
			QTextStream dataStream( &file );
			//dataStream.setVersion( QDataStream::Qt_5_5 );
			dataStream.setCodec( QTextCodec::codecForName("UTF-8") );
			dataStream << bookName <<endl ;
			for ( const auto & ic : *(headers_.allDatas) ) {
				dataStream << ic.getChapterIndex()<<endl;
				dataStream << ic.getChapterName()<<endl;
				dataStream << ic.getChapterURL()<<endl;
			}
		}

    }
	

}

namespace 
{
	class FengYunDownLoadSharedCount {
	public:
		std::shared_ptr< char > counter;
		FengYunDownLoadSharedCount() {
			counter = std::make_shared< char >();
		}
		~FengYunDownLoadSharedCount() {
		
		}
		FengYunDownLoadSharedCount copy() {
			return *this;
		}
	};

	class FengYunDownLoadSharedFile {
	public:
		std::shared_ptr<QFile> file;
	};

	class FengYunDownLoadRun : public QRunnable {
	public:
		ChapterHeaderData item;
		FengYunDownLoad * download;
		FengYunDownLoadRun(FengYunDownLoad * d,ChapterHeaderData  i):
			item(i),
			download(d){

		}
        /* 处理字符串 */
		virtual void run() override {
            
			QString data_ =  item.getChapterData();
			QString ans;
			QTextStream stream( &data_ );
			while ( stream.atEnd() == false ) {
				QString line = stream.readLine().trimmed() ;
				if (line.isEmpty()) { continue; }
				
                static const QRegExp reg1=[]() {
                    QRegExp reg1_(
                        QString::fromUtf8(u8R"(【[^】]*.[cＣ]o[mＭ][b]?】)"),
                        Qt::CaseInsensitive,
                        QRegExp::RegExp2
                        );
                    reg1_.setMinimal( true );
                    return reg1_;
                }();
                static const auto reg2 = []() {  QRegularExpression ans_(
                    QString::fromUtf8(u8R"_((</?[bp r]?[bp r]?>))_")
                    ); 
                return ans_;
                }();

                {
                    QString j = line.replace( reg1, "");
                    line = j.replace( reg2, "");
                }
                
                ans.append("    "+ line.trimmed() +"\n" );
			}
			
			item.getChapterData() = ans ;

			item.isSetData().store(true);
			download->downLoadAChapter( item.getChapterIndex() );
			 
		}
	};

	Q_DECLARE_METATYPE(  FengYunDownLoadSharedFile  )
	Q_DECLARE_METATYPE(  FengYunDownLoadSharedCount )

}

void FengYunDownLoad::_downLoadAChapter(QNetworkReply * r) {

	Q_ASSERT_X(
		QThread::currentThread() == this->thread(),
		"FengYunDownLoad::downLoad",
		"the function must run in this->thread()"
		);

	auto f = r->property("@@file").value<FengYunDownLoadSharedFile>();
	auto item = r->property("@@item").value<ChapterHeaderData>();
	auto threadPool = r->property("@@threadPool").value<QThreadPool *>();

	f.file->open(QIODevice::WriteOnly);
	QTextStream stream( f.file.get() );
	stream.setCodec(QTextCodec::codecForName("UTF-8"));

	{
		QByteArray arr = r->readAll();
		item.getChapterData() =
			QTextCodec::codecForName("GBK")->toUnicode( arr );
	}
	delete r;

	stream << item.getChapterData();
	 
	threadPool->start( new FengYunDownLoadRun(this,item) );
}

void FengYunDownLoad::_downLoadHeaders( FengYunDownLoadHeader hs ) {

	Q_ASSERT_X(
		QThread::currentThread() == this->thread(),
		"FengYunDownLoad::downLoad",
		"the function must run in this->thread()"
		);

	QThreadPool threadPool;
	threadPool.setMaxThreadCount( QThread::idealThreadCount()/2+1 );
	QNetworkAccessManager manager;
	FengYunDownLoadSharedCount counter;
	manager.connect(&manager, &QNetworkAccessManager::finished ,
		this, &FengYunDownLoad::_downLoadAChapter ,
		Qt::QueuedConnection
		);

	for (auto i : *(hs.allDatas)) {
	if ( isOnDestory.load() == false ){
		
			auto index_ = QString("%1").arg( i.getChapterIndex(),8,10,QChar('0') );
			QString filePath__ = localPath + "/" + index_ + ".txt";
			QFile * file= new  QFile( filePath__ );
			FengYunDownLoadSharedFile pfile{ std::shared_ptr<QFile>(file) };
			
			if ( file->exists() && file->open( QIODevice::ReadOnly ) ) {
				QTextStream stream( file );
				stream.setCodec( QTextCodec::codecForName("UTF-8") );
				i.getChapterData() = stream.readAll();
				 
				threadPool.start(new FengYunDownLoadRun(this, i ));
			}
			else {

				while ( counter.counter.use_count() > 16 ){
					if ( isOnDestory.load()  ) { return; }
					thread_->eventDispatcher()->processEvents(QEventLoop::EventLoopExec);
				}
			 
				QNetworkRequest req( QUrl(i.getChapterURL()) );
				QNetworkReply * rep = manager.get(req);
				rep->setProperty("@@file", QVariant::fromValue( pfile ) );
				rep->setProperty("@@item", QVariant::fromValue( i ) );
				rep->setProperty("@@threadPool", QVariant::fromValue( &threadPool ));
				rep->setProperty("@@count", QVariant::fromValue( counter.copy() ));
				rep->setParent( &manager );
			}

		}
	}

	while ( counter.counter.use_count() > 1 ){
		if (isOnDestory.load()) { return; }
		thread_->eventDispatcher()->processEvents(QEventLoop::EventLoopExec);
	}

}
 

