#pragma once

#include <QDialog>
#include "Core/EnginesModel.h"

class EnginePreset;
class QLabel;
class QLineEdit;
class QListView;
class QPushButton;
class QItemSelection;

class EnginesBrowser : public QDialog
{
	Q_OBJECT

public:
	EnginesBrowser(EnginesModel* model, bool selecting = false, QWidget* parent = nullptr);
	~EnginesBrowser();

	QString getCurrentId();
	void setCurrentId(const QString& id);
	QString getCurrentPresetId();
	void setCurrentPresetId(const QString& id);
private slots:
	void sSelectionChanged(const QItemSelection& selected,
		const QItemSelection& deselected);
	void sBrowsePath();
	void sOk();
	void sAdd();
	void sRemove();
	void sPresets();
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
	void updateEnginePropWidget();
	// Gets preset info from current preset properties widget
	EngineInfo getNewEngineInfo();
	// Checks values in edits for validity
	bool checkValues(void);
	// Presets of current engine
	PresetsModel* curEnginePresets();
	// Current engine
	EngineInfo* curEngine();
	// Current preset
	EnginePreset* curPreset();

	bool m_selecting;
	int m_selectedIdx;
	QString m_selectedPresetId;
	EngineInfo m_loadedInfo;
	EnginesModel* m_engines;
	QLineEdit* m_nameLE;
	QLineEdit* m_authorLE;
	QLineEdit* m_pathLE;
	QPushButton* m_presetsPB;
	QPushButton* m_browsePathPB;
	QListView* m_enginesLV;
	QLabel* m_nothingSelLabel;
	QLabel* m_selectedPresetLabel;
	QWidget* m_enginePropWidget;
};
