#ifndef TEXTVIEW_HPP
#define TEXTVIEW_HPP

#include <QScrollArea>
#include <QTextDocument>
#include <QImage>
#include <QTimer>

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
private:
    void gen_draw_picture();
    void _try_start_animation();
    void _key_up();
    void _key_down();
    void _key_left();
    void _key_right();
protected:
    void resizeEvent( QResizeEvent * ) override ;
    void keyPressEvent( QKeyEvent * ) override;
    private slots:
    void _animationTimer();
};

#endif // TEXTVIEW_HPP
