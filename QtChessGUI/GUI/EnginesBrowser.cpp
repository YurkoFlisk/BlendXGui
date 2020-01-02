#include "EnginesBrowser.h"
#include "PresetsBrowser.h"
#include <QtWidgets>

EnginesBrowser::EnginesBrowser(EnginesModel* model, bool selecting, QWidget* parent)
	: QDialog(parent), m_selecting(selecting), m_engines(model), m_selectedIdx(-1)
{
	m_enginesLV = new QListView;
	m_enginesLV->setModel(m_engines);
	m_enginesLV->setAlternatingRowColors(true);
	connect(m_enginesLV->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &EnginesBrowser::sSelectionChanged);

	m_nothingSelLabel = new QLabel(tr("Select engine to edit"));
	m_nothingSelLabel->setFont(QFont("Times", 16, QFont::Bold));

	m_nameLE = new QLineEdit;
	m_authorLE = new QLineEdit;
	m_pathLE = new QLineEdit;
	m_browsePathPB = new QPushButton(tr("Browse..."));
	m_pathLE->setReadOnly(true);
	QHBoxLayout* pathLayout = new QHBoxLayout;
	pathLayout->addWidget(m_pathLE);
	pathLayout->addWidget(m_browsePathPB);
	QFormLayout* formLayout = new QFormLayout;
	formLayout->addRow(tr("Name"), m_nameLE);
	formLayout->addRow(tr("Author"), m_authorLE);
	formLayout->addRow(tr("Path"), pathLayout);
	m_enginePropWidget->setLayout(formLayout);
	connect(m_browsePathPB, &QPushButton::clicked, this, &EnginesBrowser::sBrowsePath);

	QVBoxLayout* enginePropLayout = new QVBoxLayout;
	enginePropLayout->addWidget(m_nothingSelLabel);
	enginePropLayout->addWidget(m_enginePropWidget);

	QHBoxLayout* enginesLayout = new QHBoxLayout;
	enginesLayout->addWidget(m_enginesLV);
	enginesLayout->addLayout(enginePropLayout);

	QVBoxLayout* buttonsLayout = new QVBoxLayout;
	QPushButton* okPB = new QPushButton(selecting ? tr("Back") : tr("Select"));
	QPushButton* savePB = new QPushButton(tr("Save"));
	QPushButton* addPB = new QPushButton(tr("Add"));
	QPushButton* removePB = new QPushButton(tr("Remove"));
	m_presetsPB = new QPushButton(tr("Presets"));
	buttonsLayout->addWidget(addPB);
	buttonsLayout->addWidget(removePB);
	buttonsLayout->addWidget(savePB);
	buttonsLayout->addWidget(m_browsePathPB);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okPB);
	connect(okPB, &QPushButton::pressed, this, &EnginesBrowser::sOk);
	connect(savePB, &QPushButton::pressed, this, &EnginesBrowser::sSave);
	connect(addPB, &QPushButton::pressed, this, &EnginesBrowser::sAdd);
	connect(removePB, &QPushButton::pressed, this, &EnginesBrowser::sRemove);
	connect(m_presetsPB, &QPushButton::pressed, this, &EnginesBrowser::sPresets);
	if (selecting)
	{
		QPushButton* cancelPB = new QPushButton(tr("Cancel"));
		buttonsLayout->addWidget(cancelPB);
		connect(cancelPB, &QPushButton::pressed, this, &QDialog::reject);
	}

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(enginesLayout);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	updateEnginePropWidget();
}

EnginesBrowser::~EnginesBrowser() = default;

int EnginesBrowser::getCurrentIdx()
{
	return m_selectedIdx;
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
		m_enginePropWidget->hide();
		m_nothingSelLabel->show();
		m_loadedOptions.clear();
	}
	else
	{
		const EngineInfo& engineInfo = (*m_engines)[m_selectedIdx].info;
		m_nameLE->setText(engineInfo.name);
		m_authorLE->setText(engineInfo.author);
		m_pathLE->setText(engineInfo.path);
		m_loadedOptions = engineInfo.options;

		m_nothingSelLabel->hide();
		m_enginePropWidget->show();
	}
}

EngineInfo EnginesBrowser::getNewEngineInfo()
{
	EngineInfo ret;
	ret.name = m_nameLE->text();
	ret.author = m_authorLE->text();
	ret.path = m_pathLE->text();
	ret.options = m_loadedOptions;
	return ret;
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

void EnginesBrowser::sSelectionChanged(const QItemSelection& selected,
	const QItemSelection& deselected)
{
	if (!checkUnsaved())
	{
		m_enginesLV->setCurrentIndex(deselected.indexes().front());
		return;
	}
	if (selected.empty())
		m_selectedIdx = -1;
	else
		m_selectedIdx = selected.indexes().front().row();
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
	m_loadedOptions = uciInfo.options;
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

	int insertedIdx;
	do // Ask the name for new preset until a valid one is given
	{
		bool ok = false;
		const QString name = QInputDialog::getText(this, tr("Engine name"),
			tr("Engine name: "), QLineEdit::Normal, info.uciname, &ok);
		if (!ok)
			return; // Cancelled
		if ((insertedIdx = m_engines->addRow(info)) != -1)
			break;
		QMessageBox::warning(this, tr("Error"), tr("Engine name must be non-empty and unique"));
	} while (true);
	m_selectedIdx = insertedIdx;
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
	PresetsBrowser presetsBrowser((*m_engines)[m_selectedIdx].presets, false, this);
	presetsBrowser.exec();
}

bool EnginesBrowser::sSave()
{
	if (m_selectedIdx == -1 || !checkValues())
		return false;
	if ((*m_engines)[m_selectedIdx].info.options != m_loadedOptions)
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
