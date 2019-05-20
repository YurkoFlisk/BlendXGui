#include <QtSql>
#include "QtChessGUI.h"
#include "SaveDBBrowser.h"

using namespace BlendXChess;

SaveDBBrowser::SaveDBBrowser(QtChessGUI* parent)
	: QDialog(parent), m_parent(parent)
{
	m_okButton = new QPushButton("OK");
	m_cancelButton = new QPushButton("Cancel");
	m_whiteHuman = new QRadioButton("Human");
	m_whiteEngine = new QRadioButton("Engine");
	m_blackHuman = new QRadioButton("Human");
	m_blackEngine = new QRadioButton("Engine");
	m_whiteName = new QComboBox;
	m_blackName = new QComboBox;
	QGroupBox* whiteTypeGB = new QGroupBox("White type");
	QGroupBox* blackTypeGB = new QGroupBox("Black type");

	m_whiteName->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
	m_blackName->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

	m_enginesModel = new QSqlQueryModel(this);
	m_humansModel = new QSqlQueryModel(this);
	m_enginesModel->setQuery("SELECT id, name FROM engine_players");
	m_humansModel->setQuery("SELECT human_players.id, name FROM human_players INNER JOIN persons ON personId = persons.id");

	QVBoxLayout* whiteTypeLayout = new QVBoxLayout;
	whiteTypeLayout->addWidget(m_whiteHuman);
	whiteTypeLayout->addWidget(m_whiteEngine);
	QVBoxLayout* blackTypeLayout = new QVBoxLayout;
	blackTypeLayout->addWidget(m_blackHuman);
	blackTypeLayout->addWidget(m_blackEngine);
	whiteTypeGB->setLayout(whiteTypeLayout);
	blackTypeGB->setLayout(blackTypeLayout);

	QGridLayout* mainLayout = new QGridLayout;
	mainLayout->addWidget(whiteTypeGB, 0, 0);
	mainLayout->addWidget(blackTypeGB, 0, 1);
	mainLayout->addWidget(m_whiteName, 1, 0);
	mainLayout->addWidget(m_blackName, 1, 1);
	mainLayout->addWidget(m_okButton, 2, 0);
	mainLayout->addWidget(m_cancelButton, 2, 1);
	setLayout(mainLayout);

	connect(m_okButton, &QPushButton::clicked, this, &SaveDBBrowser::sSave);
	connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
	connect(m_whiteEngine, &QPushButton::toggled, this, &SaveDBBrowser::sWhiteTypeSel);
	connect(m_whiteHuman, &QPushButton::toggled, this, &SaveDBBrowser::sWhiteTypeSel);
	connect(m_blackEngine, &QPushButton::toggled, this, &SaveDBBrowser::sBlackTypeSel);
	connect(m_blackHuman, &QPushButton::toggled, this, &SaveDBBrowser::sBlackTypeSel);
}

SaveDBBrowser::~SaveDBBrowser()
{}

void SaveDBBrowser::sSave(void)
{
	const Game& game = m_parent->getBoardWidget()->game();
	QString PGN = game.getGame().data(), result;
	switch (game.getGameState())
	{
	case GameState::DRAW: result = "1/2-1/2"; break;
	case GameState::WHITE_WIN: result = "1-0"; break;
	case GameState::BLACK_WIN: result = "0-1"; break;
	default: result = "Active"; break;
	}
	int whiteId = m_whiteName->model()->data(
		m_whiteName->model()->index(m_whiteName->currentIndex(), 0)).toInt();
	int blackId = m_whiteName->model()->data(
		m_whiteName->model()->index(m_blackName->currentIndex(), 0)).toInt();
	QDate date = QDate::currentDate();
	QSqlQuery query;
	query.prepare("INSERT INTO games(whitePlayerId, blackPlayerId, PGN, date, result) VALUES(?, ?, ?, ?, ?)");
	query.addBindValue(whiteId);
	query.addBindValue(blackId);
	query.addBindValue(PGN);
	query.addBindValue(date);
	query.addBindValue(result);
	if (!query.exec())
	{
		QMessageBox::critical(this, "Error storing game in database: ", query.lastError().text());
		return;
	}
	accept();
}

void SaveDBBrowser::sWhiteTypeSel(bool checked)
{
	if (!checked) // we should continue only for one (eg, now checked) radio button
		return;
	if (m_whiteEngine->isChecked())
		m_whiteName->setModel(m_enginesModel);
	else if (m_whiteHuman->isChecked())
		m_whiteName->setModel(m_humansModel);
	m_whiteName->setModelColumn(1);
	adjustSize();
}

void SaveDBBrowser::sBlackTypeSel(bool checked)
{
	if (!checked) // we should continue only for one (eg, now checked) radio button
		return;
	if (m_blackEngine->isChecked())
		m_blackName->setModel(m_enginesModel);
	else if (m_blackHuman->isChecked())
		m_blackName->setModel(m_humansModel);
	m_blackName->setModelColumn(1);
	adjustSize();
}
