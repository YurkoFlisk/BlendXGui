#pragma once
#include "BoardWidget.h"
#include "NewGameDialog.h"
#include <QtSql>

class QtChessGUI : public QMainWindow
{
	Q_OBJECT

public:
	QtChessGUI(QWidget *parent = Q_NULLPTR);
	~QtChessGUI(void);
	inline BoardWidget* getBoardWidget(void) const;
private:
	void createActions(void);
	void createMenus(void);
	QString getEnginePath(int id);
	// Slots
	void sNewGame(void);
	void sAbout(void);
	void sQuit(void);
	void sOpenDB(void);
	void sOpenFile(void);
	void sSaveDB(void);
	void sSaveFile(void);
	void sClose(void);
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
	QAction* m_closeAction;
	QAction* m_undoAction;
	QAction* m_redoAction;
	QAction* m_quitAction;
	QAction* m_aboutAction;
	QSqlDatabase db;
};

inline BoardWidget* QtChessGUI::getBoardWidget(void) const
{
	return m_boardWidget;
}