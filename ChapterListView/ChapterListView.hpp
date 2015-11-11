#ifndef CHAPTERLISTVIEW_HPP
#define CHAPTERLISTVIEW_HPP

#include <FengYunCore/ChapterModel.hpp>
#include <ItemView/ItemWidgetView.hpp>
#include <QListView>

class ChapterListView :
        public ItemWidgetView<QListView>
{
    Q_OBJECT

private:
    typedef ItemWidgetView<QListView> PT;
    ChapterModel * model_;
public:
    ChapterListView();
    ~ChapterListView();

    const ChapterModel * getModel() const { return model_; }

    template<typename It_b >
    void resetModelData(It_b b, It_b e) {
        model_->resetModelData(b,e);
    }

    template<typename It_b >
    void resetModelData(std::size_t s_,It_b b, It_b e) {
        model_->resetModelData(s_,b, e);
    }
public slots:
    void setNextCurrentIndex(int i);

signals:
    void currentIndexChanged( int );
    void keyLeft();
    void keyRight();
protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void keyPressEvent( QKeyEvent * ) override;
};

#endif // CHAPTERLISTVIEW_HPP
