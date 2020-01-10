#pragma once

#include <QtWidgets>
#include <QDialog>

#include "Core/Game.h"

class EnginePreset;
class EnginesModel;
class EnginesBrowser;
class PresetsModel;
class PresetsBrowser;

class PresetSelector : public QWidget
{
	Q_OBJECT

public:
	PresetSelector(EnginesModel* enginesModel, QWidget* parent = nullptr);
	~PresetSelector();

	inline QString getCurrentEngineId() const;
	inline QString getCurrentPresetId() const;
private slots:
	void sChange();
private:
	QString m_currentEngineId;
	QString m_currentPresetId;
	EnginesModel* m_enginesModel;
	QLabel* m_currentLabel;
	QPushButton* m_browsePB;
};

class NewGameDialog : public QDialog
{
	Q_OBJECT

public:
	NewGameDialog(::Game::GameType gameType,
		EnginesModel* presets = nullptr, QWidget* parent = nullptr);
	~NewGameDialog();

	BlendXChess::Side getSelectedSide() const; // Valid only for withEngine
	
	// Get created game. You should change its parent since by default it is child of the dialog
	// If the dialog was accepted, game is ready to start (via startGame())
	::Game* getGame() const;
	//QString getSelectedPresetId() const; // Valid only for withEngine
	//QString getSelectedWhitePresetId() const; // Valid only for engineVsEngine
	//QString getSelectedBlackPresetId() const; // Valid only for engineVsEngine
	inline bool timeControlOn() const;
private slots:
	void sOk();
private:
	UCIEngine* launchEngine(const QString& engineID, const QString& presetID);

	::Game::GameType m_gameType;
	::Game* m_game;
	EnginesModel* m_engines;
	QGroupBox* m_timeControlGB;
	QSpinBox* m_initialSB;
	QSpinBox* m_incrementSB;
	QComboBox* m_sideCB;
	PresetSelector* m_engineSelector;
	PresetSelector* m_whiteEngineSelector;
	PresetSelector* m_blackEngineSelector;
};

inline QString PresetSelector::getCurrentEngineId() const
{
	return m_currentEngineId;
}

inline QString PresetSelector::getCurrentPresetId() const
{
	return m_currentPresetId;
}

inline bool NewGameDialog::timeControlOn() const
{
	return m_timeControlGB->isChecked();
}