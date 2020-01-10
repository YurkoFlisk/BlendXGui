#pragma once

#include <QMainWindow>
#include "engine/basic_types.h"

class QLCDNumber;
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
	void sRefreshClock();
	void sEngineError(BlendXChess::Side side, QString errorText);
	void sSearchInfo(BlendXChess::Side side, const struct SearchInfoDetails& info);

	void sNewPVPGame();
	void sNewPVEGame();
	void sNewEVEGame();
	void sAbout();
	void sQuit();
	/*void sOpenDB();*/
	void sOpenFile();
	/*void sSaveDB();*/
	void sSaveFile();
	void sClose();
	void sUndo();
	void sRedo();
	void sEngines();
private:
	void clearClocks();
	void setGame(Game* game);
	void createActions();
	void createMenus();
	/*QString getEnginePath(int id);*/
	// Members
	QTimer* m_clockTimer; // Timer for clock refresh
	QLCDNumber* m_clockDisplay[2]; // Display for clocks of sides
	BoardWidget* m_boardWidget;
	Game* m_game;
	EngineInfoWidget* m_engineInfoWidget;
	EnginesModel* m_engines;

	QMenu* m_fileMenu;
	QMenu* m_aboutMenu;
	QMenu* m_enginesMenu;
	QAction* m_newPVPGameAction; // Player with player
	QAction* m_newPVEGameAction; // Player with engine
	QAction* m_newEVEGameAction; // Engine with engine
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
};

inline BoardWidget* QtChessGUI::getBoardWidget() const
{
	return m_boardWidget;
}