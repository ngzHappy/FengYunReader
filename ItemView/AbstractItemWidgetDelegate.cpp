#include "AbstractItemWidgetView.hpp"
#include "AbstractItemWidgetDelegate.hpp"
#include "AbstractItemWidget.hpp"
#include <QStyledItemDelegate>
#include <QPainter>
#include <QCache>
#include <QEvent>
#include <QTimer>
#include <QApplication>
#include <QDebug>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QTimer>
#include <map>
#include <memory>
#include <thread>
#include <chrono>
#include <set>
#include <fstream>

namespace {
	namespace __AbstractItemWidgetDelegate {
		/*
		* ViewType should have concept openPersistentEditorWidget && closePersistentEditorWidget
		*/
		template<typename ViewType>
		class EditorManager {
		public:

			ViewType * view;
			class EditorItem {
			public:
				EditorManager<ViewType> * manager;
				QModelIndex index;
				AbstractItemWidget * widget = 0;
				EditorItem(
					EditorManager<ViewType> * m,
					const QModelIndex & index_
					) :manager(m), index(index_) {
					//m->view->openPersistentEditorWidget(index_);
				}

				~EditorItem() {
					if (manager->view->isAbstractItemWidgetViewOnDestory()) {
					/*如果正在析构 就不必关闭了 最后统一删除*/
						return;
					}
					manager->view->closePersistentEditorWidget(index);
				}

				friend bool operator<(const EditorItem & l, const EditorItem & r) {
					return l.index < r.index;
				}

			};

			EditorManager(ViewType * v) :
				view(v) {}
			~EditorManager() {}


			std::map< QModelIndex,
				std::shared_ptr<EditorItem>
			> data;

			virtual void tryClear() {
			}

		};

typedef __AbstractItemWidgetDelegate::EditorManager< AbstractItemWidgetView > ManagerType;

	}
}

namespace {
	const auto QListWidgetViewDelegateGCID = QEvent::registerEventType();
}

class AbstractItemWidgetDelegate::ThisPrivate {
public:
	AbstractItemWidgetDelegate * super;
	__AbstractItemWidgetDelegate::ManagerType manager;
	bool isPainting = false;
	bool isOpenCloseWidget = false;
	std::shared_ptr< bool > isOnDestory;
	bool isGCStarted = false;
	ThisPrivate(AbstractItemWidgetDelegate * s) :super(s),
		manager(s->getView()){
		isGCStarted = true;
		isOnDestory = std::shared_ptr<bool >(new bool(false));
		startAGCEvent();
	}

	~ThisPrivate() {
		*isOnDestory = true;
		isGCStarted = false;
	}
	
	/* gc evnet  */
	class QListWidgetViewDelegateGCEvent : public QEvent {
		AbstractItemWidgetDelegate * super_;
		std::shared_ptr<bool> isOnDestory;
	public:
		QListWidgetViewDelegateGCEvent(AbstractItemWidgetDelegate * s) :
			QEvent(QEvent::Type(QListWidgetViewDelegateGCID)) {
			super_ = s;
			isOnDestory = super_->thisp->isOnDestory;
		}
		~QListWidgetViewDelegateGCEvent() {
			if (*isOnDestory) { return; }
			super_->thisp->startAGCEvent();
		}

        void * operator new( std::size_t n ){//获得空间
              return malloc(n);
        }
        void operator delete(void * d){//释放空间
              free(d);
        }

	};

	void startAGCEvent() {
		if (*isOnDestory) { return; }
		QTimer::singleShot(50, super, [this]() {
			QApplication::postEvent(
				super,
				new QListWidgetViewDelegateGCEvent(super)
				);
		});
	}
	 
	int gcDataSize = 0;
 
	void GCFunction() { 

 
		do {
			int gcDataSizeAgo = gcDataSize;
			gcDataSize = int(manager.data.size());
			if ( (gcDataSize == gcDataSizeAgo)/* && (gcDataSize<20)*/ ) {
				/* 
				避免过于频繁调度gc
				增加下一次调用gc的概率
				*/
				++gcDataSize;
				return;
			}
#ifdef _DEBUG
			else {
				if ((gcDataSize >= 20))	{
					qDebug() << "data size:" << gcDataSize;
				}
			}
#endif

		} while (0);
 

		return _GCFunction();
	}
private:
	
	std::set<AbstractItemWidget *> childrenWidgets ;
	
	void _updateChildrenWidget(const QWidget * widget_) {
		/*
		此函数用于删除rect重复的项目
		如果两个widgetrect重复保留最上面的元素
		*/
		if (0 == widget_) { return; }
		auto & childrenWidgets_ = childrenWidgets;
		childrenWidgets_.clear();
		const auto & allChildren_ = widget_->children();

		std::map< 
			QRect, 
			AbstractItemWidget *,
			std::function<bool(const QRect &,const QRect&)> 
		> about_unique_([](const QRect & l, const QRect& r)->bool {
			if (l.x() < r.x()) { return true; }
			if (r.x() < l.x()) { return false; }
			if (l.y() < r.y()) { return true; }
			if (r.y() < l.y()) { return false; }
			if (l.width() < r.width()) { return true; }
			if (r.width() < l.width()) { return false; }
			if (l.height() < r.height()) { return true; }
			if (r.height() < l.height()) { return false; }
			return false;
		});

		{
            //int index__ = 0;

			typedef decltype(allChildren_.begin()) IT__;
			typedef std::reverse_iterator< IT__ > RIT__;

			RIT__ begin_( allChildren_.end() );
			RIT__ end_(   allChildren_.begin() );

			for (;begin_!=end_; ++begin_) {
				const auto & i = *begin_;
				auto * isChildren_ = qobject_cast<AbstractItemWidget *>(i);
				if (isChildren_) {
					/*  
					删除重复项 
					*/
					about_unique_.insert({isChildren_->finalRect(),isChildren_});
				}
			}
		}

		for (const auto &i:about_unique_) {
			childrenWidgets_.insert( i.second );
		}
		
	}

	bool _isShowing( const QWidget * widget__ ,const QRect  & rRect_ ) {
		const auto * widget_ = qobject_cast<const AbstractItemWidget *>(widget__);
		if (0 == widget_) { return false; }
	 
		/*
			增加边界
			为"setPos"之类的动画服务
			此类动画幅度不要超过5
			否则可能出现逻辑错误
		*/
		const auto && gValue_ = widget_->mapToGlobal(QPoint(-5, -5));
		const auto && size_ = widget_->size() + QSize(10, 10);
			
		return rRect_.intersects( QRect(gValue_,size_) );

	}

	void _GCFunction() try {

		if (*isOnDestory) { return; }
		if (false == isGCStarted) { return; }
		if (0 == super->view) { return; }

		/*
		采用 停止复制法
		采用全局坐标系
		*/
		QRect rRect_;
		{
			auto * v__ = super->view->getWidgetItemView();
			if (0 == v__) { return; }
			rRect_ = v__->rect();
			const auto && gValue_ = v__->mapToGlobal(QPoint(0, 0));
			rRect_ = QRect(gValue_.x(), gValue_.y(), rRect_.width(), rRect_.height());
		}

		typedef std::shared_ptr< __AbstractItemWidgetDelegate::ManagerType::EditorItem > IT_;
		std::list<IT_> tmp_pool;
		
		/* 避免出现乱七八糟的问你 */
		auto manager_data = std::move( manager.data );
		for (const auto & i : manager_data ) {
			if (i.second) {
				auto _0_widget_ = i.second->widget;
				if (0 == _0_widget_) { continue; }
				auto * p = _0_widget_->parentWidget();
				if (p) { _updateChildrenWidget(p); break; }
			}
		}

		for (const auto &i : manager_data) {

			if (false == i.second) { continue; }

			auto widget_ = i.second->widget;
			if (0 == widget_) {
				qDebug() << "!! is null";
				continue;
			}

			auto pWidget_ = widget_->parentWidget();
			if (0 == pWidget_) { 
				qDebug() << "!! is null";
				continue;
			}
			
			if ( (childrenWidgets.count(widget_)>0) && (_isShowing(widget_, rRect_) ) ) {
				tmp_pool.push_back(i.second);
			}
			else {
				i.second->widget->beforeWidgetDelete();
			}
		}
		
		manager.data.clear();
		for (const auto & i : tmp_pool) {
			manager.data.insert({ i->index,i });
		}

		/* 清除无用信息 */
		childrenWidgets.clear();

	}catch (...) {
		qDebug() 
			<< "crashed !!"
			<<__LINE__ 
			<< __FILE__ 
			<< __func__ ;
		using namespace std::chrono_literals;
		std::this_thread::sleep_for( 1ms );
		std::ofstream ofs("picture_view_gc.txt");
		ofs << "crashed !! "
			<< __LINE__ <<" "
			<< __FILE__ <<" "
			<< __func__ <<std::endl ;
		throw ;
	}

};

void AbstractItemWidgetDelegate::stopGC() {
	(thisp->isGCStarted) = false;
}
void AbstractItemWidgetDelegate::startGC() {
	(thisp->isGCStarted) = true;
}

/* 创建editor */
QWidget * AbstractItemWidgetDelegate::createEditor(
	QWidget * parent,
	const QStyleOptionViewItem & option,
	const QModelIndex & index) const {

	auto & manager = thisp->manager;

	if (manager.data.count(index)) {
		auto ans = manager.data[index]->widget;
		if (ans) { return ans; }
	}

	auto ans = creat_editor_function();

	if (ans) {
		ans->instance(parent,option,index);
		typedef __AbstractItemWidgetDelegate::ManagerType::EditorItem  T;
		typedef std::shared_ptr< T > TT;
		auto __ = new T(&manager, index);
		__->widget = ans;
		manager.data.insert({ index, TT(__) });
	}

	return ans;
}

void AbstractItemWidgetDelegate::paint(
	QPainter * painter,
	const QStyleOptionViewItem & option,
	const QModelIndex & index) const {

	/*
	防止事件循环形成递归
	*/
	if (thisp->isPainting) { return; }

	class BoolLocker {
		bool * b_;
	public:
		~BoolLocker() { *b_ = false; }
		BoolLocker(bool * b__) :b_(b__) { *b_ = true; }
	}_(&(thisp->isPainting));

	/*
	绘制父项会导致子项重绘
	*/
	painter->fillRect(option.rect, QColor(0, 0, 0, 0));
	/* ---- */
	auto ans = this->openPersistentEditorWidget(index);
	if (ans) {
		ans->beforePaint(option, index);
	}

}

AbstractItemWidget *
AbstractItemWidgetDelegate::openPersistentEditorWidget(const QModelIndex &index) const{
	if (thisp->isOpenCloseWidget) { return 0; }

	class BoolLocker {
		bool * b_;
	public:
		~BoolLocker() { *b_ = false; }
		BoolLocker(bool * b__) :b_(b__) { *b_ = true; }
	}_(&(thisp->isOpenCloseWidget));

	auto & manager = thisp->manager;
	auto v__ = view->getWidgetItemView();

	/* 如果已经存在  */
	if (manager.data.count(index)>0) {
		return manager.data[index].get()->widget;
	}

	/*  如果不存在 */
	v__->openPersistentEditor(index);
	return manager.data[index].get()->widget;

}

void AbstractItemWidgetDelegate::closeAllWidgetItem(){
    for( auto & i:thisp->manager.data ){
		/*准备关闭所有items*/
		i.second->widget->beforeWidgetDelete();
    }
	/*关闭所有items*/
    thisp->manager.data.clear();
}

void 
AbstractItemWidgetDelegate::closePersistentEditorWidget(const QModelIndex &index) const{
	if (thisp->isOpenCloseWidget) { return; }

	class BoolLocker {
		bool * b_;
	public:
		~BoolLocker() { *b_ = false; }
		BoolLocker(bool * b__) :b_(b__) { *b_ = true; }
	}_(&(thisp->isOpenCloseWidget));

	auto v__ = view->getWidgetItemView();
	v__->closePersistentEditor(index);

}


AbstractItemWidgetDelegate::AbstractItemWidgetDelegate(AbstractItemWidgetView *v,QObject *p):
	QStyledItemDelegate(p){
	setView(v);
	setCreatEditorFunction([]() { return new AbstractItemWidget; });
	thisp = new ThisPrivate(this);
}

AbstractItemWidgetDelegate::~AbstractItemWidgetDelegate() {
	delete thisp;
}

bool AbstractItemWidgetDelegate::event(QEvent * e)  {
	if ( e->type() == QListWidgetViewDelegateGCID ) {
		thisp->GCFunction();
		return true;
	}
	return QStyledItemDelegate::event(e);
}

/*设置item data*/
void AbstractItemWidgetDelegate::setEditorData(
	QWidget * editor,
	const QModelIndex & index) const   {
		{
			auto * item_ = qobject_cast<AbstractItemWidget *>(editor);
			if (item_) {
				return item_->setEditorData(index);
			}
		}
		QStyledItemDelegate::setEditorData(editor, index);
}

/*设置model data*/
void AbstractItemWidgetDelegate::setModelData(
	QWidget * editor,
	QAbstractItemModel * model,
	const QModelIndex & index) const  {
		{
			auto * item_ = qobject_cast<AbstractItemWidget *>(editor);
			if (item_) {
				return item_->setModelData(model, index);
			}
		}
		QStyledItemDelegate::setModelData(editor, model, index);
}

/*获得item的高 长没用忽略*/
QSize AbstractItemWidgetDelegate::sizeHint(
	const QStyleOptionViewItem & ,
	const QModelIndex & ) const  {
	return QSize(32,100);
}

void AbstractItemWidgetDelegate::updateEditorGeometry(
	QWidget * editor,
	const QStyleOptionViewItem & option,
	const QModelIndex & index)  const {
		{
			auto * item_ = qobject_cast<AbstractItemWidget *>(editor);
			if (item_) {
				return item_->updateEditorGeometry(option,index);
			}
		}
		QStyledItemDelegate::updateEditorGeometry(editor,option, index);
}

void 
AbstractItemWidgetDelegate::destroyEditor(
	QWidget * editor, 
	const QModelIndex & i) const {
	(void) i;
	//QStyledItemDelegate::destroyEditor( editor, i);
	/* 直接删除  */
	auto * editor_ = qobject_cast< AbstractItemWidget * >(editor);
	if (editor_) {
		editor_->hide();
		editor_->beforeWidgetDelete();
		editor_->deleteLater();
	}
	else {
		editor->deleteLater();
	}
}

/**/
