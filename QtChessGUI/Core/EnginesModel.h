#pragma once

#include <unordered_map>
#include <QAbstractTableModel>
#include "UCIEngine.h"

class PresetsModel;

class EnginesModel
	: public QAbstractTableModel
{
	Q_OBJECT

public:
	static constexpr int COLUMN_COUNT = 1;

	struct Item
	{
		inline Item(const EngineInfo& info, PresetsModel* presets)
			: info(info), presets(presets)
		{}

		EngineInfo info;
		PresetsModel* presets;
	};

	EnginesModel(QString path, QObject* parent = nullptr);
	~EnginesModel();

	// QAbstractTableModel
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	// Adds row with given engine info to model
	// Returns index of the new row or -1 if engine name is empty or not unique
	int addRow(const EngineInfo& info);
	// Erases row and returns whether this was successful
	bool eraseRow(int row);
	// Sets given row data to given engine info (must have unique name)
	// Returns whether there was an error
	bool setEngine(int row, const EngineInfo& info);

	// void loadFromSQLite();
	void loadFromJSON(const QString& path);
	void saveToJSON(const QString& path) const;

	const Item& operator[](int idx) const;
	Item& operator[](int idx);

	const EnginesModel::Item* getByName(const QString& name) const;
	EnginesModel::Item* getByName(const QString& name);
	void updateEngine(const EngineInfo& info);
private:
	std::vector<Item> m_data;
};
