#include "TextView.hpp"
#include "TextViewPrivate.hpp"
#include <QTextCursor>
#include <QTextCharFormat>
#include <QScrollBar>
#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <memory>
#include <QAbstractTextDocumentLayout>
#include <QMouseEvent>
#include <algorithm>
#include <vector>
#include <QTextBlock>

namespace {
    enum {
        DocumentMargin=3
    };
}

TextViewPrivate::TextViewPrivate(){
thisThread = new TextViewPrivateThread;
this->moveToThread( thisThread );
connect(this,&TextViewPrivate::startDraw,
        this,&TextViewPrivate::draw,
        Qt::QueuedConnection
        );
thisThread->start();
}
TextViewPrivate::~TextViewPrivate(){
thisThread->quit();
thisThread->wait( 1000 );
delete thisThread;
}

void TextViewPrivate::draw(
    QPointer< QTextDocument > _doc ,
    const int start_ ,
    const int end_ ){
    if( start_ > end_ ){ return; }
    if (_doc.isNull()) { return; }

    QTextDocument * doc=_doc.data();
    if (doc==nullptr) { return; }

    doc = doc->clone();
    std::unique_ptr< QTextDocument > __doc(  doc  );

    auto & document=*doc;
    QImage aboutToDraw;

    {
        const auto pageCount_ = document.pageCount();
        const auto pageSize = document.pageSize();

        int width_=int(pageSize.width()+0.999f);
        int height_=int(pageSize.height()*pageCount_+0.999f);

        if ((width_<=0)||(height_<=0)) {
            aboutToDraw=QImage();
            emit drawFinished( aboutToDraw ,start_,end_ );
            return;
        }

        {
            QImage image(width_, height_, QImage::Format_ARGB32 );
            image.fill( QColor(0,0,0,0) );
            {
                QPainter painter(&image);
                painter.setRenderHint(QPainter::TextAntialiasing,true);
                QAbstractTextDocumentLayout * layout_ = document.documentLayout();
                QAbstractTextDocumentLayout::PaintContext ctx;

                {
                    typedef QAbstractTextDocumentLayout::Selection Selection;
                    QVector<Selection> selections_;
                    QTextCursor tc( doc );
                    tc.setPosition( start_ ,QTextCursor::MoveAnchor );
                    tc.movePosition( QTextCursor::Right,QTextCursor::KeepAnchor,end_-start_ );
                    QTextCharFormat charFormat=tc.charFormat();
                    charFormat.setBackground( QColor(55,66,60,120 ) ) ;
                    selections_.push_back( { tc,  charFormat} );
                    ctx.selections = selections_ ;
                    ctx.cursorPosition = start_ ;
                }

                layout_->draw( &painter, ctx );
            }
            QImage image1(
                image.width()*pageCount_,
                int(pageSize.height()+0.999f),
                QImage::Format_ARGB32
                );
            image1.fill( QColor(0,0,0,0) );

            QPainter painter(&image1);
            for (int i=0; i<pageCount_;++i ) {
                painter.drawImage(
                    QPointF( image.width()*i ,0),image,
                    QRectF(  0,image1.height()*i,image.width(),image1.height() )
                    );
            }

            aboutToDraw=image1;
            emit drawFinished( aboutToDraw ,start_,end_ );
            return ;
        }


    }

    emit drawFinished( aboutToDraw ,start_,end_ );

}



void TextView::setText(const QString & t) {

    plainStringData=t;
    document.setPlainText(t);
    gen_draw_picture();

    {
        selectStartPos=-1;
        selectRange={ -1,-1 };
        aboutToDrawList.clear();
    }

    {
        auto hb=this->horizontalScrollBar();
        goalValue=0;
        hb->setValue( 0 );
    }

    if ( viewport() ) {
        viewport()->update();
    }

}

void TextView::Widget::paintEvent(QPaintEvent * e ){

    QPainter p(this);

    p.drawImage(
         e->rect().topLeft()
        ,textViewPort->aboutToDraw
        ,e->rect()
        );

}

void TextView::_try_start_animation(){
    if (animationTimer.isActive()) { return; }
    animationTimer.start(  5 );
}

void TextView::resizeEvent(QResizeEvent * e) {
    QScrollArea::resizeEvent(e);
    gen_draw_picture();

    {
        this->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        auto hb=this->horizontalScrollBar();
        hb->setPageStep( this->width() );
        hb->setSingleStep( this->width() );
        hb->setValue( 0 );
        goalValue = 0;
    }

}


void TextView::gen_draw_picture() {

    document.setDocumentMargin( DocumentMargin );
    document.setPageSize(
    QSize{
        this->width()  ,
        this->height() - int(this->verticalScrollBar()->height()*0.05)
    }
        );

    const auto pageCount_ = document.pageCount();
    const auto pageSize = document.pageSize();

    int width_=int(pageSize.width()+0.999f);
    int height_=int(pageSize.height()*pageCount_+0.999f);

    if ((width_<=0)||(height_<=0)) {
        aboutToDraw=QImage();
        return;
    }

    {
        QImage image(width_, height_, QImage::Format_ARGB32 );
        image.fill( QColor(0,0,0,0) );
        {
            QPainter painter(&image);
            painter.setRenderHint(QPainter::TextAntialiasing,true);
            document.drawContents(&painter);
        }
        QImage image1(
            image.width()*pageCount_,
            int(pageSize.height()+0.999f),
            QImage::Format_ARGB32
            );
        image1.fill( QColor(0,0,0,0) );

        QPainter painter(&image1);
        for (int i=0; i<pageCount_;++i ) {
            painter.drawImage(
                QPointF( image.width()*i ,0),image,
                QRectF(  0,image1.height()*i,image.width(),image1.height() )
                );
        }

        aboutToDraw=image1;

    }

    {
        bool isLastPageHaveData=true;

        if (aboutToDraw.width()>this->width()) {
            auto * d_ = aboutToDraw.constBits();
            const auto lineSize = aboutToDraw.bytesPerLine();
            const int offset_ = ( aboutToDraw.width()-this->width() )*4 ;
            for (int i=0; i<aboutToDraw.height();++i ) {
                auto start_= d_ + offset_ ;
                d_+=lineSize;
                for (;start_<d_;++start_) {
                    if (*start_==0) { continue; }
                    else {
                        goto lable_widget_resize;
                    }
                }
            }
            isLastPageHaveData=false;
        }

lable_widget_resize:
        if (isLastPageHaveData) {
            widget->resize(aboutToDraw.width(), aboutToDraw.height());
        }
        else {
            widget->resize(aboutToDraw.width() - this->width() , aboutToDraw.height());
        }
    }
}

TextView::TextView(QWidget *parent) :
    QScrollArea(parent){
    thisp=new TextViewPrivate;

    {
        this->startTimer( 200 );
    }

    {
        connect(
            thisp,&TextViewPrivate::drawFinished,
            this,&TextView::_redraw_select,
            Qt::QueuedConnection
            );
    }

    {
        widget=new Widget(this);
        this->setWidget( widget );
    }

    {
        typedef void(QTimer::* TTT)();
        animationTimer.connect(&animationTimer,TTT(&QTimer::timeout),
            this,&TextView::_animationTimer
            );
    }

    {
        QPalette pl =  palette();
        pl.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0  )));
        setPalette(pl);
        this->setFrameStyle(QFrame::NoFrame);
        this->setStyleSheet(" background :transparent; ");
    }

    {
        QFont font_=document.defaultFont();
        font_.setPointSize( 20 );
        font_.setWeight( QFont::Bold );
        document.setDefaultFont(font_);
    }

    {
        this->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        auto hb=this->horizontalScrollBar();
        hb->setStyleSheet(
            u8R"_(
QScrollBar:horizontal {
    border: 0px solid green;
    background: transparent ;
    height: 15px;
    margin: 0px 0px 0 0px;
}

QScrollBar::handle:horizontal {
    background: rgba( 100.100,100,100 );
    min-width: 20px;
}

QScrollBar::add-line:horizontal {
    background: transparent ;
    width: 0px;
    subcontrol-position: right;
    subcontrol-origin: margin;
    border: 0px solid black;
}

QScrollBar::sub-line:horizontal {
    background: transparent ;
    width: 0px;
    subcontrol-position: top right;
    subcontrol-origin: margin;
    border: 0px solid black;
    position: absolute;
    right: 0px;
}

QScrollBar:left-arrow:horizontal, QScrollBar::right-arrow:horizontal {
    width: 0px;
    height: 0px;
    background: pink;
}

QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
}

                        )_"
            );
    }

    {
        this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        auto * vb = this->verticalScrollBar();
        vb->setMinimum(0);
        vb->setMaximum(0);
        vb->connect(vb,&QScrollBar::valueChanged,
            [vb](int i) { if (i==0) { return; } vb->setValue(0); }
            );
        vb->setStyleSheet(
            u8R"(

     QScrollBar:vertical {
     border: 0px solid grey;
     background: rgba(222,222,222,22) ;
     width: 15px;
     margin: 0px 0 0px 0;
 }

     QScrollBar::handle:vertical {
     background: rgba( 100.100,100,100 );
     min-height: 20px;
 }

     QScrollBar::add-line:vertical {
     border: 0px solid yellow ;
     background: transparent ;
     height:  0px;
     subcontrol-position: bottom;
     subcontrol-origin: margin;
 }

     QScrollBar::sub-line:vertical {
     border: 0px solid grey;
     background: transparent ;
     height:  0px;
     subcontrol-position: top;
     subcontrol-origin: margin;
 }

     QScrollBar::add-page:vertical,
     QScrollBar::sub-page:vertical {
     background: transparent ;
 }

            )"
            );
    }

}


TextView::~TextView(){
    delete thisp;
    delete widget;
}

void TextView::_key_up() {
    emit keyUp();
}
void TextView::_key_down() {
    emit keyDown();
}
void TextView::_key_left() {
    //上一页
    auto g_ = goalValue;
    auto hb = this->horizontalScrollBar();
    g_-=hb->singleStep();
    if (g_<0) { g_=0; }
    if ( g_ == hb->value() ) { return; }
    goalValue=g_;
    _try_start_animation();
}
void TextView::_key_right() {
    //下一页
    auto g_ = goalValue;
    auto hb = this->horizontalScrollBar();
    g_+=hb->singleStep();
    g_=std::min(g_,hb->maximum());
    if ( g_ == hb->value() ) { return; }
    goalValue=g_;
    _try_start_animation();
}

void TextView::keyPressEvent(QKeyEvent * e) {

    switch (e->key())
    {
        case Qt::Key_Up: {
            _key_up();
        }; return;
        case Qt::Key_Down: {
            _key_down();
        }; return;
        case Qt::Key_Left: {
            _key_left();
        }; return;
        case Qt::Key_Right: {
            _key_right();
        }; return;
    }

    QScrollArea::keyPressEvent(e);

}

void TextView::_animationTimer() {

    auto v = this->horizontalScrollBar()->value();
    auto STEP= 1+ std::abs( v - goalValue )/2 ;
    if ( v>goalValue ) {
        v-=STEP;
        this->horizontalScrollBar()->setValue(  std::max(v, goalValue) );
        return;
    }
    else {
        if (v<goalValue) {
            v+=STEP;
            this->horizontalScrollBar()->setValue( std::min(v, goalValue)  );
            return;
        }
    }

    animationTimer.stop();
    return ;
}

QPoint TextView::_project(QPoint p ) {

    const auto hv_ = this->horizontalScrollBar()->value();
    const auto x_=hv_+p.x();
    const auto y_=p.y();
    const auto pageSize = document.pageSize();

    QPoint ans{x_,y_};

    const auto page_=x_/int( pageSize.width() );
    ans.setY( int(page_*pageSize.height()+y_) );
    ans.setX( x_-int( page_*pageSize.width() ) );

    return ans;

}

void TextView::mousePressEvent(QMouseEvent * e)  {
QScrollArea::mousePressEvent(e);

if( e->button() & Qt::LeftButton ){
    isLeftKeyPressed = true;
    auto x_ = document.documentLayout()->hitTest( _project(e->pos()) ,Qt::FuzzyHit );
    selectRange = {x_,x_};
    selectStartPos=x_;
    QPointer< QTextDocument > doc_( &document );
    aboutToDrawList.clear();
    /* 开启绘制 */
    thisp->startDraw( doc_,x_,x_ );
}

}
void TextView::mouseReleaseEvent(QMouseEvent * e){
QScrollArea::mousePressEvent(e);
isLeftKeyPressed = false ;
}
void TextView::mouseMoveEvent(QMouseEvent * e)   {
QScrollArea::mouseMoveEvent(e);
if ( isLeftKeyPressed ) {
    auto x_ = document.documentLayout()->hitTest( _project(e->pos()) ,Qt::FuzzyHit );
    if ( x_>selectStartPos ) {
        //QPointer< QTextDocument > doc_( &document );
        auto selectRange_ = std::pair<int,int>{selectStartPos,x_};
        if (selectRange_==selectRange) { return; }
        selectRange=selectRange_;
        /* 加入等待绘制队列 */
        aboutToDrawList.emplace_back( selectStartPos,x_ );
        //thisp->startDraw( doc_,selectStartPos,x_ );
    }
    else if(x_<selectStartPos){
        //QPointer< QTextDocument > doc_( &document );
        auto selectRange_ = std::pair<int,int>{x_,selectStartPos};
        if (selectRange_==selectRange) { return; }
        selectRange=selectRange_;
        /* 加入等待绘制队列 */
        aboutToDrawList.emplace_back( selectStartPos,x_ );
        //thisp->startDraw( doc_,x_,selectStartPos );
    }
}
}

void TextView::_try_draw_next() {

    {
        if (aboutToDrawList.empty() == false ) {
            std::vector< std::pair<int, int> > __dsl;
            QPointer< QTextDocument > doc_( &document );
            auto _aboutToDraw = *aboutToDrawList.begin();
            aboutToDrawList.pop_front();
            __dsl.push_back( _aboutToDraw );
            while ( aboutToDrawList.empty()==false ) {
                auto i=aboutToDrawList.begin();
                if ( i->first == _aboutToDraw.first ) {
                    __dsl.push_back(*i);
                    aboutToDrawList.pop_front();
                }
                else {
                    break;
                }
            }
            std::sort(__dsl.begin(), __dsl.end(), [](const auto & i,const auto & j) {
                return std::abs( i.first - i.second ) < std::abs( j.first - j.second );
            });
            _aboutToDraw=*( __dsl.rbegin() );
            if (_aboutToDraw.first < _aboutToDraw.second) {
                thisp->startDraw(doc_, _aboutToDraw.first, _aboutToDraw.second);
            }
            else {
                thisp->startDraw(doc_, _aboutToDraw.second, _aboutToDraw.first);
            }
        }

    }

}

void TextView::mouseDoubleClickEvent(QMouseEvent * e ){
    QScrollArea::mouseDoubleClickEvent(e);
    {
        isLeftKeyPressed=false;
        auto x_=document.documentLayout()->hitTest(_project(e->pos()), Qt::FuzzyHit);
        QTextCursor cur(&document);
        cur.setPosition(x_);
        auto block=cur.block();
        if (block.isValid()==false) { return; }
        selectRange={ block.position(),block.position()+block.length() };
        QPointer< QTextDocument > doc_(&document);
        thisp->startDraw(doc_, selectRange.first, selectRange.second);
    }
}

void TextView::timerEvent(QTimerEvent * e ){
    QScrollArea::timerEvent(e);
    this->_try_draw_next();
}

void TextView::_redraw_select(QImage i, int  x_ , int y_ ) {
   _try_draw_next();

   if (x_<selectRange.first) { return; }
   if (y_>selectRange.second) { return; }

   aboutToDraw=i;
   this->viewport()->update();
}

/*
*/
