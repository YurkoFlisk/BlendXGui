#include "DBBrowser.h"
#include <fstream>

OpenDBBrowser::OpenDBBrowser(QWidget* parent)
	: QDialog(parent), ok(false)
{
	model = new QSqlQueryModel;
	model->setQuery("call ExtGames()");
	model->setHeaderData(model->record().indexOf("wname"), Qt::Horizontal, "White Name");
	model->setHeaderData(model->record().indexOf("bname"), Qt::Horizontal, "Black Name");
	model->setHeaderData(model->record().indexOf("wtype"), Qt::Horizontal, "White Type");
	model->setHeaderData(model->record().indexOf("btype"), Qt::Horizontal, "Black Type");
	if (model->lastError().isValid())
	{
		QMessageBox::warning(this, "Error", "Could not fetch data from table: "
			+ model->lastError().text());
		return;
	}

	QTableView* view = new QTableView;
	view->setSelectionBehavior(QAbstractItemView::SelectRows);
	view->setModel(model);
	view->hideColumn(0);
	view->hideColumn(1);
	view->hideColumn(2);
	view->hideColumn(3);
	view->hideColumn(model->record().indexOf("PGN"));
	connect(view, &QTableView::doubleClicked, this, &OpenDBBrowser::sViewClicked);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(view);
	setLayout(mainLayout);

	ok = true;
}

OpenDBBrowser::~OpenDBBrowser()
{}

bool OpenDBBrowser::isOK(void) const
{
	return ok;
}

int OpenDBBrowser::getSelectedGameId(void) const
{
	return m_selectedGameId;
}

void OpenDBBrowser::sViewClicked(const QModelIndex& index)
{
	m_selectedGameId = model->data(model->index(index.row(), 0)).toInt();
	accept();
}
