#include "EngineInfoWidget.h"
#include "Core/UCIEngine.h"

namespace BXC = BlendXChess;

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
	append(QString::fromStdString(str));
}

void EngineInfoWidget::setInfo(BlendXChess::Side side, const SearchInfoDetails& info)
{
	using ScoreType = SearchInfoDetails::ScoreType;

	QString line;
	line += tr("Depth: %1.").arg(info.depth);

	QString scoreStr;
	if (info.scoreType == ScoreType::Cp)
		scoreStr = tr("Score (cp): %1.").arg(info.score);
	else if (info.score < 0)
		scoreStr = tr("Mated in: %1.").arg(-info.score);
	else
		scoreStr = tr("Mate in: %1.").arg(info.score);
	if (info.scoreBound == BXC::BOUND_LOWER)
		scoreStr.prepend(tr("Lowerbound"));
	else if (info.scoreBound == BXC::BOUND_UPPER)
		scoreStr.prepend(tr("Upperbound"));
	line += scoreStr;

	line += tr("Current move: %1.").arg(QString::fromStdString(info.moveStr));
	append(line);
}
