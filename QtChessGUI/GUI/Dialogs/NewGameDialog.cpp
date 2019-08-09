#include "NewGameDialog.h"

using namespace BlendXChess;

NewGameDialog::NewGameDialog(QWidget *parent)
	: QDialog(parent)
{
	QPushButton* okButton = new QPushButton("&Ok");
	QPushButton* cancelButton = new QPushButton("&Cancel");
	m_pvp = new QRadioButton("Pvp");
	m_withEngine = new QRadioButton("With engine");
	m_engineVsEngine = new QRadioButton("Engine vs engine");
	m_sideCB = new QComboBox;
	m_engineCB = new QComboBox;
	m_whiteEngineCB = new QComboBox;
	m_blackEngineCB = new QComboBox;

	QGroupBox* typeGB = new QGroupBox("Game type");
	QHBoxLayout* typeGBLayout = new QHBoxLayout;
	typeGBLayout->addWidget(m_pvp);
	typeGBLayout->addWidget(m_withEngine);
	typeGBLayout->addWidget(m_engineVsEngine);
	typeGB->setLayout(typeGBLayout);

	m_sideCB->addItem("White", BlendXChess::WHITE);
	m_sideCB->addItem("Black", BlendXChess::BLACK);
	m_sideCB->addItem("Random", BlendXChess::NULL_COLOR);

	pvpW = new QGroupBox("Game options"); // now empty

	withEngineW = new QGroupBox("Game options");
	QFormLayout* withEngineLayout = new QFormLayout;
	withEngineLayout->addRow("Your side: ", m_sideCB);
	withEngineLayout->addRow("Engine: ", m_engineCB);
	withEngineW->setLayout(withEngineLayout);

	engineVsEngineW = new QGroupBox("Game options");
	QFormLayout* engineVsEngineLayout = new QFormLayout;
	engineVsEngineLayout->addRow("White engine: ", m_whiteEngineCB);
	engineVsEngineLayout->addRow("Black engine: ", m_blackEngineCB);
	engineVsEngineW->setLayout(engineVsEngineLayout);

	QHBoxLayout* okCancelLayout = new QHBoxLayout;
	okCancelLayout->addWidget(okButton);
	okCancelLayout->addWidget(cancelButton);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(typeGB);
	mainLayout->addWidget(pvpW);
	mainLayout->addWidget(withEngineW);
	mainLayout->addWidget(engineVsEngineW);
	mainLayout->addLayout(okCancelLayout);
	setLayout(mainLayout);

	m_sideCB->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
	m_engineCB->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
	m_whiteEngineCB->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
	m_blackEngineCB->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

	m_enginesModel = new QSqlQueryModel(this);
	m_enginesModel->setQuery("SELECT id, name FROM engine_players");
	m_engineCB->setModel(m_enginesModel);
	m_engineCB->setModelColumn(1);
	m_whiteEngineCB->setModel(m_enginesModel);
	m_whiteEngineCB->setModelColumn(1);
	m_blackEngineCB->setModel(m_enginesModel);
	m_blackEngineCB->setModelColumn(1);

	connect(okButton, &QPushButton::clicked, this, &NewGameDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &NewGameDialog::reject);
	connect(m_pvp, &QRadioButton::toggled, this, &NewGameDialog::sTypeToggled);
	connect(m_withEngine, &QRadioButton::toggled, this, &NewGameDialog::sTypeToggled);
	connect(m_engineVsEngine, &QRadioButton::toggled, this, &NewGameDialog::sTypeToggled);

	m_pvp->setChecked(true); // after connect's
}

NewGameDialog::~NewGameDialog()
{}

Side NewGameDialog::getSelectedSide(void) const
{
	return (Side)m_sideCB->currentData().toInt();
}

int NewGameDialog::getSelectedEngineId(void) const
{
	auto model = m_engineCB->model();
	return model->data(model->index(m_engineCB->currentIndex(), 0)).toInt();
}

int NewGameDialog::getSelectedWhiteEngineId(void) const
{
	auto model = m_whiteEngineCB->model();
	return model->data(model->index(m_whiteEngineCB->currentIndex(), 0)).toInt();
}

int NewGameDialog::getSelectedBlackEngineId(void) const
{
	auto model = m_blackEngineCB->model();
	return model->data(model->index(m_blackEngineCB->currentIndex(), 0)).toInt();
}

void NewGameDialog::refresh(void)
{
	m_enginesModel->setQuery(m_enginesModel->query().executedQuery());
}

void NewGameDialog::sTypeToggled(bool checked)
{
	if (!checked) // we should continue only for one (eg, now checked) radio button
		return;
	pvpW->hide();
	withEngineW->hide();
	engineVsEngineW->hide();
	if (m_pvp->isChecked())
		pvpW->show();
	else if (m_withEngine->isChecked())
		withEngineW->show();
	else
		engineVsEngineW->show();
	adjustSize();
}
