#include "BoardWidget.h"

BoardWidget::BoardWidget(QWidget* parent)
	: QWidget(parent), m_tileSize(50)
{}

BoardWidget::~BoardWidget()
{}

void BoardWidget::paintEvent(QPaintEvent* eventInfo)
{
	QWidget::paintEvent(eventInfo);
	QPainter painter;
	painter.begin(this);
	bool blackTile;
	QPoint tilePos; // Left upper corner
	for (int rank = 0; rank < 8; ++rank)
		for (int col = 0; col < 8; ++col)
		{
			blackTile = (rank & 1) ^ (col & 1);
			tilePos.setX(col * m_tileSize);
			tilePos.setY((7 - rank) * m_tileSize);
			painter.drawImage(tilePos, blackTile ? m_blackTileImage : m_whiteTileImage);


		}
	painter.end();
}

void BoardWidget::resizeEvent(QResizeEvent* eventInfo)
{
	QWidget::resizeEvent(eventInfo);
}

void BoardWidget::mousePressEvent(QMouseEvent* eventInfo)
{

}
