#include "QtChessGUI.h"
#include "Engine/engine.h"

QtChessGUI::QtChessGUI(QWidget *parent)
	: QMainWindow(parent)
{
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

void QtChessGUI::createActions(void)
{
	m_newAction = new QAction("&New");
	m_newAction->setToolTip("Start new game");
	m_newAction->setShortcut(QKeySequence::New);
	connect(m_newAction, &QAction::triggered, this, &QtChessGUI::sNewGame);

	m_openFromDBAction = new QAction("&Open from &DB");
	m_openFromDBAction->setToolTip("Browse data from database and select the game to view");
	connect(m_openFromDBAction, &QAction::triggered, this, &QtChessGUI::sOpenDB);

	m_openFromFileAction = new QAction("&Open from file");
	m_openFromFileAction->setToolTip("Open game from PGN file");
	connect(m_openFromFileAction, &QAction::triggered, this, &QtChessGUI::sOpenFile);

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

}

void QtChessGUI::sOpenFile(void)
{

}

void QtChessGUI::sUndo(void)
{
	m_boardWidget->undo();
}

void QtChessGUI::sRedo(void)
{
	m_boardWidget->redo();
}
