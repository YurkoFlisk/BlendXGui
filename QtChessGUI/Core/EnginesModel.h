#pragma once

#include <unordered_map>
#include <QAbstractTableModel>
#include "UCIEngine.h"

class PresetsModel;

class EnginesModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	static constexpr int COLUMN_COUNT = 4;

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

	// void loadFromSQLite();
	void loadFromJSON(const QString& path);
	void saveToJSON(const QString& path) const;

	const EnginesModel::Item* getByName(const QString& name) const;
	EnginesModel::Item* getByName(const QString& name);
	void updateEngine(const EngineInfo& info);
private:
	std::vector<Item> m_data;
};
