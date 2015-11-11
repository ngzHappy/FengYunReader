#ifndef MAINWINDOW_HPP_P
#define MAINWINDOW_HPP_P

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include "MainWindow.hpp"
#include "ChapterListView/ChapterListView.hpp"
#include "TextView/TextView.hpp"
#include "FengYunCore/FengYunDownLoad.hpp"

class MainWindow::MainWindowPrivate{
public:
	QSplitter * splitter;
	ChapterListView * chapterListView;
	TextView * textView;
	int chapter = 0 ;
	std::shared_ptr<FengYunDownLoad>   downLoad  ;
	MainWindowPrivate() {}
	~MainWindowPrivate() {}

};

#endif // MAINWINDOW_HPP
