#include "MainWindow.hpp"
#include <QApplication>
#include <QImage>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    if (argc == 1) {
        return -1;
    }

    QApplication app(argc, argv);

    {/* 强制加载qimage插件 */
        QImage * temp = new QImage("@@null.png");
        delete temp;
    }

    MainWindow window;

    {
        bool ok_;
        QString argv_1(argv[1]);
        int bookindex = argv_1.toInt(&ok_);
        if (false == ok_) { return -1; }
        if (bookindex <= 0) { return -1; }
        window.startDownload(bookindex);
    }

    {
        auto desktopWidget = QApplication::desktop();
        auto size = desktopWidget->availableGeometry().size();
        if (size.width() > 1024) { size.setWidth(1024); }
        else { size.setWidth(int(0.8*size.width())); }
        if (size.height() > 768) { size.setHeight(768); }
        else { size.setHeight(int(0.9*size.height())); }
        window.resize(size);
    }

    window.show();

    return app.exec();
}

/*
*/
