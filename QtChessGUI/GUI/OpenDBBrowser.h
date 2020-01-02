#pragma once

#include <QtWidgets>

class OpenDBBrowser : public QDialog
{
	Q_OBJECT

public:
	OpenDBBrowser(QWidget *parent);
	~OpenDBBrowser();
	bool isOK(void) const;
	int getSelectedGameId(void) const;
	void resizeEvent(QResizeEvent*) override;
private:
	void sViewClicked(const QModelIndex& index);

	bool ok;
	int m_selectedGameId;
	QString qString;
	QSqlQueryModel* model;
	QTableView* view;
};
