#include <QtWidgets>
#include "PresetsBrowser.h"
#include "Dialogs/EngineParamsDialog.h"

EngineParamsWidget::EngineParamsWidget(const EngineOptions& options,
	const EngineOptionValues* initOptValues, QWidget* parent)
	: QWidget(parent)
{
	QFormLayout* optionsLayout = new QFormLayout;
	QGroupBox* optionsGB = new QGroupBox(tr("Options"));
	QCheckBox* checkEdit;
	QComboBox* comboEdit;
	QSpinBox* spinEdit;
	QLineEdit* stringEdit;
	QWidget* editWidget;

	for (const auto& [name, option] : options)
	{
		QString qname = QString::fromStdString(name);
		switch (option.getType())
		{
		case UciOption::Type::Check:
			editWidget = checkEdit = new QCheckBox;
			checkEdit->setChecked(initOptValues
				? std::get<bool>(initOptValues->at(name))
				: option.getDefaultBool());
			break;
		case UciOption::Type::Combo:
			editWidget = comboEdit = new QComboBox;
			for (const auto& comboVar : option.getComboVars())
				comboEdit->addItem(QString::fromStdString(comboVar));
			comboEdit->setCurrentText(QString::fromStdString(initOptValues
				? std::get<std::string>(initOptValues->at(name))
				: option.getDefaultString()));
			break;
		case UciOption::Type::Spin:
			qname += tr("[min: %1, max: %2]").arg(
				QString::number(option.getMin()),
				QString::number(option.getMax()));
			editWidget = spinEdit = new QSpinBox;
			spinEdit->setMinimum(option.getMin());
			spinEdit->setMaximum(option.getMax());
			spinEdit->setValue(initOptValues
				? std::get<int>(initOptValues->at(name))
				: option.getDefaultInt());
			break;
		case UciOption::Type::String:
			editWidget = stringEdit = new QLineEdit;
			stringEdit->setText(QString::fromStdString(initOptValues
				? std::get<std::string>(initOptValues->at(name))
				: option.getDefaultString()));
			break;
		default:
			continue;
		}
		qname += ":";
		optionsLayout->addRow(qname, editWidget);
		m_optionEdits.emplace(name, std::pair{ option.getType(), editWidget });
	}

	setLayout(optionsLayout);
}

EngineParamsWidget::~EngineParamsWidget() = default;

EngineOptionValues EngineParamsWidget::getOptionValues() const
{
	EngineOptionValues ret;
	for (const auto& [name, typeEdit] : m_optionEdits)
	{
		const auto& [type, editWidget] = typeEdit;
		switch (type)
		{
		case UciOption::Type::Check:
			ret[name] = static_cast<QCheckBox*>(editWidget)->isChecked();
			break;
		case UciOption::Type::Combo:
			ret[name] = static_cast<QComboBox*>(editWidget)->currentText().toStdString();
			break;
		case UciOption::Type::Spin:
			ret[name] = static_cast<QSpinBox*>(editWidget)->value();
			break;
		case UciOption::Type::String:
			ret[name] = static_cast<QLineEdit*>(editWidget)->text().toStdString();
			break;
		default:
			throw std::runtime_error("Unexpected option type for editing");
		}
	}
	return ret;
}

PresetsBrowser::PresetsBrowser(PresetsModel* model, bool selecting, QWidget* parent)
	: QDialog(parent), m_selecting(selecting), m_presets(model), m_selectedIdx(-1)
{
	m_presetsLV = new QListView;
	m_presetsLV->setModel(model);
	m_presetsLV->setAlternatingRowColors(true);
	connect(m_presetsLV->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &PresetsBrowser::sSelectionChanged);

	m_presetPropWidget = nullptr;

	m_presetsLayout = new QHBoxLayout;
	m_presetsLayout->addWidget(m_presetsLV);
	updatePresetPropWidget();

	QPushButton* okPB = new QPushButton(selecting ? tr("Back") : tr("Select"));
	QPushButton* savePB = new QPushButton(tr("Save"));
	QPushButton* addPB = new QPushButton(tr("Add"));
	QPushButton* removePB = new QPushButton(tr("Remove"));
	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(addPB);
	buttonsLayout->addWidget(removePB);
	buttonsLayout->addWidget(savePB);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okPB);
	connect(okPB, &QPushButton::pressed, this, &PresetsBrowser::sOk);
	connect(savePB, &QPushButton::pressed, this, &PresetsBrowser::sSave);
	connect(addPB, &QPushButton::pressed, this, &PresetsBrowser::sAdd);
	connect(removePB, &QPushButton::pressed, this, &PresetsBrowser::sRemove);
	if (selecting)
	{
		QPushButton* cancelPB = new QPushButton(tr("Cancel"));
		buttonsLayout->addWidget(cancelPB);
		connect(cancelPB, &QPushButton::pressed, this, &QDialog::reject);
	}

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(m_presetsLayout);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
}

PresetsBrowser::~PresetsBrowser() = default;

int PresetsBrowser::getCurrentIdx()
{
	return m_selectedIdx;
}

void PresetsBrowser::sSelectionChanged(const QItemSelection& selected,
	const QItemSelection& deselected)
{
	if (!checkUnsaved())
	{
		m_presetsLV->setCurrentIndex(deselected.indexes().front());
		return;
	}
	if (selected.empty())
		m_selectedIdx = -1;
	else
		m_selectedIdx = selected.indexes().front().row();
	updatePresetPropWidget();
}

void PresetsBrowser::sOk()
{
	if (!checkUnsaved())
		return;
	accept();
}

void PresetsBrowser::sAdd()
{
	if (!checkUnsaved())
		return;
	int insertedIdx;
	do // Ask the name for new preset until a valid one is given
	{
		bool ok = false;
		const QString name = QInputDialog::getText(this, tr("Preset name"),
			tr("Preset name: "), QLineEdit::Normal, "Default", &ok);
		if (!ok)
			return; // Cancelled
		const EnginePreset preset = EnginePreset::defaultFor(
			*m_presets->engineInfo(), name);
		if ((insertedIdx = m_presets->addRow(preset)) != -1)
			break;
		QMessageBox::warning(this, tr("Error"), tr("Preset name must be non-empty and unique"));
	} while (true);
	m_selectedIdx = insertedIdx;
	updatePresetPropWidget();
}

void PresetsBrowser::sRemove()
{
	if (m_selectedIdx == -1)
		return;
	if (!m_presets->eraseRow(m_selectedIdx))
		return;
	m_selectedIdx = -1;
	updatePresetPropWidget();
}

bool PresetsBrowser::sSave()
{
	if (m_selectedIdx == -1)
		return false;
	if (!m_presets->setPreset(m_selectedIdx, getNewPresetInfo()))
	{
		QMessageBox::warning(this, tr("Saving error"),
			tr("Can't save current preset info. Check that it has unique name"));
		return false;
	}
	return true;
}

bool PresetsBrowser::checkUnsaved()
{
	if (m_selectedIdx == -1)
		return true; // Could not be any changes
	const EnginePreset& oldInfo = (*m_presets)[m_selectedIdx];
	const EnginePreset newInfo = getNewPresetInfo();
	if (oldInfo == newInfo)
		return true; // No changes
	// Unsaved changes are present, so ask the user what to do with them
	QMessageBox msgBox;
	msgBox.setText(tr("Current preset was changed."));
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

void PresetsBrowser::updatePresetPropWidget()
{
	QWidget* newPresetPropWidget = new QWidget;
	QVBoxLayout* presetPropLayout = new QVBoxLayout;
	if (m_selectedIdx == -1)
	{
		QLabel* nothingSelLabel = new QLabel(tr("Select preset to edit"));
		nothingSelLabel->setFont(QFont("Times", 16, QFont::Bold));
		presetPropLayout->addWidget(nothingSelLabel);
		m_params = nullptr;
	}
	else
	{
		const EnginePreset& preset = (*m_presets)[m_selectedIdx];
		const EngineInfo& engineInfo = *m_presets->engineInfo();
		m_nameLE = new QLineEdit(preset.name);
		m_params = new EngineParamsWidget(engineInfo.options, &preset.optionValues);
		presetPropLayout->addWidget(m_nameLE);
		presetPropLayout->addWidget(m_params);
	}
	newPresetPropWidget->setLayout(presetPropLayout);
	// Replace a widget and delete layout item and old widget
	if (m_presetPropWidget)
	{
		delete m_presetsLayout->replaceWidget(m_presetPropWidget, newPresetPropWidget);
		delete m_presetPropWidget;
	}
	else
		m_presetsLayout->addWidget(newPresetPropWidget);
	m_presetPropWidget = newPresetPropWidget;
}

EnginePreset PresetsBrowser::getNewPresetInfo()
{
	EnginePreset preset;
	preset.name = m_nameLE->text();
	preset.optionValues = m_params->getOptionValues();
	return preset;
}
