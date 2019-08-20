#pragma once

#include "UCIEngine.h"
#include "EnginesModel.h" // Why does IntelliSense go crazy with just a forward declaration?

class EnginesModel;

struct EnginePreset
{
	QString name;
	QString engineName;
	EngineOptionValues optionValues;
};

class PresetsModel : public QAbstractTableModel
{
public:
	static constexpr int COLUMN_COUNT = 2;

	PresetsModel(QString path, EnginesModel& engines);
	~PresetsModel();

	// QAbstractTableModel
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void loadFromJSON(const QString& path);
	void saveToJSON(const QString& path) const;
private:
	EnginePreset loadPresetFromJSON(QJsonObject obj);
	QJsonObject presetToJSON(const EnginePreset& preset) const;

	std::vector<EnginePreset> m_data;
	const EnginesModel& m_engines;
};