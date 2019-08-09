#include "EngineInfoWidget.h"

EngineInfoWidget::EngineInfoWidget(QWidget *parent)
	: QTextEdit(parent)
{
	setReadOnly(true);
}

EngineInfoWidget::~EngineInfoWidget(void)
{}

void EngineInfoWidget::clear(void)
{
	QTextEdit::clear();
}

void EngineInfoWidget::appendLine(const std::string& str)
{
	QTextEdit::append(QString::fromStdString(str));
}
