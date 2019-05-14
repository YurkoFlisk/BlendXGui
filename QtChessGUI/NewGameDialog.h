#pragma once
#include <QtWidgets>
#include <QDialog>
#include "Engine/basic_types.h"

class NewGameDialog : public QDialog
{
	Q_OBJECT

public:
	NewGameDialog(QWidget *parent);
	~NewGameDialog();
	BlendXChess::Side getSelectedSide(void) const;
private:
	QComboBox* m_sideCB;
};
