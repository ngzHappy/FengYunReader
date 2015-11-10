#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	class MainWindowPrivate;
	friend class MainWindowPrivate;
	MainWindowPrivate * thisp = nullptr;
};

#endif // MAINWINDOW_HPP
