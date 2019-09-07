#include "EngineInfoWidget.h"
#include "Core/UCIEngine.h"

EngineInfoWidget::EngineInfoWidget(QWidget *parent)
	: QTextEdit(parent)
{
	setReadOnly(true);
}

EngineInfoWidget::~EngineInfoWidget() = default;

void EngineInfoWidget::clear()
{
	QTextEdit::clear();
}

void EngineInfoWidget::appendLine(const std::string& str)
{
	QTextEdit::append(QString::fromStdString(str));
}

void EngineInfoWidget::setInfo(BlendXChess::Side side, const SearchInfoDetails& info)
{

}
