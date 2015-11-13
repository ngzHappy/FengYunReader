#ifndef TEXTVIEW_HPP
#define TEXTVIEW_HPP

#include <QScrollArea>
#include <QTextDocument>
#include <QImage>
#include <QTimer>
#include <utility>
#include <list>

class TextViewPrivate;
class TextView :
    public QScrollArea
{
    Q_OBJECT
private:

    QString plainStringData;
    QTextDocument document;
    QImage aboutToDraw;
    QTimer animationTimer;
    int goalValue = 0 ;
    TextViewPrivate * thisp = nullptr ;
    std::pair<int,int> selectRange ;
    int selectStartPos = -1;
    std::list< std::pair<int, int> > aboutToDrawList;
    bool isLeftKeyPressed = false ;

    class Widget : public QWidget {
    public:
        TextView * textViewPort;
        Widget( TextView * v ):textViewPort(v) {}
    protected:
        void paintEvent( QPaintEvent * ) override;
    };

    Widget * widget = nullptr ;

public:
    explicit TextView(QWidget *parent = 0);
    ~TextView();

    void setText(const QString & t);

signals:
    void keyUp();
    void keyDown();
public slots:
    void nextPage( int i ) {
        if (i==0) { return; }
        if (i<0) { return _key_left(); }
        _key_right();
    }
private slots:
    void gen_draw_picture();
    void _try_start_animation();
    void _try_draw_next(  );
    void _key_up();
    void _key_down();
    void _key_left();
    void _key_right();
private:
    QPoint _project( QPoint );
protected:
    void resizeEvent( QResizeEvent * )        override;
    void keyPressEvent( QKeyEvent * )         override;
    void mousePressEvent(QMouseEvent *)       override;
    void mouseReleaseEvent(QMouseEvent *)     override;
    void mouseMoveEvent(QMouseEvent *)        override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void timerEvent(QTimerEvent * event)      override;
    private slots:
    void _animationTimer();
    void _redraw_select( QImage,int,int );
};

#endif // TEXTVIEW_HPP
