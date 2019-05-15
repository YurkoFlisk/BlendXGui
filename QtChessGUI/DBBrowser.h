#pragma once

#include <QtWidgets>
#include <QtSql>

class OpenDBBrowser : public QDialog
{
	Q_OBJECT

public:
	OpenDBBrowser(QWidget *parent);
	~OpenDBBrowser();
	bool isOK(void) const;
	int getSelectedGameId(void) const;
private:
	void sViewClicked(const QModelIndex& index);

	bool ok;
	int m_selectedGameId;
	QString qString;
	QSqlQueryModel* model;
};
