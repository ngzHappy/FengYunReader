#ifndef MAINWINDOW_HPP_P
#define MAINWINDOW_HPP_P

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include "MainWindow.hpp"
#include "ChapterListView/ChapterListView.hpp"

class MainWindow::MainWindowPrivate{
public:
	QSplitter * splitter;
	ChapterListView * chapterListView;

	MainWindowPrivate() {}
	~MainWindowPrivate() {}

};

#endif // MAINWINDOW_HPP
