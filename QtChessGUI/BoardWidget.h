#pragma once
#include <QtWidgets>
#include <QtSvg>

class BoardWidget : public QWidget
{
	Q_OBJECT

public:
	BoardWidget(QWidget *parent);
	~BoardWidget();

	void paintEvent(QPaintEvent* eventInfo) override;
	void resizeEvent(QResizeEvent* eventInfo) override;
	void mousePressEvent(QMouseEvent* eventInfo) override;
protected:
	QImage m_whiteTileImage;
	QImage m_blackTileImage;
	int m_tileSize;
};
