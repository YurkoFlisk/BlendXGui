#include "QtChessGUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtChessGUI w;
	w.show();
	return a.exec();
}
