#ifndef CHAPTERITEMWIDGET_HPP
#define CHAPTERITEMWIDGET_HPP

#include <ItemView/AbstractItemWidget.hpp>
#include <FengYunCore/ChapterModel.hpp>
#include <bitset>

class ChapterItemWidget :
        public AbstractItemWidget
{
    Q_OBJECT
private:
    enum {
        STATE_SELECTED,
        STATE_ENTER,
        STATE_PAINTING,
        STATE_SIZE,
    };
    ChapterHeaderData chapterData{  (ChapterHeaderData::Undefine *)(nullptr) };
    ChapterModel * model ;
    std::bitset<STATE_SIZE> state;
    QObject * objectManager;
private:
    void _mouse_enter();
public:
    ChapterItemWidget();
    ~ChapterItemWidget();

protected:
    /* 即将删除widget */
    virtual void beforeWidgetDelete() override;

    /* 设置paint数据 */
    virtual void beforePaint(
            const QStyleOptionViewItem & option,
            const QModelIndex & index) override;

    /* paint */
    virtual void paintEvent( QPaintEvent * )   override;

    /* 设置模型数据 */
    virtual void setModelData(
            QAbstractItemModel * model,
            const QModelIndex & index
            ) const override;

    /* 重新设置数据 */
    virtual void setEditorData(
            const QModelIndex & index
            )  override;

    virtual void updateEditorGeometry(
        const QStyleOptionViewItem & option,
        const QModelIndex & index)   override;

    /* 构造 */
    virtual AbstractItemWidget * instance(
            QWidget * parent,
            const QStyleOptionViewItem & option,
            const QModelIndex & index
            ) override;

    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );

};

#endif // CHAPTERITEMWIDGET_HPP
