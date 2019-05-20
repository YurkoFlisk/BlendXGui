#pragma once

#include <QDialog>
#include <QtWidgets>

class QtChessGUI;
class QSqlQueryModel;

class SaveDBBrowser : public QDialog
{
	Q_OBJECT

public:
	SaveDBBrowser(QtChessGUI* parent);
	~SaveDBBrowser();
protected:
	void sSave(void);
	void sWhiteTypeSel(bool checked);
	void sBlackTypeSel(bool checked);

	QtChessGUI* m_parent;
	QSqlQueryModel* m_enginesModel;
	QSqlQueryModel* m_humansModel;
	QRadioButton* m_whiteHuman;
	QRadioButton* m_blackHuman;
	QRadioButton* m_whiteEngine;
	QRadioButton* m_blackEngine;
	QComboBox* m_whiteName;
	QComboBox* m_blackName;
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;
};
