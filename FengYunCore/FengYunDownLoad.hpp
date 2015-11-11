#ifndef FENGYUNDOWNLOAD_HPP
#define FENGYUNDOWNLOAD_HPP

#include <memory>
#include <QObject>
#include <QThread>
#include <atomic>
#include <set>
class QNetworkReply;
#include "ChapterModel.hpp"

class FengYunDownLoadHeader {
public:
	std::shared_ptr< std::vector< ChapterHeaderData > > allDatas;
	QString bookName;
};

class FengYunDownLoad :
	public QObject
{
    Q_OBJECT
private:
    QString localPath;
    class Thread : public QThread {
    public:
        void run() override{ exec(); }
    };
	Thread * thread_;
	std::atomic< bool > isOnDestory;
	
public:
    FengYunDownLoad( const QString & /* path */ );
protected:
    ~FengYunDownLoad();
public:
    static std::shared_ptr<FengYunDownLoad> instance(const QString & i){
        return std::shared_ptr<FengYunDownLoad>(
                    new FengYunDownLoad(i),
                    [](FengYunDownLoad * t){delete t;}
                    );
    }
signals:
	void startDownLoad( int );
	void downLoadHeaders( FengYunDownLoadHeader );
	void downLoadAChapter( int );
public slots:
    void downLoad( int ); /* 此函数被设计不在主线程运行 */
private:
    using QObject::moveToThread ;
    using QObject::setParent    ;
    using QObject::deleteLater  ;
private slots:
    void _downLoadHeaders(FengYunDownLoadHeader);
	void _downLoadAChapter( QNetworkReply * );
protected:
	void customEvent(QEvent * event) override;
};

Q_DECLARE_METATYPE( FengYunDownLoadHeader )
Q_DECLARE_METATYPE( ChapterHeaderData )

#endif // FENGYUNDOWNLOAD_HPP



