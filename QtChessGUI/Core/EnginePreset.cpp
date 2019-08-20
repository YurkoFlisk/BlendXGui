#include "EnginePreset.h"
#include "EnginesModel.h"

PresetsModel::PresetsModel(QString path, EnginesModel& engines)
	: QAbstractTableModel(static_cast<QObject*>(&engines)),
	m_engines(engines)
{
	loadFromJSON(path);
}

PresetsModel::~PresetsModel() = default;

int PresetsModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	return m_data.size();
}

int PresetsModel::columnCount(const QModelIndex& parent = QModelIndex()) const
{
	return COLUMN_COUNT;
}

QVariant PresetsModel::data(const QModelIndex& index, int role = Qt::DisplayRole) const
{
	if (!index.isValid())
		return QVariant();
	const int row = index.row(), column = index.column();
	if (row < 0 || m_data.size() <= row || column < 0 || COLUMN_COUNT <= column)
		return QVariant();

	const auto& preset = m_data[row];
	if (role == Qt::DisplayRole || Qt::EditRole)
		switch (column)
		{
		case 0: return preset.name;
		case 1: return preset.engineName;
		}
	else
		QAbstractTableModel::data(index, role);
}

void PresetsModel::loadFromJSON(const QString& path)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		throw std::runtime_error(tr("Could not open JSON file").toStdString());
	
	QJsonDocument doc;
	doc.fromJson(file.readAll());
	QJsonArray presetArr = doc.array();

	for (const auto& presetRef : presetArr)
	{
		const QJsonObject presetInfo = presetRef.toObject();
		if (!presetInfo.empty())
			m_data.push_back(loadPresetFromJSON(presetInfo));
	}
}

void PresetsModel::saveToJSON(const QString& path) const
{
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly))
		throw std::runtime_error(tr("Could not open JSON file").toStdString());

	QJsonArray presetArr;
	for (const auto& preset : m_data)
		presetArr.append(presetToJSON(preset));

	file.write(QJsonDocument(presetArr).toJson());
}

EnginePreset PresetsModel::loadPresetFromJSON(QJsonObject obj)
{
	EnginePreset ep;
	ep.name = obj["name"].toString();
	ep.engineName = obj["engineName"].toString();

	if (ep.name.isEmpty())
		throw std::runtime_error(QObject::tr(
			"Invalid or missing preset name").toStdString());
	if (ep.engineName.isEmpty())
		throw std::runtime_error(QObject::tr(
			"Invalid or missing engine name").toStdString());

	const QJsonObject optionMap = obj["optionValues"].toObject();
	for (const auto& optionName : optionMap.keys())
	{
		const EngineInfo& ei = m_engines.getByName(ep.engineName);
		const std::string& optionStrS = optionMap[optionName].toVariant().toString().toStdString();
		const std::string& optionNameS = optionName.toStdString();
		if (!ep.optionValues.emplace(optionNameS,
			ei.options.at(optionNameS).parseValue(optionStrS)).second)
			throw std::runtime_error(tr(
				"Option name appeared twice in preset definition").toStdString());
	}

	return ep;
}

QJsonObject PresetsModel::presetToJSON(const EnginePreset& preset) const
{
	QJsonObject obj;
	obj["name"] = preset.name;
	obj["engineName"] = preset.engineName;

	QJsonObject optValues;
	for (const auto& [name, val] : preset.optionValues)
		optValues[name.data()] = m_engines.getByName(
			preset.engineName).options.at(name).toString(val).data();
	obj["optionValues"] = optValues;

	return obj;
}