#pragma once

#include <QtWidgets>
#include <QDialog>
#include "Engine/basic_types.h"

class EnginePreset;
class PresetsModel;
class PresetsBrowser;

class PresetSelector : public QWidget
{
	Q_OBJECT

public:
	PresetSelector(PresetsBrowser* browser, QWidget* parent = nullptr);
	~PresetSelector();

	inline std::string getCurrentId() const;
private slots:
	void sChange();
private:
	std::string m_currentId;
	PresetsBrowser* m_browser;
	QLabel* m_currentLabel;
	QPushButton* m_browsePB;
};

class NewGameDialog : public QDialog
{
	Q_OBJECT

public:
	NewGameDialog(PresetsModel* presets, QWidget* parent = nullptr);
	~NewGameDialog();

	BlendXChess::Side getSelectedSide() const; // Valid only for withEngine
	std::string getSelectedPresetId() const; // Valid only for withEngine
	std::string getSelectedWhitePresetId() const; // Valid only for engineVsEngine
	std::string getSelectedBlackPresetId() const; // Valid only for engineVsEngine
	void refresh();
	inline bool pvp() const;
	inline bool withEngine() const;
	inline bool engineVsEngine() const;
private slots:
	void sTypeToggled(bool checked);
private:
	PresetsModel* m_presets;
	QWidget* pvpW;
	QWidget* withEngineW;
	QWidget* engineVsEngineW;
	QComboBox* m_sideCB;
	PresetSelector* m_engineSelector;
	PresetSelector* m_whiteEngineSelector;
	PresetSelector* m_blackEngineSelector;
	QRadioButton* m_pvp;
	QRadioButton* m_withEngine;
	QRadioButton* m_engineVsEngine;
};

inline std::string PresetSelector::getCurrentId() const
{
	return m_currentId;
}

inline bool NewGameDialog::pvp(void) const
{
	return m_pvp->isChecked();
}

inline bool NewGameDialog::withEngine(void) const
{
	return m_withEngine->isChecked();
}

inline bool NewGameDialog::engineVsEngine(void) const
{
	return m_engineVsEngine->isChecked();
}