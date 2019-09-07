#pragma once

#include <QtSql>
#include <QMainWindow>
#include "engine/basic_types.h"

class NewGameDialog;
class BoardWidget;
class EngineInfoWidget;
class EnginesModel;
class PresetsModel;
class Game;

class QtChessGUI : public QMainWindow
{
	Q_OBJECT

public:
	QtChessGUI(QWidget* parent = Q_NULLPTR);
	~QtChessGUI();
	inline BoardWidget* getBoardWidget() const;
private slots:
	void sEngineError(BlendXChess::Side side, QString errorText);
	void sEngineInfo(BlendXChess::Side side, const struct SearchInfoDetails& info);

	void sNewGame();
	void sAbout();
	void sQuit();
	void sOpenDB();
	void sOpenFile();
	void sSaveDB();
	void sSaveFile();
	void sClose();
	void sUndo();
	void sRedo();
	void sEngines();
	void sPresets();
private:
	void createActions();
	void createMenus();
	QString getEnginePath(int id);
	// Members
	NewGameDialog* m_newDialog;
	BoardWidget* m_boardWidget;
	Game* m_game;
	EngineInfoWidget* m_engineInfoWidget;
	EnginesModel* m_engines;
	PresetsModel* m_presets;

	QMenu* m_fileMenu;
	QMenu* m_aboutMenu;
	QMenu* m_enginesMenu;
	QAction* m_newAction;
	QAction* m_openFromFileAction;
	QAction* m_openFromDBAction;
	QAction* m_saveToDBAction;
	QAction* m_saveToFileAction;
	QAction* m_closeAction;
	QAction* m_undoAction;
	QAction* m_redoAction;
	QAction* m_enginesAction;
	QAction* m_presetsAction;
	QAction* m_quitAction;
	QAction* m_aboutAction;
	QSqlDatabase db;
};

inline BoardWidget* QtChessGUI::getBoardWidget() const
{
	return m_boardWidget;
}