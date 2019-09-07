#include <fstream>
#include <sstream>
#include "QtChessGUI.h"
#include "BoardWidget.h"
#include "EngineInfoWidget.h"
#include "OpenDBBrowser.h"
#include "EnginesBrowser.h"
#include "PresetsBrowser.h"
#include "Engine/engine.h"
#include "Core/EnginesModel.h"
#include "Dialogs/NewGameDialog.h"
#include "Dialogs/SaveDBBrowser.h"

QtChessGUI::QtChessGUI(QWidget* parent)
	: QMainWindow(parent)
{
	//db = QSqlDatabase::addDatabase("QMYSQL");
	//db.setHostName("localhost");
	//db.setDatabaseName("chessdb");
	//db.setUserName("root");
	//db.setPassword("LamboV3n3n0");
	//if (!db.open())
	//	QMessageBox::critical(this, tr("Error"), tr("Could not connect to database: ")
	//		+ db.lastError().text());
	///*if (!db.driver()->hasFeature(QSqlDriver::Transactions))
	//	QMessageBox::warning(this, "Error",
	//		"Your database doesn't support transactions");*/

	BlendXChess::Game::initialize();
	m_game = new Game(this);

	connect(m_game, &Game::engineErrorSignal, this, &QtChessGUI::sEngineError);
	connect(m_game, &Game::searchInfoSignal, this, &QtChessGUI::sEngineInfo);
	connect(m_game, &Game::engineInitSignal, m_engines, &EnginesModel::updateEngine);

	m_newDialog = new NewGameDialog(this);
	m_engineInfoWidget = new EngineInfoWidget(this);
	m_boardWidget = new BoardWidget(m_game, this);
	QWidget* const centralWidget = new QWidget;
	QHBoxLayout* const mainLayout = new QHBoxLayout;

	mainLayout->addWidget(m_boardWidget);
	mainLayout->addWidget(m_engineInfoWidget);

	centralWidget->setLayout(mainLayout);

	setCentralWidget(centralWidget);
	createActions();
	createMenus();
}

QtChessGUI::~QtChessGUI(void)
{
	db.close();
}

void QtChessGUI::createActions(void)
{
	m_newAction = new QAction(tr("&New"));
	m_newAction->setToolTip(tr("Start new game"));
	m_newAction->setShortcut(QKeySequence::New);
	connect(m_newAction, &QAction::triggered, this, &QtChessGUI::sNewGame);

	m_openFromDBAction = new QAction(tr("Open from &DB"));
	m_openFromDBAction->setToolTip(tr("Browse data from database and select the game to view"));
	connect(m_openFromDBAction, &QAction::triggered, this, &QtChessGUI::sOpenDB);

	m_openFromFileAction = new QAction(tr("Open from &file"));
	m_openFromFileAction->setToolTip(tr("Open game from PGN file"));
	connect(m_openFromFileAction, &QAction::triggered, this, &QtChessGUI::sOpenFile);

	m_saveToDBAction = new QAction(tr("&Save to DB"));
	m_saveToDBAction->setToolTip(tr("Save to DB in PGN format"));
	connect(m_saveToDBAction, &QAction::triggered, this, &QtChessGUI::sSaveDB);

	m_saveToFileAction = new QAction(tr("&Save to file"));
	m_saveToFileAction->setToolTip(tr("Save to file in PGN format"));
	connect(m_saveToFileAction, &QAction::triggered, this, &QtChessGUI::sSaveFile);

	m_undoAction = new QAction(tr("&Undo"));
	m_undoAction->setToolTip(tr("Undo the last move"));
	m_undoAction->setShortcut(QKeySequence::Undo);
	connect(m_undoAction, &QAction::triggered, this, &QtChessGUI::sUndo);

	m_redoAction = new QAction(tr("&Redo"));
	m_redoAction->setToolTip(tr("Redo the lastly undone move"));
	m_redoAction->setShortcut(QKeySequence::Redo);
	connect(m_redoAction, &QAction::triggered, this, &QtChessGUI::sRedo);

	m_closeAction = new QAction(tr("&Close"));
	m_closeAction->setToolTip(tr("Close current game"));
	m_closeAction->setShortcut(QKeySequence::Close);
	connect(m_closeAction, &QAction::triggered, this, &QtChessGUI::sClose);

	m_enginesAction = new QAction(tr("E&dit engines"));
	m_enginesAction->setToolTip(tr("Manage engines"));
	connect(m_enginesAction, &QAction::triggered, this, &QtChessGUI::sEngines);

	m_presetsAction = new QAction(tr("Edit &presets"));
	m_presetsAction->setToolTip(tr("Manage engine presets"));
	connect(m_presetsAction, &QAction::triggered, this, &QtChessGUI::sPresets);

	m_quitAction = new QAction(tr("&Quit"));
	m_quitAction->setToolTip(tr("Quit the program"));
	m_quitAction->setShortcut(QKeySequence::Quit);
	connect(m_quitAction, &QAction::triggered, this, &QtChessGUI::sQuit);

	m_aboutAction = new QAction(tr("&About"));
	m_aboutAction->setToolTip(tr("About program"));
	connect(m_aboutAction, &QAction::triggered, this, &QtChessGUI::sAbout);
}

void QtChessGUI::createMenus(void)
{
	// File menu
	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(m_newAction);
	m_fileMenu->addAction(m_closeAction);

	QMenu* const openSubmenu = m_fileMenu->addMenu(tr("&Open"));
	openSubmenu->addAction(m_openFromFileAction);
	openSubmenu->addAction(m_openFromDBAction);

	QMenu* const saveSubmenu = m_fileMenu->addMenu(tr("&Save"));
	saveSubmenu->addAction(m_saveToFileAction);
	saveSubmenu->addAction(m_saveToDBAction);

	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_undoAction);
	m_fileMenu->addAction(m_redoAction);

	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_quitAction);

	// Engines
	m_enginesMenu = menuBar()->addMenu(tr("&Engines"));
	m_enginesMenu->addAction(m_enginesAction);

	// About menu
	m_aboutMenu = menuBar()->addMenu(tr("&About"));
	m_aboutMenu->addAction(m_aboutAction);
}

QString QtChessGUI::getEnginePath(int id)
{
	QSqlQuery query;
	query.prepare("SELECT path FROM engine_players WHERE id = ?");
	query.addBindValue(id);
	if (!query.exec() || !query.first())
		throw std::runtime_error(tr("Error reading engine path from database: ")
			.toStdString() + db.lastError().text().toStdString());
	return query.value("path").toString();
}

void QtChessGUI::sEngineError(BlendXChess::Side side, QString errorText)
{
	QMessageBox::critical(this, tr("Engine error"), errorText);
}

void QtChessGUI::sEngineInfo(BlendXChess::Side side, const SearchInfoDetails& info)
{
	m_engineInfoWidget->setInfo(side, info);
}

void QtChessGUI::sNewGame(void)
{
	m_newDialog->refresh();
	if (m_newDialog->exec() != QDialog::Accepted)
		return;
	try
	{
		if (m_newDialog->pvp())
		{
			m_game->startPVP();
		}
		else if (m_newDialog->withEngine())
		{
			QString enginePath = getEnginePath(m_newDialog->getSelectedEngineId());
			m_game->startWithEngine(m_newDialog->getSelectedSide(), enginePath);
		}
		else
		{
			QString whiteEnginePath = getEnginePath(m_newDialog->getSelectedWhiteEngineId());
			QString blackEnginePath = getEnginePath(m_newDialog->getSelectedWhiteEngineId());
			m_game->startEngineVsEngine(whiteEnginePath, blackEnginePath);
		}
	}
	catch (const std::runtime_error& err)
	{
		QMessageBox::critical(this, tr("Error"), err.what());
	}
}

void QtChessGUI::sAbout(void)
{

}

void QtChessGUI::sQuit(void)
{
	close();
}

void QtChessGUI::sOpenDB(void)
{
	OpenDBBrowser* const dbBrowser = new OpenDBBrowser(this);
	if (dbBrowser->exec() == QDialog::Accepted)
	{
		int id = dbBrowser->getSelectedGameId();
		QSqlQuery query;
		query.prepare("SELECT PGN FROM games WHERE id = ?;");
		query.addBindValue(id);
		if (!query.exec() || !query.first())
		{
			QMessageBox::critical(this, tr("Error"),
				tr("Error reading PGN from database: ") + db.lastError().text());
			return;
		}
		std::istringstream iss(query.value("PGN").toString().toStdString());
		try
		{
			m_game->loadPGN(iss);
			statusBar()->showMessage(tr("Game loaded successfully"));
		}
		catch (const std::runtime_error& err)
		{
			statusBar()->showMessage(tr("Error loading game: ") + err.what());
		}
	}
}

void QtChessGUI::sOpenFile(void)
{
	const QString path = QFileDialog::getOpenFileName(this,
		tr("Open PGN file"), "", tr("All files (*)"));
	if (path.isEmpty())
		return;
	std::ifstream inGame(path.toStdString());
	try
	{
		m_game->loadPGN(inGame);
		statusBar()->showMessage(tr("Game loaded successfully"));
	}
	catch (const std::runtime_error& err)
	{
		statusBar()->showMessage(tr("Error loading game: ") + err.what());
	}
}

void QtChessGUI::sSaveDB(void)
{
	SaveDBBrowser* const dbBrowser = new SaveDBBrowser(this);
	if (dbBrowser->exec() == QDialog::Accepted)
		statusBar()->showMessage(tr("Game saved successfully"));
}

void QtChessGUI::sSaveFile(void)
{
	const QString savePath = QFileDialog::getSaveFileName(this, tr("Save current game"),
		QString(), tr("Portable game notation (*.pgn)"));
	if (savePath.isEmpty())
		return;
	std::ofstream outGame(savePath.toStdString());
	m_game->getGame().writeGame(outGame);
	statusBar()->showMessage(tr("Game saved successfully"));
}

void QtChessGUI::sClose(void)
{
	const auto reply = QMessageBox::question(this, tr("Closing game"),
		tr("Do you want to save the game to file before closing?"),
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if (reply == QMessageBox::Cancel)
		return;
	if (reply == QMessageBox::Yes)
		sSaveFile();
	m_game->closeGame();
	statusBar()->showMessage(tr("Game closed"));
}

void QtChessGUI::sUndo(void)
{
	m_game->undo();
}

void QtChessGUI::sRedo(void)
{
	m_game->redo();
}

void QtChessGUI::sEngines(void)
{
	EnginesBrowser* const enginesBrowser = new EnginesBrowser(this);
	enginesBrowser->exec();
}

void QtChessGUI::sPresets(void)
{
	PresetsBrowser* const presetsBrowser = new PresetsBrowser(this);
	presetsBrowser->exec();
}
