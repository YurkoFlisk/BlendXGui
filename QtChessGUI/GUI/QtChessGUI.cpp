#include <fstream>
#include <sstream>
#include "QtChessGUI.h"
#include "BoardWidget.h"
#include "EngineInfoWidget.h"
#include "EnginesBrowser.h"
#include "PresetsBrowser.h"
#include "Engine/engine.h"
#include "Core/EnginesModel.h"
#include "Dialogs/NewGameDialog.h"
#include "Dialogs/SaveDBBrowser.h"

namespace BXC = BlendXChess;

QtChessGUI::QtChessGUI(QWidget* parent)
	: QMainWindow(parent), m_game(nullptr)
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

	m_clockTimer = new QTimer(this);
	for (int i = 0; i < 2; ++i)
		m_clockDisplay[i] = new QLCDNumber(this);
	(void)connect(m_clockTimer, &QTimer::timeout, this, &QtChessGUI::sRefreshClock);
	clearClocks();

	m_engineInfoWidget = new EngineInfoWidget(this);
	m_boardWidget = new BoardWidget(m_game, this);

	m_engines = new EnginesModel("engines.json", this);

	BXC::Game::initialize();
	setGame(new Game(this));

	QVBoxLayout* boardClockLayout = new QVBoxLayout;
	boardClockLayout->addWidget(m_clockDisplay[0]);
	boardClockLayout->addWidget(m_boardWidget);
	boardClockLayout->addWidget(m_clockDisplay[1]);

	QWidget* const centralWidget = new QWidget;
	QHBoxLayout* const mainLayout = new QHBoxLayout;
	mainLayout->addLayout(boardClockLayout);
	mainLayout->addWidget(m_engineInfoWidget);
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);
	
	createActions();
	createMenus();
	
	(void)connect(m_game, &Game::readyToStart, m_game, &Game::startGame);
	m_game->preparePVP();
}

QtChessGUI::~QtChessGUI(void)
{
	m_engines->saveToJSON("engines.JSON");
}

void QtChessGUI::createActions(void)
{
	m_newPVPGameAction = new QAction(tr("&PVP"));
	m_newPVPGameAction->setToolTip(tr("Start new PVP game"));
	m_newPVPGameAction->setShortcut(QKeySequence::New);
	(void)connect(m_newPVPGameAction, &QAction::triggered, this, &QtChessGUI::sNewPVPGame);

	m_newPVEGameAction = new QAction(tr("&VS Engine"));
	m_newPVEGameAction->setToolTip(tr("Start new game against engine"));
	m_newPVEGameAction->setShortcut(QKeySequence::New);
	(void)connect(m_newPVEGameAction, &QAction::triggered, this, &QtChessGUI::sNewPVEGame);

	m_newEVEGameAction = new QAction(tr("&Engine VS Engine"));
	m_newEVEGameAction->setToolTip(tr("Start new game between engines"));
	m_newEVEGameAction->setShortcut(QKeySequence::New);
	(void)connect(m_newEVEGameAction, &QAction::triggered, this, &QtChessGUI::sNewEVEGame);

	/*m_openFromDBAction = new QAction(tr("Open from &DB"));
	m_openFromDBAction->setToolTip(tr("Browse data from database and select the game to view"));
	connect(m_openFromDBAction, &QAction::triggered, this, &QtChessGUI::sOpenDB);*/

	m_openFromFileAction = new QAction(tr("Open from &file"));
	m_openFromFileAction->setToolTip(tr("Open game from PGN file"));
	(void)connect(m_openFromFileAction, &QAction::triggered, this, &QtChessGUI::sOpenFile);

	/*m_saveToDBAction = new QAction(tr("&Save to DB"));
	m_saveToDBAction->setToolTip(tr("Save to DB in PGN format"));
	connect(m_saveToDBAction, &QAction::triggered, this, &QtChessGUI::sSaveDB);*/

	m_saveToFileAction = new QAction(tr("&Save to file"));
	m_saveToFileAction->setToolTip(tr("Save to file in PGN format"));
	(void)connect(m_saveToFileAction, &QAction::triggered, this, &QtChessGUI::sSaveFile);

	m_undoAction = new QAction(tr("&Undo"));
	m_undoAction->setToolTip(tr("Undo the last move"));
	m_undoAction->setShortcut(QKeySequence::Undo);
	(void)connect(m_undoAction, &QAction::triggered, this, &QtChessGUI::sUndo);

	m_redoAction = new QAction(tr("&Redo"));
	m_redoAction->setToolTip(tr("Redo the lastly undone move"));
	m_redoAction->setShortcut(QKeySequence::Redo);
	(void)connect(m_redoAction, &QAction::triggered, this, &QtChessGUI::sRedo);

	m_closeAction = new QAction(tr("&Close"));
	m_closeAction->setToolTip(tr("Close current game"));
	m_closeAction->setShortcut(QKeySequence::Close);
	(void)connect(m_closeAction, &QAction::triggered, this, &QtChessGUI::sClose);

	m_enginesAction = new QAction(tr("E&dit engines"));
	m_enginesAction->setToolTip(tr("Manage engines"));
	(void)connect(m_enginesAction, &QAction::triggered, this, &QtChessGUI::sEngines);

	m_quitAction = new QAction(tr("&Quit"));
	m_quitAction->setToolTip(tr("Quit the program"));
	m_quitAction->setShortcut(QKeySequence::Quit);
	(void)connect(m_quitAction, &QAction::triggered, this, &QtChessGUI::sQuit);

	m_aboutAction = new QAction(tr("&About"));
	m_aboutAction->setToolTip(tr("About program"));
	(void)connect(m_aboutAction, &QAction::triggered, this, &QtChessGUI::sAbout);
}

void QtChessGUI::createMenus(void)
{
	// File menu
	m_fileMenu = menuBar()->addMenu(tr("&File"));

	QMenu* const newGameSubmenu = m_fileMenu->addMenu(tr("New &game"));
	newGameSubmenu->addAction(m_newPVPGameAction);
	newGameSubmenu->addAction(m_newPVEGameAction);
	newGameSubmenu->addAction(m_newEVEGameAction);

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

void QtChessGUI::clearClocks()
{
	m_clockTimer->stop();
	for (int i = 0; i < 2; ++i)
	{
		auto& clockDisplay = m_clockDisplay[i];
		QString str = "00:00";
		clockDisplay->setDigitCount(str.size());
		clockDisplay->display(str);
	}
}

void QtChessGUI::setGame(Game* game)
{
	if (m_game == game)
		return;
	if (m_game)
		delete m_game;
	m_game = game;
	if (!game)
		return;
	game->setParent(this);
	(void)connect(game, &Game::engineErrorSignal, this, &QtChessGUI::sEngineError);
	(void)connect(game, &Game::searchInfoSignal, this, &QtChessGUI::sSearchInfo);
	(void)connect(game, &Game::engineInitSignal, m_engines, &EnginesModel::updateEngine);
	m_boardWidget->setGame(game);
	m_clockTimer->start();
	m_game->startGame();
}

void QtChessGUI::sRefreshClock()
{
	using namespace std::chrono_literals;
	for (BXC::Side side : {BXC::WHITE, BXC::BLACK})
	{
		auto& clockDisplay = m_clockDisplay[
			m_boardWidget->getWhiteDown() ? BXC::opposite(side) : side];
		const auto clockTime = m_game->getClock(side).remainingTime();
		QString str;
		if (clockTime < 30s)
		{
			const int secs = clockTime.count() / 1000;
			const int msecs = clockTime.count() - secs * 1000;
			str = QString("00:%1.%2")
				.arg(secs, 2, 10, (QChar)'0').arg(msecs, 3, 10, (QChar)'0');
			if (m_clockTimer->interval() != 20)
				m_clockTimer->setInterval(20);
		}
		else
		{
			int secs = clockTime.count() / 1000;
			const int mins = secs / 60;
			secs -= mins * 60;
			str = QString("%1:%2")
				.arg(mins, 2, 10, (QChar)'0').arg(secs, 2, 10, (QChar)'0');
			if (m_clockTimer->interval() != 200)
				m_clockTimer->setInterval(200);
		}
		clockDisplay->setDigitCount(str.count());
		clockDisplay->display(str);
	}
}

void QtChessGUI::sEngineError(BlendXChess::Side side, QString errorText)
{
	QMessageBox::critical(this, tr("Engine error"), errorText);
}

void QtChessGUI::sSearchInfo(BlendXChess::Side side, const SearchInfoDetails& info)
{
	m_engineInfoWidget->setInfo(side, info);
}

void QtChessGUI::sNewPVPGame()
{
	NewGameDialog ngd(Game::GameType::PlayerVsPlayer, m_engines);
	if (ngd.exec() == QDialog::Accepted)
		setGame(ngd.getGame());
}

void QtChessGUI::sNewPVEGame()
{
	NewGameDialog ngd(Game::GameType::PlayerVsEngine, m_engines);
	if (ngd.exec() == QDialog::Accepted)
		setGame(ngd.getGame());
}

void QtChessGUI::sNewEVEGame()
{
	NewGameDialog ngd(Game::GameType::EngineVsEngine, m_engines);
	if (ngd.exec() == QDialog::Accepted)
		setGame(ngd.getGame());
}

//void QtChessGUI::sNewGame(void)
//{
//	m_newDialog->refresh();
//	if (m_newDialog->exec() != QDialog::Accepted)
//		return;
//	try
//	{
//		if (m_newDialog->pvp())
//		{
//			m_game->startPVP();
//		}
//		else if (m_newDialog->withEngine())
//		{
//			QString enginePath = getEnginePath(m_newDialog->getSelectedEngineId());
//			m_game->startWithEngine(m_newDialog->getSelectedSide(), enginePath);
//		}
//		else
//		{
//			QString whiteEnginePath = getEnginePath(m_newDialog->getSelectedWhiteEngineId());
//			QString blackEnginePath = getEnginePath(m_newDialog->getSelectedWhiteEngineId());
//			m_game->startEngineVsEngine(whiteEnginePath, blackEnginePath);
//		}
//	}
//	catch (const std::runtime_error& err)
//	{
//		QMessageBox::critical(this, tr("Error"), err.what());
//	}
//}

void QtChessGUI::sAbout(void)
{

}

void QtChessGUI::sQuit(void)
{
	close();
}

//void QtChessGUI::sOpenDB(void)
//{
//	OpenDBBrowser* const dbBrowser = new OpenDBBrowser(this);
//	if (dbBrowser->exec() == QDialog::Accepted)
//	{
//		int id = dbBrowser->getSelectedGameId();
//		QSqlQuery query;
//		query.prepare("SELECT PGN FROM games WHERE id = ?;");
//		query.addBindValue(id);
//		if (!query.exec() || !query.first())
//		{
//			QMessageBox::critical(this, tr("Error"),
//				tr("Error reading PGN from database: ") + db.lastError().text());
//			return;
//		}
//		std::istringstream iss(query.value("PGN").toString().toStdString());
//		try
//		{
//			m_game->loadPGN(iss);
//			statusBar()->showMessage(tr("Game loaded successfully"));
//		}
//		catch (const std::runtime_error& err)
//		{
//			statusBar()->showMessage(tr("Error loading game: ") + err.what());
//		}
//	}
//}

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

//void QtChessGUI::sSaveDB(void)
//{
//	SaveDBBrowser* const dbBrowser = new SaveDBBrowser(this);
//	if (dbBrowser->exec() == QDialog::Accepted)
//		statusBar()->showMessage(tr("Game saved successfully"));
//}

void QtChessGUI::sSaveFile(void)
{
	if (!m_game)
		return;
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
	if (!m_game)
		return;
	const auto reply = QMessageBox::question(this, tr("Closing game"),
		tr("Do you want to save the game to file before closing?"),
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if (reply == QMessageBox::Cancel)
		return;
	if (reply == QMessageBox::Yes)
		sSaveFile();
	clearClocks();
	m_game->closeGame();
	statusBar()->showMessage(tr("Game closed"));
}

void QtChessGUI::sUndo(void)
{
	if (m_game)
		m_game->undo();
}

void QtChessGUI::sRedo(void)
{
	if (m_game)
		m_game->redo();
}

void QtChessGUI::sEngines(void)
{
	EnginesBrowser enginesBrowser(m_engines);
	enginesBrowser.exec();
}
