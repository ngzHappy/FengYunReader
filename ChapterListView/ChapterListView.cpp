#include "ChapterListView.hpp"
#include "ChapterItemDelegate.hpp"
#include "ChapterItemWidget.hpp"
#include <FengYunCore/ChapterModel.hpp>
#include <QScrollBar>
#include <QKeyEvent>

ChapterListView::ChapterListView(){
   this->setItemDelegate(new ChapterItemDelegate(this,
   [](const QModelIndex &)->AbstractItemWidget*{ return new ChapterItemWidget; } ,
   this)  );

   this->setMinimumWidth( 98 );
   this->setFrameStyle(QFrame::NoFrame);

   {
       model_ = new ChapterModel;
       model_->setParent(this);
       this->setItemWidgetModel(model_);
   }

   {
	   QPalette pl = palette();
	   pl.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
	   setPalette(pl);
   }

   {
	   auto * vb = this->verticalScrollBar();
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
     border: 0px solid grey;
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

 QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
     background: transparent ;
 }

)"
		   );
   }

}


void ChapterListView::currentChanged(
        const QModelIndex &current,
        const QModelIndex &previous){
    PT::currentChanged(current,previous);
    currentIndexChanged( current.row() );
}

ChapterListView::~ChapterListView(){

}

void ChapterListView::setNextCurrentIndex(int i) {

    const auto size_ = model_->size();
    if (size_<=0) { return; }

    auto index_=currentIndex();
    if ( index_.isValid() == false ) {
        return setCurrentIndex( model_->index(0, 0) );
    }

    if (i>0) {
        i=index_.row()+1;
    }
    else if(i<0) {
        i=index_.row()-1;
    }

    if (i<0) {  i=size_-1; }
    else if (i>=size_) { i=0; }

    setCurrentIndex( model_->index(i, 0) );

}

void ChapterListView::keyPressEvent(QKeyEvent *e) {
    switch ( e->key() )
    {
        case Qt::Key_Left:return keyLeft();
        case Qt::Key_Right:return keyRight();
    }
    PT::keyPressEvent( e );
}

/*
 *
*/
