#include "TextView.hpp"
#include <QTextCursor>
#include <QTextCharFormat>
#include <QScrollBar>
#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <algorithm>

void TextView::setText(const QString & t) {

    plainStringData=t;
    document.setPlainText(t);
    gen_draw_picture();

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

    enum {
        DocumentMargin = 3
    };
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
