#include "ChapterListView.hpp"
#include "ChapterItemDelegate.hpp"
#include "ChapterItemWidget.hpp"

ChapterListView::ChapterListView(){
   this->setItemDelegate(new ChapterItemDelegate(this,
   [](const QModelIndex &)->AbstractItemWidget*{ return new ChapterItemWidget; } ,
   this)  );
}


ChapterListView::~ChapterListView(){

}

/*
 *
*/
