#pragma once

#include <QTextEdit>
#include "Engine/basic_types.h"

class EngineInfoWidget : public QTextEdit
{
	Q_OBJECT

public:
	EngineInfoWidget(QWidget *parent);
	~EngineInfoWidget();

	void clear();
	void appendLine(const std::string& str);

	void setInfo(BlendXChess::Side side, const struct SearchInfoDetails& info);
};
