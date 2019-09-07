#include "NewGameDialog.h"
#include "Core/EnginesModel.h"
#include "Core/PresetsModel.h"
#include "GUI/PresetsBrowser.h"

using namespace BlendXChess;

PresetSelector::PresetSelector(PresetsBrowser* browser, QWidget* parent)
	: QWidget(parent), m_browser(browser)
{
	m_currentLabel = new QLabel(tr("Current preset: none"));
	m_browsePB = new QPushButton(tr("Change"));

	connect(m_browsePB, &QPushButton::clicked, this, PresetSelector::sChange);

	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->addWidget(m_currentLabel);
	mainLayout->addWidget(m_browsePB);
	setLayout(mainLayout);
}

PresetSelector::~PresetSelector() = default;

void PresetSelector::sChange()
{
	if (m_browser->exec() == QDialog::Accepted)
		m_currentId = m_browser->getCurrentId();
}

NewGameDialog::NewGameDialog(PresetsModel* presets, QWidget* parent)
	: QDialog(parent), m_presets(presets)
{
	QPushButton* okButton = new QPushButton(tr("&Ok"));
	QPushButton* cancelButton = new QPushButton(tr("&Cancel"));
	m_pvp = new QRadioButton(tr("Pvp"));
	m_withEngine = new QRadioButton(tr("With engine"));
	m_engineVsEngine = new QRadioButton(tr("Engine vs engine"));
	m_sideCB = new QComboBox;

	PresetsBrowser* presetsBrowser = new PresetsBrowser(presets, true, this);
	m_engineSelector = new PresetSelector(presetsBrowser);
	m_whiteEngineSelector = new PresetSelector(presetsBrowser);
	m_blackEngineSelector = new PresetSelector(presetsBrowser);

	QGroupBox* typeGB = new QGroupBox(tr("Game type"));
	QHBoxLayout* typeGBLayout = new QHBoxLayout;
	typeGBLayout->addWidget(m_pvp);
	typeGBLayout->addWidget(m_withEngine);
	typeGBLayout->addWidget(m_engineVsEngine);
	typeGB->setLayout(typeGBLayout);

	m_sideCB->addItem(tr("White"), BlendXChess::WHITE);
	m_sideCB->addItem(tr("Black"), BlendXChess::BLACK);
	m_sideCB->addItem(tr("Random"), BlendXChess::NULL_COLOR);

	pvpW = new QGroupBox(tr("Game options")); // now empty

	withEngineW = new QGroupBox(tr("Game options"));
	QFormLayout* withEngineLayout = new QFormLayout;
	withEngineLayout->addRow(tr("Your side: "), m_sideCB);
	withEngineLayout->addRow(tr("Preset: "), m_engineSelector);
	withEngineW->setLayout(withEngineLayout);

	engineVsEngineW = new QGroupBox(tr("Game options"));
	QFormLayout* engineVsEngineLayout = new QFormLayout;
	engineVsEngineLayout->addRow(tr("White preset: "), m_whiteEngineSelector);
	engineVsEngineLayout->addRow(tr("Black preset: "), m_blackEngineSelector);
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

	connect(okButton, &QPushButton::clicked, this, &NewGameDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &NewGameDialog::reject);
	connect(m_pvp, &QRadioButton::toggled, this, &NewGameDialog::sTypeToggled);
	connect(m_withEngine, &QRadioButton::toggled, this, &NewGameDialog::sTypeToggled);
	connect(m_engineVsEngine, &QRadioButton::toggled, this, &NewGameDialog::sTypeToggled);

	m_pvp->setChecked(true); // after connect's
}

NewGameDialog::~NewGameDialog() = default;

Side NewGameDialog::getSelectedSide() const
{
	return (Side)m_sideCB->currentData().toInt();
}

std::string NewGameDialog::getSelectedPresetId() const
{
	return m_engineSelector->getCurrentId();
}

std::string NewGameDialog::getSelectedWhitePresetId() const
{
	return m_whiteEngineSelector->getCurrentId();
}

std::string NewGameDialog::getSelectedBlackPresetId() const
{
	return m_blackEngineSelector->getCurrentId();
}

void NewGameDialog::refresh()
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