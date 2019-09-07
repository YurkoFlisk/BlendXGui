#pragma once

#include <QDialog>
#include "Core/PresetsModel.h"

class EngineParamsWidget : public QWidget
{
	Q_OBJECT

public:
	EngineParamsWidget(const EngineOptions& options,
		const EngineOptionValues* initOptValues, QWidget* parent = nullptr);
	~EngineParamsWidget();

	EngineOptionValues getOptionValues() const;
private:
	std::unordered_map<std::string, std::pair<UciOption::Type, QWidget*>> m_optionEdits;
};

class PresetsBrowser : public QDialog
{
	Q_OBJECT

public:
	PresetsBrowser(PresetsModel* model, bool selecting = false, QWidget *parent = nullptr);
	~PresetsBrowser();

	QString getCurrentId();
private slots:
	void sSelectionChanged(const QItemSelection& selected,
		const QItemSelection& deselected);
	void sAdd();
	void sRemove();
	// Returns whether save was successful
	bool sSave();
private:
	// Checks for unsaved changes in current row and asks the user to confirm them
	// Returns true if there were no changes, changes were saved
	// or the user decided not to save them
	// Returns false otherwise (user decided to cancel the operation or
	// there were changes which could not be saved, e.g. non-unique preset name) 
	bool checkUnsaved();
	// Updates preset widget to properly reflect the currently selected model row
	void updatePresetPropWidget();
	// Gets preset info from current preset properties widget
	EnginePreset getNewPresetInfo();

	bool m_selecting;
	int m_selectedIdx;
	QLineEdit* m_nameLE;
	PresetsModel* m_presets;
	QWidget* m_presetPropWidget;
	QHBoxLayout* m_presetsLayout;
	class QListView* m_presetsLV;
	class EngineParamsWidget* m_params;
};