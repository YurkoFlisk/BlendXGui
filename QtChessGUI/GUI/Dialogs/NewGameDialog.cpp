#include "NewGameDialog.h"
#include "GUI/EnginesBrowser.h"
#include "GUI/PresetsBrowser.h"

using namespace BlendXChess;

PresetSelector::PresetSelector(EnginesModel* enginesModel, QWidget* parent)
	: QWidget(parent), m_enginesModel(enginesModel)
{
	m_currentLabel = new QLabel(tr("Current engine/preset: none"));
	m_browsePB = new QPushButton(tr("Change"));

	connect(m_browsePB, &QPushButton::clicked, this, &PresetSelector::sChange);

	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->addWidget(m_currentLabel);
	mainLayout->addWidget(m_browsePB);
	setLayout(mainLayout);
}

PresetSelector::~PresetSelector() = default;

void PresetSelector::sChange()
{
	EnginesBrowser browser(m_enginesModel, true, this);
	browser.setCurrentId(m_currentEngineId);
	browser.setCurrentPresetId(m_currentPresetId);
	if (browser.exec() == QDialog::Accepted)
	{
		m_currentEngineId = browser.getCurrentId();
		m_currentPresetId = browser.getCurrentPresetId();
		m_currentLabel->setText(tr("Current engine/preset: %1/%2")
			.arg(m_currentEngineId).arg(m_currentPresetId));
	}
}

NewGameDialog::NewGameDialog(::Game::GameType gameType,
	EnginesModel* engines, QWidget* parent)
	: QDialog(parent), m_engines(engines), m_gameType(gameType)
{
	m_initialSB = new QSpinBox;
	m_incrementSB = new QSpinBox;
	m_initialSB->setRange(1, 60);
	m_initialSB->setValue(5);
	m_incrementSB->setRange(0, 100);
	m_incrementSB->setValue(3);
	m_timeControlGB = new QGroupBox(tr("Time control"));
	m_timeControlGB->setCheckable(true);
	m_timeControlGB->setChecked(false);
	QFormLayout* timeGBLayout = new QFormLayout;
	timeGBLayout->addRow(tr("Initial (minutes): "), m_initialSB);
	timeGBLayout->addRow(tr("Increment (seconds): "), m_incrementSB);
	m_timeControlGB->setLayout(timeGBLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_timeControlGB);

	if (gameType == ::Game::GameType::PlayerVsEngine)
	{
		m_sideCB = new QComboBox;
		m_sideCB->addItem(tr("White"), WHITE);
		m_sideCB->addItem(tr("Black"), BLACK);
		m_sideCB->addItem(tr("Random"), NULL_COLOR);
		mainLayout->addWidget(m_sideCB);

		m_engineSelector = new PresetSelector(m_engines, this);
		mainLayout->addWidget(m_engineSelector);
	}
	else if (gameType == ::Game::GameType::EngineVsEngine)
	{
		m_whiteEngineSelector = new PresetSelector(m_engines, this);
		m_blackEngineSelector = new PresetSelector(m_engines, this);
		mainLayout->addWidget(m_whiteEngineSelector);
		mainLayout->addWidget(m_blackEngineSelector);
	}

	QPushButton* okButton = new QPushButton(tr("&Start"));
	QPushButton* cancelButton = new QPushButton(tr("&Cancel"));
	QHBoxLayout* okCancelLayout = new QHBoxLayout;
	okCancelLayout->addWidget(okButton);
	okCancelLayout->addWidget(cancelButton);
	mainLayout->addLayout(okCancelLayout);
	setLayout(mainLayout);

	connect(okButton, &QPushButton::clicked, this, &NewGameDialog::sOk);
	connect(cancelButton, &QPushButton::clicked, this, &NewGameDialog::reject);
}

NewGameDialog::~NewGameDialog() = default;

Side NewGameDialog::getSelectedSide() const
{
	return (Side)m_sideCB->currentData().toInt();
}

::Game* NewGameDialog::getGame() const
{
	return m_game;
}

//std::string NewGameDialog::getSelectedPresetId() const
//{
//	return m_engineSelector->getCurrentId();
//}
//
//std::string NewGameDialog::getSelectedWhitePresetId() const
//{
//	return m_whiteEngineSelector->getCurrentId();
//}
//
//std::string NewGameDialog::getSelectedBlackPresetId() const
//{
//	return m_blackEngineSelector->getCurrentId();
//}

void NewGameDialog::sOk()
{
	m_game = new ::Game(this);
	connect(m_game, &::Game::readyToStart, this, [this]() {
		accept();
	});
	if (m_gameType == ::Game::GameType::PlayerVsPlayer)
		m_game->preparePVP();
	else if (m_gameType == ::Game::GameType::PlayerVsEngine)
	{
		QString engineID = m_engineSelector->getCurrentEngineId();
		QString presetID = m_engineSelector->getCurrentPresetId();
		m_game->prepareWithEngine(m_sideCB->currentData().toInt(),
			launchEngine(engineID, presetID));
	}
	else if (m_gameType == ::Game::GameType::EngineVsEngine)
	{
		QString whiteEngineID = m_whiteEngineSelector->getCurrentEngineId();
		QString whitePresetID = m_whiteEngineSelector->getCurrentPresetId();
		QString blackEngineID = m_blackEngineSelector->getCurrentEngineId();
		QString blackPresetID = m_blackEngineSelector->getCurrentPresetId();
		m_game->prepareEngineVsEngine(
			launchEngine(whiteEngineID, whitePresetID),
			launchEngine(blackEngineID, blackPresetID));
	}
}

UCIEngine* NewGameDialog::launchEngine(const QString& engineID, const QString& presetID)
{
	if (engineID.isEmpty() || presetID.isEmpty())
		return nullptr;
	const auto& [engineInfo, presets] = *m_engines->getByName(engineID);
	const EnginePreset& preset = (*presets)[presets->findByName(presetID)];
	UCIEngine* eng = new UCIEngine(engineInfo.path, UCIEngine::LaunchType::Play);
	eng->initialize();
	for (const auto& [name, val] : preset.optionValues)
		eng->setOption(name, val);
	return eng;
}