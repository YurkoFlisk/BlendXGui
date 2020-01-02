#include "EnginesModel.h"
#include "PresetsModel.h"

EnginesModel::EnginesModel(QString path, QObject *parent)
	: QAbstractTableModel(parent)
{
	loadFromJSON(path);
}

EnginesModel::~EnginesModel() = default;

int EnginesModel::rowCount(const QModelIndex& parent) const
{
	return m_data.size();
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
	if (row < 0 || m_data.size() <= row || column < 0 || COLUMN_COUNT <= column)
		return QVariant();

	const auto& engine = m_data[row];
	if (role == Qt::DisplayRole || role == Qt::EditRole)
		switch (column)
		{
		case 0: return engine.info.name;
		case 1: return engine.info.author;
		case 2: return engine.info.path;
		}
	else
		return QAbstractTableModel::data(index, role);
}

int EnginesModel::addRow(const EngineInfo& info)
{
	if (info.name.isEmpty())
		return -1;
	if (getByName(info.name))
		return -1;

	beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
	m_data.emplace_back(info, new PresetsModel(
		{ EnginePreset::defaultFor(info) }, *this, info.name));
	endInsertRows();

	return static_cast<int>(m_data.size() - 1);
}

bool EnginesModel::eraseRow(int row)
{
	if (row < 0 || m_data.size() <= row)
		return false;

	beginRemoveRows(QModelIndex(), row, row);
	m_data.erase(m_data.begin() + row);
	endRemoveRows();

	return true;
}

bool EnginesModel::setEngine(int row, const EngineInfo& info)
{
	if (row < 0 || m_data.size() <= row)
		return false;
	if (info.name.isEmpty())
		return false;

	auto nameRow = getByName(info.name);
	if (nameRow && nameRow != &m_data[row])
		return false;

	auto& item = m_data[row];
	if (item.info.options != info.options)
	{
		delete item.presets;
		item.presets = new PresetsModel(
			{ EnginePreset::defaultFor(info) }, *this, info.name);
	}
	item.info = info;

	emit dataChanged(index(row, 0), index(row, 0));
	return true;
}

void EnginesModel::loadFromJSON(const QString& path)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		throw std::runtime_error(tr("Could not open JSON file").toStdString());

	QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
	QJsonArray engineArr = doc.array();

	for (const auto& engRef : engineArr)
	{
		const QJsonObject engineObj = engRef.toObject();
		if (!engineObj.empty())
		{
			const QJsonObject infoObj = engineObj["info"].toObject();
			const QJsonArray presetsArr = engineObj["presets"].toArray();
			if (infoObj.empty())
				continue;
			const EngineInfo engineInfo = EngineInfo::fromJSON(infoObj);
			m_data.emplace_back(engineInfo,
				new PresetsModel(presetsArr, *this, engineInfo.name));
		}
	}
}

void EnginesModel::saveToJSON(const QString& path) const
{
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly))
		throw std::runtime_error(tr("Could not open JSON file").toStdString());

	QJsonArray enginesArr;
	for (const auto& engine : m_data)
	{
		QJsonObject engineObj;
		engineObj["info"] = engine.info.toJSON();
		engineObj["presets"] = engine.presets->toJSON();
		enginesArr.append(engineObj);
	}

	file.write(QJsonDocument(enginesArr).toJson());
}

const EnginesModel::Item& EnginesModel::operator[](int idx) const
{
	return m_data[idx];
}

EnginesModel::Item& EnginesModel::operator[](int idx)
{
	return const_cast<EnginesModel::Item&>(
		const_cast<const EnginesModel*>(this)->operator[](idx));
}

const EnginesModel::Item* EnginesModel::getByName(const QString& name) const
{
	const auto it = std::find_if(m_data.begin(), m_data.end(),
		[&name](const EngineInfo& ei) {
			return ei.name == name;
		});
	if (it == m_data.end())
		return nullptr;
	return &*it;
}

EnginesModel::Item* EnginesModel::getByName(const QString& name)
{
	return const_cast<EnginesModel::Item*>(
		static_cast<const EnginesModel*>(this)->getByName(name));
}

void EnginesModel::updateEngine(const EngineInfo& newInfo)
{
	EnginesModel::Item* const curItem = getByName(newInfo.name);
	if (curItem == nullptr)
		m_data.emplace_back(newInfo, new PresetsModel(
			{ EnginePreset::defaultFor(newInfo) }, *this, newInfo.name));
	else if (curItem->info == newInfo)
		return;
	else
	{
		curItem->info = newInfo;
		curItem->presets; // TODO: Clear some presets if they become incompatible
	}
}
