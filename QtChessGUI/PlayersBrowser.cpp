#include "PlayersBrowser.h"
#include <QtWidgets>

PlayersBrowser::PlayersBrowser(QWidget* parent)
	: QDialog(parent), m_selectedEngineId(-1)
{
	enginesModel = new QSqlQueryModel;
	enginesModel->setQuery(R"(
		SELECT
			players.id AS id,
			engine_players.name AS name,
			version,
			persons.name AS authorName,
			programmingLanguage,
			ELO,
			path
		FROM engine_players
		INNER JOIN players ON engine_players.id = players.id
		INNER JOIN persons ON authorId = persons.Id)"
	);
	enginesModel->setHeaderData(enginesModel->record().indexOf("name"), Qt::Horizontal, "Name");
	enginesModel->setHeaderData(enginesModel->record().indexOf("version"), Qt::Horizontal, "Version");
	enginesModel->setHeaderData(enginesModel->record().indexOf("authorName"), Qt::Horizontal, "Author");
	enginesModel->setHeaderData(enginesModel->record().indexOf("programmingLanguage"), Qt::Horizontal, "Language");
	enginesModel->setHeaderData(enginesModel->record().indexOf("path"), Qt::Horizontal, "UCI Executable Path");
	if (enginesModel->lastError().isValid())
	{
		QMessageBox::warning(this, "Error", "Could not fetch data from table: "
			+ enginesModel->lastError().text());
		reject();
		return;
	}

	authorsModel = new QSqlQueryModel;
	authorsModel->setQuery("SELECT id, name FROM persons");

	view = new QTableView;
	view->setModel(enginesModel);
	view->hideColumn(0);
	view->setSelectionBehavior(QAbstractItemView::SelectRows);
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	view->resizeColumnsToContents();
	view->horizontalHeader()->setStretchLastSection(true);

	QVBoxLayout* buttonsLayout = new QVBoxLayout;
	addButton = new QPushButton("Add");
	modifyButton = new QPushButton("Modify");
	removeButton = new QPushButton("Remove");
	closeButton = new QPushButton("Close");
	modifyButton->setDisabled(true);
	removeButton->setDisabled(true);
	buttonsLayout->addWidget(addButton);
	buttonsLayout->addWidget(modifyButton);
	buttonsLayout->addWidget(removeButton);
	buttonsLayout->addWidget(closeButton);

	nameEdit = new QLineEdit;
	versionEdit = new QLineEdit;
	langEdit = new QLineEdit;
	pathEdit = new QLineEdit;
	authorCB = new QComboBox;
	eloSB = new QSpinBox;
	eloSB->setMinimum(0);
	eloSB->setMaximum(5000);
	eloSB->setValue(1500);
	authorCB->setModel(authorsModel);
	authorCB->setModelColumn(1);

	browsePathButton = new QPushButton("Browse...");
	QHBoxLayout* pathLayout = new QHBoxLayout;
	pathLayout->addWidget(pathEdit);
	pathLayout->addWidget(browsePathButton);

	QFormLayout* formLayout = new QFormLayout;
	formLayout->addRow("Name ", nameEdit);
	formLayout->addRow("Version", versionEdit);
	formLayout->addRow("Author", authorCB);
	formLayout->addRow("ELO", eloSB);
	formLayout->addRow("Language", langEdit);
	formLayout->addRow("UCI Executable Path", pathLayout);

	QGroupBox* editForm = new QGroupBox("Engine data");
	editForm->setLayout(formLayout);

	QHBoxLayout* editLayout = new QHBoxLayout;
	editLayout->addWidget(editForm);
	editLayout->addLayout(buttonsLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(view);
	mainLayout->addLayout(editLayout);
	setLayout(mainLayout);

	connect(view->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &PlayersBrowser::sSelectionChanged);
	connect(browsePathButton, &QPushButton::clicked, this, &PlayersBrowser::sBrowsePath);
	connect(addButton, &QPushButton::clicked, this, &PlayersBrowser::sAdd);
	connect(modifyButton, &QPushButton::clicked, this, &PlayersBrowser::sModify);
	connect(removeButton, &QPushButton::clicked, this, &PlayersBrowser::sRemove);
	connect(closeButton, &QPushButton::clicked, this, &PlayersBrowser::reject);
}

PlayersBrowser::~PlayersBrowser(void)
{}

bool PlayersBrowser::checkValues(void)
{
	if (pathEdit->text().isEmpty())
		QMessageBox::warning(this, "Error", "You must provide executable path");
	else if (nameEdit->text().isEmpty())
		QMessageBox::warning(this, "Error", "You must provide name");
	else
		return true;
	return false;
}

void PlayersBrowser::refreshTable(void)
{
	enginesModel->setQuery(enginesModel->query().executedQuery());
}

void PlayersBrowser::sSelectionChanged(const QItemSelection & selected, const QItemSelection&)
{
	if (selected.indexes().isEmpty())
	{
		m_selectedEngineId = -1;
		modifyButton->setDisabled(true);
		removeButton->setDisabled(true);
		nameEdit->clear();
		versionEdit->clear();
		langEdit->clear();
		pathEdit->clear();
		eloSB->setValue(1500);
		authorCB->setCurrentIndex(-1);
		return;
	}
	modifyButton->setEnabled(true);
	removeButton->setEnabled(true);
	const int row = selected.indexes()[0].row();
	m_selectedEngineId = enginesModel->data(enginesModel->index(row, 0)).toInt();
	nameEdit->setText(enginesModel->data(enginesModel->index(row, 1)).toString());
	versionEdit->setText(enginesModel->data(enginesModel->index(row, 2)).toString());
	authorCB->setCurrentText(enginesModel->data(enginesModel->index(row, 3)).toString());
	langEdit->setText(enginesModel->data(enginesModel->index(row, 4)).toString());
	eloSB->setValue(enginesModel->data(enginesModel->index(row, 5)).toInt());
	pathEdit->setText(enginesModel->data(enginesModel->index(row, 6)).toString());
}

void PlayersBrowser::sBrowsePath(void)
{
	QString path = QFileDialog::getOpenFileName(this, "Select UCI Executable",
		pathEdit->text(), "Executable files (*.exe);;All files (*.*)");
	if (path.isEmpty())
		return;
	pathEdit->setText(path);
}

void PlayersBrowser::sAdd(void)
{
	if (m_selectedEngineId != -1 && QMessageBox::question(this, "Operation",
		"Do you really want to add a new engine (maybe you meant 'modify')?",
		QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		return;
	if (!checkValues())
		return;

	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.prepare("INSERT INTO players(ELO) VALUES(?)");
	query.addBindValue(eloSB->value());
	if (!query.exec())
	{
		QSqlDatabase::database().rollback();
		QMessageBox::critical(this, "Operation result",
			"Error storing engine in database: " + query.lastError().text());
		return;
	}

	int playerId = query.lastInsertId().toInt();
	QVariant authorId = (authorCB->currentIndex() == -1 ?
		QVariant() : authorsModel->data(authorsModel->index(authorCB->currentIndex(), 0)).toInt());
	query.prepare("INSERT INTO engine_players(id,authorId,programmingLanguage,name,version,path) VALUES(?,?,?,?,?,?)");
	query.addBindValue(playerId);
	query.addBindValue(authorId);
	query.addBindValue(langEdit->text());
	query.addBindValue(nameEdit->text());
	query.addBindValue(versionEdit->text());
	query.addBindValue(pathEdit->text());
	if (!query.exec())
	{
		QSqlDatabase::database().rollback();
		QMessageBox::critical(this, "Operation result",
			"Error storing engine in database: " + query.lastError().text());
		return;
	}
	QSqlDatabase::database().commit();
	QMessageBox::information(this, "Operation result", "Insert completed successfully");
	refreshTable();
}

void PlayersBrowser::sModify(void)
{
	if (m_selectedEngineId == -1)
		return;
	if (!checkValues())
		return;

	//QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.prepare("UPDATE players SET ELO = ? WHERE id = ?");
	query.addBindValue(eloSB->value());
	query.addBindValue(m_selectedEngineId);
	if (!query.exec())
	{
		//QSqlDatabase::database().rollback();
		QMessageBox::critical(this, "Operation result",
			"Error update engine database: " + query.lastError().text());
		return;
	}

	QVariant authorId = (authorCB->currentIndex() == -1 ?
		QVariant() : authorsModel->data(authorsModel->index(authorCB->currentIndex(), 0)).toInt());
	query.prepare("UPDATE engine_players SET authorId = ?, programmingLanguage = ?, name = ?, version = ?, path = ? WHERE id = ?");
	query.addBindValue(authorId);
	query.addBindValue(langEdit->text());
	query.addBindValue(nameEdit->text());
	query.addBindValue(versionEdit->text());
	query.addBindValue(pathEdit->text());
	query.addBindValue(m_selectedEngineId);
	if (!query.exec())
	{
		//QSqlDatabase::database().rollback();
		QMessageBox::critical(this, "Operation result",
			"Error update engine database: " + query.lastError().text());
		return;
	}
	//QSqlDatabase::database().commit();
	QMessageBox::information(this, "Operation result", "Update completed successfully");
	refreshTable();
}

void PlayersBrowser::sRemove(void)
{
	if (m_selectedEngineId == -1)
		return;

	QSqlDatabase::database().transaction();
	QSqlQuery query;
	query.prepare("DELETE FROM engine_players WHERE id = ?");
	query.addBindValue(m_selectedEngineId);
	if (!query.exec())
	{
		QSqlDatabase::database().rollback();
		QMessageBox::critical(this, "Operation result",
			"Error deleting from engine database: " + query.lastError().text());
		return;
	}

	query.prepare("DELETE FROM players WHERE id = ?");
	query.addBindValue(m_selectedEngineId);
	if (!query.exec())
	{
		QSqlDatabase::database().rollback();
		QMessageBox::critical(this, "Operation result",
			"Error deleting from engine database: " + query.lastError().text());
		return;
	}
	QSqlDatabase::database().commit();
	QMessageBox::information(this, "Operation result", "Delete completed successfully");
	refreshTable();
}
