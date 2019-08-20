#pragma once

#include <unordered_map>
#include <QAbstractTableModel>
#include "UCIEngine.h"

struct EnginePreset
{
	using OptionValues = std::unordered_map<std::string, UciOption::ValueType>;

	QString name;
	QString engineName;
	OptionValues optionValues;
};

class EnginesModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	static constexpr int COLUMN_COUNT = 4;

	EnginesModel(QString path, bool db = false, QObject* parent = nullptr);
	~EnginesModel();

	// QAbstractTableModel
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	// void loadFromSQLite();
	void loadFromJSON(const QString& path);
	void saveToJSON(const QString& path) const;
	const EngineInfo& getByName(const QString& name) const;
private:
	std::vector<EngineInfo> m_data;
};
