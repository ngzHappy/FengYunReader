#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>
#include <ChapterListView/ChapterListView.hpp>
#include <FengYunCore/FengYunDownLoad.hpp>
#include <QTime>

class MainWindow : public QWidget
{
    Q_OBJECT
private:
    QColor background_color;
    QColor background_color_1;
    QTime painterTime;
    QString bookName ;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    ChapterListView * getChapterListView()const ;

    int getChapter(  ) const;

public:
    void startDownload( int  );
signals:

private:
    class MainWindowPrivate;
    friend class MainWindowPrivate;
    MainWindowPrivate * thisp = nullptr;
public slots:
    void setChapter( int );
    void trySetChapter(int  );
    void downLoadHeaders(FengYunDownLoadHeader);
protected:

    void paintEvent(QPaintEvent * );
    void showEvent(QShowEvent * event) override ;
};

#endif // MAINWINDOW_HPP
