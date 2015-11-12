#ifndef MAINWINDOW_HPP_P
#define MAINWINDOW_HPP_P

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include "MainWindow.hpp"
#include "ChapterListView/ChapterListView.hpp"
#include "TextView/TextView.hpp"
#include "FengYunCore/FengYunDownLoad.hpp"

class MainWindowSplitter : public QSplitter{
    Q_OBJECT
public:

    using QSplitter::moveSplitter;

};

class MainWindow::MainWindowPrivate{
public:
    MainWindowSplitter * splitter;
	ChapterListView * chapterListView;
	TextView * textView;
	int chapter = 0 ;
	std::shared_ptr<FengYunDownLoad>   downLoad  ;
    bool isFirstShow=true;
	MainWindowPrivate() {}
	~MainWindowPrivate() {}

};

#endif // MAINWINDOW_HPP
