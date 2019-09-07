#pragma once

#include <QDialog>
#include <QtSql>
#include "Core/EnginesModel.h"

class EnginesBrowser : public QDialog
{
	Q_OBJECT

public:
	EnginesBrowser(QWidget *parent);
	~EnginesBrowser();
private:
	bool checkValues(void);
	void refreshTable(void);

	void sSelectionChanged(const QItemSelection& selected, const QItemSelection&);
	void sBrowsePath(void);
	void sAdd(void);
	void sModify(void);
	void sRemove(void);

	int m_selectedEngineId;
	QSqlQueryModel* enginesModel;
	QSqlQueryModel* authorsModel;
	class QLineEdit* nameEdit;
	class QLineEdit* versionEdit;
	class QComboBox* authorCB;
	class QSpinBox* eloSB;
	class QLineEdit* langEdit;
	class QLineEdit* pathEdit;
	class QPushButton* browsePathButton;
	class QPushButton* addButton;
	class QPushButton* modifyButton;
	class QPushButton* removeButton;
	class QPushButton* closeButton;
	class QTableView* view;
};
