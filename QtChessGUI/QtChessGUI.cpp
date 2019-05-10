#include "QtChessGUI.h"

QtChessGUI::QtChessGUI(QWidget *parent)
	: QMainWindow(parent)
{
	BoardWidget* boardWidget = new BoardWidget(this);
	QWidget* centralWidget = new QWidget;
	QHBoxLayout* mainLayout = new QHBoxLayout;

	mainLayout->addWidget(boardWidget);

	centralWidget->setLayout(mainLayout);

	setCentralWidget(centralWidget);
}
