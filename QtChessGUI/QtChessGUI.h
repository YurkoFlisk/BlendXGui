#pragma once
#include "BoardWidget.h"
#include "NewGameDialog.h"
#include "DBBrowser.h"

class QtChessGUI : public QMainWindow
{
	Q_OBJECT

public:
	QtChessGUI(QWidget *parent = Q_NULLPTR);
	~QtChessGUI(void);
private:
	void createActions(void);
	void createMenus(void);
	// Slots
	void sNewGame(void);
	void sAbout(void);
	void sQuit(void);
	void sOpenDB(void);
	void sOpenFile(void);
	void sSaveDB(void);
	void sSaveFile(void);
	void sUndo(void);
	void sRedo(void);
	// Members
	NewGameDialog* m_newDialog;
	BoardWidget* m_boardWidget;
	QMenu* m_fileMenu;
	QMenu* m_aboutMenu;
	QAction* m_newAction;
	QAction* m_openFromFileAction;
	QAction* m_openFromDBAction;
	QAction* m_saveToDBAction;
	QAction* m_saveToFileAction;
	QAction* m_undoAction;
	QAction* m_redoAction;
	QAction* m_quitAction;
	QAction* m_aboutAction;
	QSqlDatabase db;
};
