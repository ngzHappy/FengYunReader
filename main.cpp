#include "MainWindow.hpp"
#include <QApplication>
#include <QImage>
 
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
    window.show();

	{
		bool ok_;
		QString argv_1(argv[1]);
		int bookindex = argv_1.toInt(&ok_);
		if (false == ok_) { return -1; }
		if (bookindex <= 0) { return -1; }
		window.startDownload(bookindex);
	}

    window.resize(1024,768);

    return app.exec();
}

/*
*/
