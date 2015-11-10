#ifndef __ITEM__WIDGET__VIEW__H0PP__
#define __ITEM__WIDGET__VIEW__H0PP__

#include "AbstractItemWidget.hpp"
#include "AbstractItemWidgetDelegate.hpp"
#include "AbstractItemWidgetView.hpp"
#include <QAbstractItemView>
#include <QEvent>
#include <QScrollBar>

template<typename ViewBaseType>
class ItemWidgetView  :
        public ViewBaseType ,
        public AbstractItemWidgetView
{
    bool isOnChangingStruct = false;
public:

    QAbstractItemView * getWidgetItemView() const override {
        return (QAbstractItemView *)this;
    }

    void beginChangeModelStruct(){
		isOnChangingStruct = true;
        if(this->isAbstractItemWidgetViewOnDestory()){return;}
        auto* d_ = qobject_cast<AbstractItemWidgetDelegate *>(this->itemDelegate());
        if (d_) { 
			d_->stopGC();
			d_->closeAllWidgetItem();
		}
    }

    void endChangeModelStruct(){
		isOnChangingStruct = false;
        if(this->isAbstractItemWidgetViewOnDestory()){return;}
		auto* d_ = qobject_cast<AbstractItemWidgetDelegate *>(this->itemDelegate());
		if (d_) {
			d_->startGC();
		}
        this->viewport()->update();
    }

protected:
    void paintEvent(QPaintEvent *e) override{
        /* 在model struct改变期间禁止重绘 */
        if(isOnChangingStruct){return;}
        ViewBaseType::paintEvent(e);
    }
    bool viewportEvent(QEvent *event) override{
        /* 在model struct改变期间禁止重绘 */
        if(isOnChangingStruct&&(event->type() == QEvent::Paint)){
            return true;
        }
        return ViewBaseType::viewportEvent(event);
    }



public:
    explicit ItemWidgetView( QWidget * s =0): ViewBaseType(s){
		QAbstractItemView * view_ = this;
        QScrollBar * sb_ = view_->verticalScrollBar();
        sb_->connect(sb_,&QScrollBar::valueChanged,
                     [this](int){
            this->viewport()->update();
        }
                     );
    }
    ~ItemWidgetView() {
		/* 关闭绘制体系 */
        isOnChangingStruct = true;
		/* 关闭所有item widget */
        auto * d_ = qobject_cast<AbstractItemWidgetDelegate *>(this->itemDelegate());
        if (d_) { d_->closeAllWidgetItem(); }
    }

};


#endif


