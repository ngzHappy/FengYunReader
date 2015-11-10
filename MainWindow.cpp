#include "MainWindow.hpp"
#include "MainWindowPrivate.hpp"


MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    auto * layout_ = new QHBoxLayout;
    this->setLayout(layout_);
    layout_->setMargin(0);
    layout_->setSpacing(0);
    this->setMinimumSize(360,360);

    thisp = new MainWindowPrivate;

    thisp->splitter = new QSplitter;
    layout_->addWidget ( thisp->splitter );

	thisp->chapterListView = new ChapterListView;
	thisp->splitter->addWidget(thisp->chapterListView);
}

MainWindow::~MainWindow()
{
    delete thisp;
}
