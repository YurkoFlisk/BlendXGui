#include "PresetsModel.h"
#include "EnginesModel.h"

EnginePreset EnginePreset::defaultFor(const EngineInfo& engineInfo, const QString& name)
{
	EnginePreset ret{ .name = name };
	for (const auto& [name, optionInfo] : engineInfo.options)
		ret.optionValues[name] = optionInfo.getDefault();
	return ret;
}

PresetsModel::PresetsModel(EnginesModel& engines, const QString& engineName)
	: QAbstractTableModel(&engines), m_engines(engines), m_engineName(engineName)
{}

PresetsModel::PresetsModel(const std::vector<EnginePreset>& presetsObj,
	EnginesModel& engines, const QString& engineName)
	: QAbstractTableModel(&engines), m_engines(engines), m_engineName(engineName),
	m_data(presetsObj)
{}

PresetsModel::PresetsModel(const QJsonArray& presetsArr,
	EnginesModel& engines, const EngineInfo& ei)
	: QAbstractTableModel(&engines), m_engines(engines), m_engineName(ei.name)
{
	// engineInfo() may not work now
	loadFromJSON(presetsArr, ei);
}

PresetsModel::~PresetsModel() = default;

int PresetsModel::rowCount(const QModelIndex& parent) const
{
	return m_data.size();
}

int PresetsModel::columnCount(const QModelIndex& parent) const
{
	return COLUMN_COUNT;
}

QVariant PresetsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();
	const int row = index.row(), column = index.column();
	if (row < 0 || m_data.size() <= row || column < 0 || COLUMN_COUNT <= column)
		return QVariant();

	const auto& preset = m_data[row];
	if (role == Qt::DisplayRole || role == Qt::EditRole)
		switch (column)
		{
		case 0: return preset.name;
		default: return QVariant();
		}
	else
		return QVariant();
}

const EnginePreset& PresetsModel::operator[](int idx) const
{
	return m_data[idx];
}

EnginePreset& PresetsModel::operator[](int idx)
{
	return const_cast<EnginePreset&>(
		const_cast<const PresetsModel*>(this)->operator[](idx));
}

int PresetsModel::addRow(const EnginePreset& preset)
{
	if (preset.name.isEmpty())
		return -1;
	if (findByName(preset.name) != -1)
		return -1;

	beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
	m_data.push_back(preset);
	endInsertRows();

	return static_cast<int>(m_data.size() - 1);
}

bool PresetsModel::eraseRow(int row)
{
	if (row < 0 || m_data.size() <= row)
		return false;

	beginRemoveRows(QModelIndex(), row, row);
	m_data.erase(m_data.begin() + row);
	endRemoveRows();

	return true;
}

int PresetsModel::findByName(const QString& name)
{
	auto it = std::find_if(m_data.begin(), m_data.end(),
		[&](const EnginePreset& ep) {
			return ep.name == name;
		});
	if (it == m_data.end())
		return -1;
	return static_cast<int>(it - m_data.begin());
}

QModelIndex PresetsModel::findByNameQMI(const QString& name)
{
	const int row = findByName(name);
	return row == -1 ? QModelIndex() : index(row, 0);
}

bool PresetsModel::setPreset(int row, const EnginePreset& preset)
{
	if (row < 0 || m_data.size() <= row)
		return false;
	if (preset.name.isEmpty())
		return false;

	const int nameRow = findByName(preset.name);
	if (nameRow != -1 && nameRow != row)
		return false;

	m_data[row] = preset;
	emit dataChanged(index(row, 0), index(row, 0));
	return true;
}

const EngineInfo* PresetsModel::engineInfo() const
{
	return &m_engines.getByName(m_engineName)->info;
}

void PresetsModel::loadFromJSON(const QJsonArray& presets, const EngineInfo& info)
{	
	for (const auto& presetRef : presets)
	{
		const QJsonObject presetInfo = presetRef.toObject();
		if (!presetInfo.empty())
			m_data.push_back(loadPresetFromJSON(presetInfo, info));
	}
}

QJsonArray PresetsModel::toJSON() const
{
	QJsonArray presetArr;
	for (const auto& preset : m_data)
		presetArr.append(presetToJSON(preset));
	return presetArr;
}

EnginePreset PresetsModel::loadPresetFromJSON(QJsonObject obj, const EngineInfo& info)
{
	EnginePreset ep;
	ep.name = obj["name"].toString();

	if (ep.name.isEmpty())
		throw std::runtime_error(tr(
			"Invalid or missing preset name").toStdString());

	const QJsonObject optionMap = obj["optionValues"].toObject();
	for (const auto& optionName : optionMap.keys())
	{
		const auto& optionV = optionMap[optionName];
		const std::string& optionStrS = optionV.isDouble()
			? std::to_string(optionV.toInt()) : optionV.toVariant().toString().toStdString();
		const std::string& optionNameS = optionName.toStdString();
		if (!ep.optionValues.emplace(optionNameS,
			info.options.at(optionNameS).parseValue(optionStrS)).second)
			throw std::runtime_error(tr(
				"Option name appeared twice in preset definition").toStdString());
	}

	return ep;
}

QJsonObject PresetsModel::presetToJSON(const EnginePreset& preset) const
{
	QJsonObject obj;
	obj["name"] = preset.name;

	QJsonObject optValues;
	for (const auto& [name, val] : preset.optionValues)
		optValues[name.data()] = engineInfo()->options.at(name).toString(val).data();
	obj["optionValues"] = optValues;

	return obj;
}