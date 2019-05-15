#include <fstream>
#include <sstream>
#include "QtChessGUI.h"
#include "Engine/engine.h"

QtChessGUI::QtChessGUI(QWidget* parent)
	: QMainWindow(parent)
{
	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName("localhost");
	db.setDatabaseName("chessdb");
	db.setUserName("root");
	db.setPassword("LamboV3n3n0");
	if (!db.open())
	{
		QMessageBox::warning(this, "Error", "Could not connect to database: " + db.lastError().text());
		return;
	}

	BlendXChess::Game::initialize();
	m_newDialog = new NewGameDialog(this);
	m_boardWidget = new BoardWidget(this);
	QWidget* centralWidget = new QWidget;
	QHBoxLayout* mainLayout = new QHBoxLayout;

	mainLayout->addWidget(m_boardWidget);

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
	m_newAction = new QAction("&New");
	m_newAction->setToolTip("Start new game");
	m_newAction->setShortcut(QKeySequence::New);
	connect(m_newAction, &QAction::triggered, this, &QtChessGUI::sNewGame);

	m_openFromDBAction = new QAction("Open from &DB");
	m_openFromDBAction->setToolTip("Browse data from database and select the game to view");
	connect(m_openFromDBAction, &QAction::triggered, this, &QtChessGUI::sOpenDB);

	m_openFromFileAction = new QAction("Open from &file");
	m_openFromFileAction->setToolTip("Open game from PGN file");
	connect(m_openFromFileAction, &QAction::triggered, this, &QtChessGUI::sOpenFile);

	m_saveToDBAction = new QAction("&Save to DB");
	m_saveToDBAction->setToolTip("Save to DB in PGN format");
	connect(m_saveToDBAction, &QAction::triggered, this, &QtChessGUI::sSaveDB);

	m_saveToFileAction = new QAction("&Save to file");
	m_saveToFileAction->setToolTip("Save to file in PGN format");
	connect(m_saveToFileAction, &QAction::triggered, this, &QtChessGUI::sSaveFile);

	m_undoAction = new QAction("&Undo");
	m_undoAction->setToolTip("Undo the last move");
	m_undoAction->setShortcut(QKeySequence::Undo);
	connect(m_undoAction, &QAction::triggered, this, &QtChessGUI::sUndo);

	m_redoAction = new QAction("&Redo");
	m_redoAction->setToolTip("Redo the lastly undone move");
	m_redoAction->setShortcut(QKeySequence::Redo);
	connect(m_redoAction, &QAction::triggered, this, &QtChessGUI::sRedo);

	m_quitAction = new QAction("&Quit");
	m_quitAction->setToolTip("Quit the program");
	m_quitAction->setShortcut(QKeySequence::Quit);
	connect(m_quitAction, &QAction::triggered, this, &QtChessGUI::sQuit);

	m_aboutAction = new QAction("&About");
	m_aboutAction->setToolTip("About program");
	connect(m_aboutAction, &QAction::triggered, this, &QtChessGUI::sAbout);
}

void QtChessGUI::createMenus(void)
{
	// File menu
	m_fileMenu = menuBar()->addMenu("&File");
	m_fileMenu->addAction(m_newAction);

	QMenu* openSubmenu = m_fileMenu->addMenu("&Open");
	openSubmenu->addAction(m_openFromFileAction);
	openSubmenu->addAction(m_openFromDBAction);

	QMenu* saveSubmenu = m_fileMenu->addMenu("&Save");
	saveSubmenu->addAction(m_saveToFileAction);
	saveSubmenu->addAction(m_saveToDBAction);

	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_undoAction);
	m_fileMenu->addAction(m_redoAction);

	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_quitAction);
	// About menu
	m_aboutMenu = menuBar()->addMenu("&About");
	m_aboutMenu->addAction(m_aboutAction);
}

void QtChessGUI::sNewGame(void)
{
	if (m_newDialog->exec() == QDialog::Accepted)
		m_boardWidget->restart(m_newDialog->getSelectedSide());
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
	OpenDBBrowser* m_DBBrowser = new OpenDBBrowser(nullptr);
	if (m_DBBrowser->exec() == QDialog::Accepted)
	{
		int id = m_DBBrowser->getSelectedGameId();
		QSqlQuery query;
		query.prepare("SELECT PGN FROM games WHERE id = ?;");
		query.addBindValue(id);
		if (!query.exec() || !query.first())
		{
			QMessageBox::critical(this, "Error", "Error reading PGN from database: "
				+ db.lastError().text());
			return;
		}
		std::istringstream iss(query.value("PGN").toString().toStdString());
		m_boardWidget->loadPGN(iss);
	}
}

void QtChessGUI::sOpenFile(void)
{
	QString path = QFileDialog::getOpenFileName(this, "Open PGN file", "", "All files (*)");
	if (path.isEmpty())
		return;
	std::ifstream inGame(path.toStdString());
	m_boardWidget->loadPGN(inGame);
}

void QtChessGUI::sSaveDB(void)
{

}

void QtChessGUI::sSaveFile(void)
{
	QString savePath = QFileDialog::getSaveFileName(this, "Save current game", QString(),
		"Portable game notation (*.pgn)");
	if (savePath.isEmpty())
		return;
	std::ofstream outGame(savePath.toStdString());
	m_boardWidget->game().writeGame(outGame);
}

void QtChessGUI::sUndo(void)
{
	m_boardWidget->undo();
}

void QtChessGUI::sRedo(void)
{
	m_boardWidget->redo();
}
