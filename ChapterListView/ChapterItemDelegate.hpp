#ifndef CHAPTERITEMDELEGATE_HPP
#define CHAPTERITEMDELEGATE_HPP

#include <ItemView/AbstractItemWidgetDelegate.hpp>

class ChapterItemDelegate
        :public AbstractItemWidgetDelegate
{
    Q_OBJECT
public:
    ChapterItemDelegate(AbstractItemWidgetView * v,const CreatEditorFunctionType & f, QObject *p);
    ~ChapterItemDelegate();
};

#endif // CHAPTERITEMDELEGATE_HPP
