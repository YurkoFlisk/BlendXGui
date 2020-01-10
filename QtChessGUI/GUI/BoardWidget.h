#pragma once

#include <QtWidgets>
#include <QtSvg>
#include <map>
#include <memory>
#include "Core/Game.h"

class BoardWidget : public QWidget
{
	Q_OBJECT

public:
	enum class GameType {
		None, PlayerVsPlayer, PlayerVsEngine, EngineVsEngine
	};

	BoardWidget(Game* game, QWidget *parent = nullptr);
	~BoardWidget(void);

	inline bool getWhiteDown() const noexcept;
	inline const Game* getGame() const noexcept;
	inline void setWhiteDown(bool wd);
	void setGame(Game* game);
protected slots:
	void sPositionChanged();
	void sGameFinished();
protected:
	void paintEvent(QPaintEvent* eventInfo) override;
	void resizeEvent(QResizeEvent* eventInfo) override;
	void mousePressEvent(QMouseEvent* eventInfo) override;

	/*void loadEngineOptions(UCIEngine* engine);
	void updateEngineInfo(const EngineInfo& info);*/

	int fileFromCol(int col) const;
	int rankFromRow(int row) const;
	BlendXChess::Square squareByPoint(QPoint point) const;
	BlendXChess::Square squareByTileCoord(int row, int col) const;
	std::pair<int, int> tileCoordBySquare(BlendXChess::Square sq) const; // row, col
	QPoint tilePointBySquare(BlendXChess::Square sq) const;

	Game* m_game; // Game instance
	BlendXChess::Square m_selSq; // Selected square (NOT tile)
	std::map<BlendXChess::Piece, QSvgRenderer> m_svgPieces; // Svg images of pieces
	QImage m_whiteTileImage; // Image of white tile
	QImage m_blackTileImage; // Image of black tile
	QSizeF m_tileQSize; // Size of a tile
	QSize m_boardSize; // Size of the board
	QPoint m_boardLUCorner; // Left upper corner of the board itself (excluding border)
	float m_tileSize; // Length of tile side
	int m_borderWidth; // Width (normal to board side) of each border
	int m_borderLength; // Length (along board side) of each border
	bool m_whiteDown; // Whether board is viewed with first rows in the bottom
};

inline bool BoardWidget::getWhiteDown() const noexcept
{
	return m_whiteDown;
}

inline const Game* BoardWidget::getGame(void) const noexcept
{
	return m_game;
}

inline void BoardWidget::setWhiteDown(bool wd)
{
	m_whiteDown = wd;
}
