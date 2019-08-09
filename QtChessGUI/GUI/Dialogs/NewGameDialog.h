#pragma once
#include <QtWidgets>
#include <QDialog>
#include <QtSql>
#include "Engine/basic_types.h"

class NewGameDialog : public QDialog
{
	Q_OBJECT

public:
	NewGameDialog(QWidget *parent);
	~NewGameDialog();
	BlendXChess::Side getSelectedSide(void) const; // Valid only for Pvp
	int getSelectedEngineId(void) const; // Valid only for withEngine
	int getSelectedWhiteEngineId(void) const; // Valid only for engineVsEngine
	int getSelectedBlackEngineId(void) const; // Valid only for engineVsEngine
	void refresh(void);
	inline bool pvp(void) const;
	inline bool withEngine(void) const;
	inline bool engineVsEngine(void) const;
private:
	void sTypeToggled(bool checked);

	QSqlQueryModel* m_enginesModel;
	QWidget* pvpW;
	QWidget* withEngineW;
	QWidget* engineVsEngineW;
	QComboBox* m_sideCB;
	QComboBox* m_engineCB;
	QComboBox* m_whiteEngineCB;
	QComboBox* m_blackEngineCB;
	QRadioButton* m_pvp;
	QRadioButton* m_withEngine;
	QRadioButton* m_engineVsEngine;
};

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