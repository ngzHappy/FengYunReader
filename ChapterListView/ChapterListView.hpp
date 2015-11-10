#ifndef CHAPTERLISTVIEW_HPP
#define CHAPTERLISTVIEW_HPP

#include <ItemView/ItemWidgetView.hpp>
#include <QListView>

class ChapterListView :
        public ItemWidgetView<QListView>
{
    Q_OBJECT
public:
    ChapterListView();
    ~ChapterListView();
};

#endif // CHAPTERLISTVIEW_HPP
