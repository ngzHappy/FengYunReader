#ifndef __PRIVATE__TEXT__VIEW__
#define __PRIVATE__TEXT__VIEW__

#include <QThread>
#include <QTextDocument>
#include <QImage>
#include <QPointer>

class TextViewPrivateThread : public QThread {
    Q_OBJECT
public:
    void run() override { exec(); }
};

class TextViewPrivate : public QObject{
Q_OBJECT

    TextViewPrivateThread * thisThread = nullptr ;

public:

    TextViewPrivate();
    ~TextViewPrivate();

public slots:
    void draw( QPointer<QTextDocument> doc,const int start_ ,const int end_ );
signals:
    void drawFinished( QImage , int start_ , int end_ );
    void startDraw( QPointer<QTextDocument> doc,const int start_ ,const int end_ );
};


#endif

/*
 *
*/

