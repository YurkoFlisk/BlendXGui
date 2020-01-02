#pragma once

#include "UCIEngine.h"
// #include "EnginesModel.h" // Why does IntelliSense go crazy with just a forward declaration?

class EnginesModel;

struct EnginePreset
{
	// Default preset for given engine with given preset name
	static EnginePreset defaultFor(const EngineInfo& engineInfo,
		const QString& name = "Default");

	inline bool operator==(const EnginePreset& ep) const
	{
		return name == ep.name && optionValues == ep.optionValues;
	}
	inline bool operator!=(const EnginePreset& ep) const
	{
		return !(*this == ep);
	}

	QString name; // Id (name) of the preset
	EngineOptionValues optionValues; // Predefined option set for the engine
};

class PresetsModel
	: public QAbstractTableModel
{
public:
	static constexpr int COLUMN_COUNT = 1;

	PresetsModel(EnginesModel& engines, const QString& engineName);
	PresetsModel(const std::vector<EnginePreset>& presetsObj,
		EnginesModel& engines, const QString& engineName);
	PresetsModel(const QJsonArray& presetsObj,
		EnginesModel& engines, const QString& engineName);
	~PresetsModel();

	// QAbstractTableModel
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	const EngineInfo* engineInfo() const;

	const EnginePreset& operator[](int idx) const;
	EnginePreset& operator[](int idx);

	// Adds row with given preset to model
	// Returns index of the new row or -1 if preset name is empty or not unique
	int addRow(const EnginePreset& preset);
	// Erases row and returns whether this was successful
	bool eraseRow(int row);
	// Finds preset by name and returns its index or -1 in case of absence
	int findByName(const QString& name);
	// Sets given row data to given preset (must have unique name)
	// Returns whether there was an error
	bool setPreset(int row, const EnginePreset& preset);

	void loadFromJSON(const QJsonArray& object);
	QJsonArray toJSON() const;
private:
	EnginePreset loadPresetFromJSON(QJsonObject obj);
	QJsonObject presetToJSON(const EnginePreset& preset) const;

	std::vector<EnginePreset> m_data;
	const EnginesModel& m_engines;
	QString m_engineName;
};