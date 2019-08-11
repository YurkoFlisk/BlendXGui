#include "EnginesModel.h"

EnginesModel::EnginesModel(QString path, bool db = false, QObject *parent)
	: QAbstractTableModel(parent)
{
	/*if (db)
		loadFromSQLite(path);
	else*/
		loadFromJSON(path);
}

EnginesModel::~EnginesModel()
{
}

int EnginesModel::rowCount(const QModelIndex& parent) const
{
	return engines.size();
}

int EnginesModel::columnCount(const QModelIndex& parent) const
{
	return COLUMN_COUNT;
}

QVariant EnginesModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();
	const int row = index.row(), column = index.column();
	if (row < 0 || engines.size() <= row || column < 0 || COLUMN_COUNT <= column)
		return QVariant();
	const auto& engine = engines[row];
	if (role == Qt::DisplayRole || role == Qt::EditRole)
		switch (column)
		{
		case 0: return engine.name;
		case 1: return engine.path;
		case 2: return engine.author;
		}
	else
		return QAbstractTableModel::data(index, role);
}

void EnginesModel::loadFromJSON(const QString& path)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		throw std::runtime_error(tr("Could not open JSON file").toStdString());

	QJsonDocument doc;
	doc.fromJson(file.readAll());
	QJsonArray enginesArr = doc.array();

	for (const auto& engRef : enginesArr)
	{
		const QJsonObject engineInfo = engRef.toObject();
		if (!engineInfo.empty())
			engines.push_back(EngineInfo::fromJSON(engineInfo));
	}
}

void EnginesModel::saveToJSON(const QString& path) const
{
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly))
		throw std::runtime_error(tr("Could not open JSON file").toStdString());

	QJsonArray enginesArr;
	for (const auto& engine : engines)
		enginesArr.append(engine.toJSON());

	file.write(QJsonDocument(enginesArr).toJson());
}
