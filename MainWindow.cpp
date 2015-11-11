#include "MainWindow.hpp"
#include "MainWindowPrivate.hpp"
#include <QApplication>
#include <QDir>
#include <QPainter>
#include <QLinearGradient>

int MainWindow::getChapter( )const {
    return thisp->chapter;
}

void MainWindow::setChapter(int i) {
    thisp->chapter = i ;
    auto data_ = thisp->chapterListView->getModel()->get( i );
	if (data_.isSetData().load()){
		thisp->textView->setText(data_.getChapterData());
	}
}

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{

    background_color=QColor(
        12+(std::rand()&1)-2,
        121+(std::rand()&1)-2,
        121+(std::rand()&1)-2);

    background_color_1=QColor(
        125+(std::rand()&5)-50,
        122+(std::rand()&5)-50,
        126+(std::rand()&5)-50);


    painterTime.start();
    background_color=QColor( 111,111,111 );
    auto * layout_ = new QHBoxLayout;
    this->setLayout(layout_);
    layout_->setMargin(0);
    layout_->setSpacing(0);
    this->setMinimumSize(360,360);

    thisp = new MainWindowPrivate;

    thisp->splitter = new QSplitter;
    layout_->addWidget ( thisp->splitter );
    //thisp->splitter->setHandleWidth( 6 );

    thisp->chapterListView = new ChapterListView;
    thisp->splitter->addWidget(thisp->chapterListView);

    thisp->textView = new TextView;
    thisp->splitter->addWidget( thisp->textView );

	thisp->splitter->setStretchFactor(0, -1);
	thisp->splitter->setStretchFactor(1,  1);

    thisp->chapterListView->connect(
              thisp->chapterListView, &ChapterListView::currentIndexChanged,
              this ,&MainWindow::setChapter
                );
	thisp->splitter->setStyleSheet(
		u8R"(
QSplitter::handle {
    background: transparent   ;
}
)");

    connect(
        thisp->chapterListView,&ChapterListView::keyLeft,
        [this]() { thisp->textView->nextPage(-1); }
        );
    connect(
        thisp->chapterListView,&ChapterListView::keyRight,
        [this]() { thisp->textView->nextPage( 1); }
    );
    connect(
        thisp->textView,&TextView::keyUp,
        [this]() { thisp->chapterListView->setNextCurrentIndex( -1); }
        );
    connect(
        thisp->textView,&TextView::keyDown,
        [this]() { thisp->chapterListView->setNextCurrentIndex(  1); }
    );
     
}

ChapterListView * MainWindow::getChapterListView()const {
    return thisp->chapterListView;
}

MainWindow::~MainWindow()
{
    delete thisp;
}

void  MainWindow::paintEvent(QPaintEvent *){
	QPainter p(this);

    if( std::abs(painterTime.elapsed()) > 500 )
    {
        auto goal_=(std::rand()&5)+120;
        if (background_color.red()>goal_) {
            goal_=background_color.red()-1;
        }
        else {
            goal_=background_color.red()+1;
        }

        if (goal_>125) {
            goal_=124;
        }
        else if (goal_<120) {
            goal_=121;
        }

        QLinearGradient gradient(rect().topLeft(), rect().bottomLeft());

        background_color=QColor(
            goal_+(std::rand()&1)-2,
            goal_+(std::rand()&1)-2,
            goal_+(std::rand()&1)-2);

        gradient.setColorAt(0, background_color);

        background_color_1=QColor(
            goal_+(std::rand()&5)-50,
            goal_+(std::rand()&5)-50,
            goal_+(std::rand()&5)-50);
        gradient.setColorAt(1,background_color_1 );

        QPen pen;
        pen.setWidth(0);
        pen.setColor(QColor(0, 0, 0, 0));
        p.setPen(pen);

        QBrush brush(gradient);

        p.setBrush(brush);

        p.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painterTime.restart();
    }
    else {
        QLinearGradient gradient(rect().topLeft(), rect().bottomLeft());
        gradient.setColorAt(0, background_color);
        gradient.setColorAt(1,background_color_1 );
        QPen pen;
        pen.setWidth(0);
        pen.setColor(QColor(0, 0, 0, 0));
        p.setPen(pen);

        QBrush brush(gradient);

        p.setBrush(brush);

        p.setRenderHint(QPainter::HighQualityAntialiasing, true);
    }

    p.drawRect( this->rect() );

}

void MainWindow::startDownload(int v) {

	if (thisp->downLoad) {
		thisp->downLoad->disconnect(thisp->downLoad.get(),nullptr,this,nullptr);
	}
	
	QString dirPath = QApplication::applicationDirPath() ;
	{
		QDir dir(dirPath);
		dirPath.append( "/data" );
		dir.mkdir(dirPath);
		dirPath += "/"+QString("%1").arg( int(v) )  ;
		dir.mkdir(dirPath);
	}
	thisp->downLoad = 
		FengYunDownLoad::instance( dirPath );

	connect(
		thisp->downLoad.get(),
		&FengYunDownLoad::downLoadHeaders,
		this,
		&MainWindow::downLoadHeaders,
		Qt::QueuedConnection
		);

    connect(
        thisp->downLoad.get(),
        &FengYunDownLoad::downLoadAChapter,
        this,
        &MainWindow::trySetChapter,
        Qt::QueuedConnection
        );

    thisp->chapter = 0;
    thisp->textView->setText("");
	thisp->downLoad->startDownLoad( v );

}

void MainWindow::trySetChapter(int i) {
    if ( i == thisp->chapter ) {
        setChapter( i );
    }
}

void MainWindow::downLoadHeaders( FengYunDownLoadHeader h ) {

	this->setWindowTitle( h.bookName );
	thisp->chapterListView->resetModelData(
		h.allDatas->size(),
		h.allDatas->begin(),
		h.allDatas->end() 
		);

}

