#include "EnginesBrowser.h"
#include "PresetsBrowser.h"
#include <QtWidgets>

EnginesBrowser::EnginesBrowser(EnginesModel* model, bool selecting, QWidget* parent)
	: QDialog(parent), m_selecting(selecting), m_engines(model), m_selectedIdx(-1)
{
	m_enginesLV = new QListView;
	m_enginesLV->setModel(m_engines);
	m_enginesLV->setAlternatingRowColors(true);
	(void)connect(m_enginesLV->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &EnginesBrowser::sSelectionChanged);

	m_nothingSelLabel = new QLabel(tr("Select engine to edit"));
	m_nothingSelLabel->setFont(QFont("Times", 16, QFont::Bold));

	m_nameLE = new QLineEdit;
	m_authorLE = new QLineEdit;
	m_pathLE = new QLineEdit;
	m_browsePathPB = new QPushButton(tr("Browse..."));
	m_enginePropWidget = new QWidget;
	m_pathLE->setReadOnly(true);
	QHBoxLayout* pathLayout = new QHBoxLayout;
	pathLayout->addWidget(m_pathLE);
	pathLayout->addWidget(m_browsePathPB);
	QFormLayout* formLayout = new QFormLayout;
	formLayout->addRow(tr("Name"), m_nameLE);
	formLayout->addRow(tr("Author"), m_authorLE);
	formLayout->addRow(tr("Path"), pathLayout);
	m_enginePropWidget->setLayout(formLayout);
	(void)connect(m_browsePathPB, &QPushButton::clicked, this, &EnginesBrowser::sBrowsePath);

	QHBoxLayout* m_presetSelLayout = new QHBoxLayout;
	m_selectedPresetLabel = new QLabel;
	if (selecting)
		m_presetSelLayout->addWidget(m_selectedPresetLabel);
	m_presetsPB = new QPushButton(tr("Presets"));
	m_presetSelLayout->addWidget(m_presetsPB);

	QVBoxLayout* enginePropLayout = new QVBoxLayout;
	enginePropLayout->addWidget(m_nothingSelLabel);
	enginePropLayout->addWidget(m_enginePropWidget);
	enginePropLayout->addLayout(m_presetSelLayout);

	QHBoxLayout* enginesLayout = new QHBoxLayout;
	enginesLayout->addWidget(m_enginesLV);
	enginesLayout->addLayout(enginePropLayout);

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	QPushButton* okPB = new QPushButton(selecting ? tr("Select") : tr("Back"));
	QPushButton* savePB = new QPushButton(tr("Save"));
	QPushButton* addPB = new QPushButton(tr("Add"));
	QPushButton* removePB = new QPushButton(tr("Remove"));
	buttonsLayout->addWidget(addPB);
	buttonsLayout->addWidget(removePB);
	buttonsLayout->addWidget(savePB);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okPB);
	(void)connect(okPB, &QPushButton::pressed, this, &EnginesBrowser::sOk);
	(void)connect(savePB, &QPushButton::pressed, this, &EnginesBrowser::sSave);
	(void)connect(addPB, &QPushButton::pressed, this, &EnginesBrowser::sAdd);
	(void)connect(removePB, &QPushButton::pressed, this, &EnginesBrowser::sRemove);
	(void)connect(m_presetsPB, &QPushButton::pressed, this, &EnginesBrowser::sPresets);
	if (selecting)
	{
		QPushButton* cancelPB = new QPushButton(tr("Cancel"));
		buttonsLayout->addWidget(cancelPB);
		(void)connect(cancelPB, &QPushButton::pressed, this, &QDialog::reject);
	}

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(enginesLayout);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	updateEnginePropWidget();
}

EnginesBrowser::~EnginesBrowser() = default;

QString EnginesBrowser::getCurrentId()
{
	if (m_selectedIdx == -1)
		return "";
	return curEngine()->name;
}

void EnginesBrowser::setCurrentId(const QString& id)
{
	m_enginesLV->selectionModel()->select(m_engines->findByNameQMI(id),
		QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

QString EnginesBrowser::getCurrentPresetId()
{
	return m_selectedPresetId;
}

void EnginesBrowser::setCurrentPresetId(const QString& id)
{
	m_selectedPresetId = id;
	EnginePreset* const preset = curPreset();
	if (preset == nullptr)
	{
		m_selectedPresetId = "";
		m_selectedPresetLabel->setText(tr("Selected preset: none"));
	}
	else
		m_selectedPresetLabel->setText(tr("Selected preset: %1").arg(preset->name));
}

bool EnginesBrowser::checkUnsaved()
{
	if (m_selectedIdx == -1)
		return true; // Could not be any changes
	const EngineInfo& oldInfo = (*m_engines)[m_selectedIdx].info;
	const EngineInfo newInfo = getNewEngineInfo();
	if (oldInfo == newInfo)
		return true; // No changes
	// Unsaved changes are present, so ask the user what to do with them
	QMessageBox msgBox;
	msgBox.setText(tr("Current engine info was changed."));
	msgBox.setInformativeText(tr("Do you want to save the changes?"));
	msgBox.setStandardButtons(QMessageBox::Save
		| QMessageBox::Discard | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Save);
	switch (msgBox.exec())
	{
	case QMessageBox::Save:
		return sSave(); // If save was unsuccessful, don't proceed with desired operation
	case QMessageBox::Cancel:
		return false; // Operation was canceled
	case QMessageBox::Discard:
		return true; // User decided not to save
	}
	return false; // Should not happen
}

void EnginesBrowser::updateEnginePropWidget()
{
	if (m_selectedIdx == -1)
	{
		setCurrentPresetId("");

		m_enginePropWidget->hide();
		m_nothingSelLabel->show();
		m_loadedInfo = EngineInfo();
	}
	else
	{
		const EngineInfo& engineInfo = *curEngine();
		m_nameLE->setText(engineInfo.name);
		m_authorLE->setText(engineInfo.author);
		m_pathLE->setText(engineInfo.path);
		m_loadedInfo = engineInfo;

		setCurrentPresetId("Default");

		m_nothingSelLabel->hide();
		m_enginePropWidget->show();
	}
}

EngineInfo EnginesBrowser::getNewEngineInfo()
{
	m_loadedInfo.name = m_nameLE->text();
	m_loadedInfo.author = m_authorLE->text();
	m_loadedInfo.path = m_pathLE->text();
	return m_loadedInfo;
}

bool EnginesBrowser::checkValues(void)
{
	if (m_pathLE->text().isEmpty())
		QMessageBox::warning(this, tr("Error"), tr("You must provide executable path"));
	else if (m_nameLE->text().isEmpty())
		QMessageBox::warning(this, tr("Error"), tr("You must provide name"));
	else
		return true;
	return false;
}

PresetsModel* EnginesBrowser::curEnginePresets()
{
	if (m_selectedIdx == -1)
		return nullptr;
	return (*m_engines)[m_selectedIdx].presets;
}

EngineInfo* EnginesBrowser::curEngine()
{
	if (m_selectedIdx == -1)
		return nullptr;
	return &(*m_engines)[m_selectedIdx].info;
}

EnginePreset* EnginesBrowser::curPreset()
{
	PresetsModel* const presets = curEnginePresets();
	if (presets == nullptr)
		return nullptr;
	const int idx = presets->findByName(m_selectedPresetId);
	if (idx == -1)
		return nullptr;
	return &(*presets)[idx];
}

void EnginesBrowser::sSelectionChanged(const QItemSelection& selected,
	const QItemSelection& deselected)
{
	int newIdx;
	if (selected.empty())
		newIdx = -1;
	else
		newIdx = selected.indexes().front().row();
	if (newIdx == m_selectedIdx)
		return;
	if (!checkUnsaved())
	{
		if (!deselected.indexes().empty())
			m_enginesLV->setCurrentIndex(deselected.indexes().front());
		return;
	}
	m_selectedIdx = newIdx;
	updateEnginePropWidget();
}

void EnginesBrowser::sBrowsePath(void)
{
	QString path = QFileDialog::getOpenFileName(this, tr("Select UCI Executable"),
		m_pathLE->text(), tr("Executable files (*.exe);;All files (*.*)"));
	if (path.isEmpty())
		return;

	UCIEngine uciEng(path, UCIEngine::LaunchType::Info);
	uciEng.initialize();
	const EngineInfo& uciInfo = uciEng.getEngineInfo();
	QString m_nameLEText = uciInfo.uciname, m_quthorLEText = m_authorLE->text();
	if (m_nameLE->text() != uciInfo.uciname || m_authorLE->text() != uciInfo.author)
	{
		auto res = QMessageBox::question(this, tr("Naming"), tr(
			"UCI Engine name or author is not the same as currently inputted.\n"
			"Do you want to use UCI's?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (res == QMessageBox::Yes)
		{
			m_nameLE->setText(uciInfo.uciname);
			m_authorLE->setText(uciInfo.author);
		}
		else if (res == QMessageBox::Cancel)
			return;
	}

	m_pathLE->setText(path);
	m_loadedInfo = uciInfo;
}

void EnginesBrowser::sOk()
{
	if (!checkUnsaved())
		return;
	accept();
}

void EnginesBrowser::sAdd(void)
{
	if (!checkUnsaved())
		return;

	QString path = QFileDialog::getOpenFileName(this, tr("Select UCI Executable"),
		m_pathLE->text(), tr("Executable files (*.exe);;All files (*.*)"));
	if (path.isEmpty())
		return;

	UCIEngine uciEng(path, UCIEngine::LaunchType::Info);
	uciEng.initialize();
	const EngineInfo& uciInfo = uciEng.getEngineInfo();

	EngineInfo info;
	info.name = info.uciname = uciInfo.uciname;
	info.author = uciInfo.author;
	info.path = path;
	info.options = uciInfo.options;

	int insertedIdx;
	do // Ask the name for new preset until a valid one is given
	{
		bool ok = false;
		const QString name = QInputDialog::getText(this, tr("Engine name"),
			tr("Engine name: "), QLineEdit::Normal, info.uciname, &ok);
		info.name = name;
		if (!ok)
			return; // Cancelled
		if ((insertedIdx = m_engines->addRow(info)) != -1)
			break;
		QMessageBox::warning(this, tr("Error"), tr("Engine name must be non-empty and unique"));
	} while (true);
	m_enginesLV->setCurrentIndex(m_engines->findByNameQMI(info.name));
	updateEnginePropWidget();
}

void EnginesBrowser::sRemove(void)
{
	if (m_selectedIdx == -1)
		return;
	if (!m_engines->eraseRow(m_selectedIdx))
		return;
	m_selectedIdx = -1;
	updateEnginePropWidget();
}

void EnginesBrowser::sPresets()
{
	if (m_selectedIdx == -1)
		return;
	PresetsBrowser presetsBrowser(curEnginePresets(), m_selecting, this);
	presetsBrowser.setCurrentId(m_selectedPresetId);
	if (presetsBrowser.exec() == QDialog::Accepted && m_selecting)
		setCurrentPresetId(presetsBrowser.getCurrentId());
}

bool EnginesBrowser::sSave()
{
	if (m_selectedIdx == -1 || !checkValues())
		return false;
	if (curEngine()->options != m_loadedInfo.options)
	{
		auto res = QMessageBox::question(this, tr("Options"), tr(
			"UCI Engine option set is not equal to the current option set for this record.\n"
			"Do you want to proceed (all current presets will be lost)?"),
			QMessageBox::Yes | QMessageBox::No);
		if (res == QMessageBox::No)
			return false;
	}
	if (!m_engines->setEngine(m_selectedIdx, getNewEngineInfo()))
		return false;
	return true;
}
