#pragma once

#include <QDialog>
#include "Core/UCIEngine.h"

class EngineParamsDialog : public QDialog
{
	Q_OBJECT

public:
	EngineParamsDialog(QWidget *parent, const EngineOptions& options);
	~EngineParamsDialog(void);
	UciOption::ValueType getOptionValue(const std::string& name);
private:
	std::unordered_map<std::string, std::pair<UciOption::Type, QWidget*>> optionEdits;
	QPushButton* okButton;
	QPushButton* cancelButton;
};
