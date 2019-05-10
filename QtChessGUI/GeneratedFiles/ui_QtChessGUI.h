/********************************************************************************
** Form generated from reading UI file 'QtChessGUI.ui'
**
** Created by: Qt User Interface Compiler version 5.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTCHESSGUI_H
#define UI_QTCHESSGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtChessGUIClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtChessGUIClass)
    {
        if (QtChessGUIClass->objectName().isEmpty())
            QtChessGUIClass->setObjectName(QStringLiteral("QtChessGUIClass"));
        QtChessGUIClass->resize(600, 400);
        menuBar = new QMenuBar(QtChessGUIClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        QtChessGUIClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtChessGUIClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        QtChessGUIClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(QtChessGUIClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        QtChessGUIClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(QtChessGUIClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        QtChessGUIClass->setStatusBar(statusBar);

        retranslateUi(QtChessGUIClass);

        QMetaObject::connectSlotsByName(QtChessGUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtChessGUIClass)
    {
        QtChessGUIClass->setWindowTitle(QApplication::translate("QtChessGUIClass", "QtChessGUI", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtChessGUIClass: public Ui_QtChessGUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTCHESSGUI_H
