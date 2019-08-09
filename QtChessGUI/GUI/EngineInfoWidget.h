#pragma once

#include <QTextEdit>

class EngineInfoWidget : public QTextEdit
{
	Q_OBJECT

public:
	EngineInfoWidget(QWidget *parent);
	~EngineInfoWidget(void);
	void clear(void);
	void appendLine(const std::string& str);
};
