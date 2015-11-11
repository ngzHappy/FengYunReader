#include "ChapterItemWidget.hpp"
#include <type_traits>
#include <QPropertyAnimation>
#include <QPainter>
#include <cstdlib>
#include <QTextLayout>
#include <QFontMetrics>

namespace {
    int randInt_80_180() {
        return (std::rand()%100) + 80;
    }

    int randInt_150_180() {
        return (std::rand()%30) + 150;
    }
}

ChapterItemWidget::ChapterItemWidget(){
    objectManager = new QObject;
}


ChapterItemWidget::~ChapterItemWidget(){
    beforeWidgetDelete();
}


/* 即将删除widget */
void ChapterItemWidget::beforeWidgetDelete() {

    auto od_ = objectManager;
    objectManager = 0;

    if (od_ == nullptr) { return; }
    delete od_;

}

/* 设置paint数据 */
void ChapterItemWidget::beforePaint(
        const QStyleOptionViewItem & option_ ,
        const QModelIndex & index_ ) {

    if ( option_.state&QStyle::State_Selected ) {
        state[STATE_SELECTED] = true;
    }
    else {
        state[STATE_SELECTED] = false;
    }

    if ( false == state[STATE_ENTER] ) {
        /* 设置item大小 */
        updateEditorGeometry(option_, index_);
    }
    else {
        /* 由动画负责设置大小 */
        finalRect_ = option_.rect;
    }

    setEditorData( index_ );

}

void ChapterItemWidget::_mouse_enter() {

    if ( objectManager == nullptr ) { return; }
    if ( isFirstPainted == false ) { return; }
    if ( state[STATE_ENTER] ) { return; }

    state[STATE_ENTER] = true;
    const auto && finalPos = this->geometry().topLeft();
    auto * animation = new QPropertyAnimation(this, "pos", objectManager);
    animation->setDuration(100);
    animation->setStartValue(finalPos - QPoint(12, 0));
    animation->setEndValue(finalPos);
    animation->connect(animation, &QPropertyAnimation::finished,
        [this]() {
        state[STATE_ENTER] = false;
        this->setGeometry(finalRect_);
    });
    animation->start(QPropertyAnimation::DeleteWhenStopped);

}

/* paint */
void ChapterItemWidget::paintEvent( QPaintEvent * )   {

    if ( state[STATE_PAINTING] ) { return; }
    if (objectManager == 0) { return; }

    //isPainting
    class PaintingLocker {
        ChapterItemWidget * data__;
    public:
        ~PaintingLocker() {
            data__->isFirstPainted = true;
            data__->state[STATE_PAINTING] = false;
        }
        PaintingLocker(ChapterItemWidget * d) :data__(d) {
            d->state[STATE_PAINTING] = true;
        }
    };

    if ( chapterData.isValid() == false ) { return; }

    PaintingLocker paintLocker(this);

    QPainter painter(this);

    /* 绘制背景 */
    if ( this->underMouse()  ) {
        painter.fillRect( this->rect(),QColor(randInt_80_180(), randInt_80_180(), randInt_80_180() , 150 ) );
    }
    else {
        painter.fillRect( this->rect(), QColor(randInt_80_180()/2-10, randInt_80_180()/2-10, randInt_80_180()/2-10 , 222 )  );
    }

    /* 绘制是否选中 */
    if ( state[STATE_SELECTED] ) {
        QPen pen;
        pen.setWidth( 6 );
        pen.setColor(QColor(220, 223, 222, 200 ));
        painter.setPen(pen);
        painter.setBrush(QColor(0,0,0,0));
        painter.drawRect( this->rect() );
    }

    /* 绘制文字 */
    QString stringData = QString::asprintf(" %5d. ",
        chapterData.getChapterIndex()) +
        chapterData.getChapterName() ;
    {
        {
            QFont font = painter.font();
            font.setPointSize(13);
            //font.setWeight( QFont::Bold );
            painter.setFont(font);
        }
        QFontMetricsF fm_( painter.font() );
        painter.setBrush( QColor(randInt_150_180()+30, randInt_150_180()+30, randInt_150_180()+30,199));
        painter.setPen(   QColor(randInt_150_180()+30, randInt_150_180()+30, randInt_150_180()+30, 199));
        painter.drawText(
            QRect( 0, 0, int( fm_.width(stringData)+1 ) ,this->height()),
            Qt::AlignLeft| Qt::AlignVCenter,
            stringData );
    }

    //if ( false == isFirstPainted ) {
    //    isFirstPainted=true;
    //    return _mouse_enter();
    //}

}

/* 设置模型数据 */
void ChapterItemWidget::setModelData(
        QAbstractItemModel * model,
        const QModelIndex & index
        ) const {
    (void)model; (void)index;
}

/* 重新设置数据 */
void ChapterItemWidget::setEditorData(
        const QModelIndex & index_
        )  {

    if (index_.isValid() == false) { return; }
    typedef  QAbstractItemModel *  type_;
    model = qobject_cast<ChapterModel *>( type_( index_.model() ) );
    if (model == nullptr) { return; }
    chapterData = model->get(index_.row());
    if (false == bool(chapterData)) { return; }

}

void ChapterItemWidget::updateEditorGeometry(
    const QStyleOptionViewItem & option,
    const QModelIndex & index)  {
    finalRect_ = option.rect;
    this->setGeometry( finalRect_ );
    return; (void)index;
}


/* 构造 */
AbstractItemWidget * ChapterItemWidget::instance(
        QWidget * parent,
        const QStyleOptionViewItem & option,
        const QModelIndex & index
        ) {
    /* 设置父,保证被删除 */
    this->setParent(parent);
    /* 设置绘制数据 */
    this->beforePaint(option,index);
    return this;
}

void ChapterItemWidget::enterEvent(QEvent *){
    this->update();
    return _mouse_enter();
}
void ChapterItemWidget::leaveEvent(QEvent *){
    this->update();
}

/*
*/
