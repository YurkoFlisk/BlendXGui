#pragma once
#include <QtWidgets>
#include <QtSvg>
#include <map>
#include <memory>
#include "Engine/engine.h"
#include "EngineParams.h"

class BoardWidget : public QWidget
{
	Q_OBJECT

public:
	enum class GameType {
		None, PlayerVsPlayer, PlayerVsEngine, EngineVsEngine
	};

	BoardWidget(QWidget *parent);
	~BoardWidget(void);

	const BlendXChess::Game& game(void) const;
	void closeGame(void);
	void startPVP(void);
	void startWithEngine(BlendXChess::Side userSide, QString enginePath);
	void startEngineVsEngine(QString whiteEnginePath, QString blackEnginePath);
	void undo(void);
	void redo(void);
	bool loadPGN(std::istream& inGame);
protected:
	void paintEvent(QPaintEvent* eventInfo) override;
	void resizeEvent(QResizeEvent* eventInfo) override;
	void mousePressEvent(QMouseEvent* eventInfo) override;

	// Starting game when all necessary conditions (eg, engines are set up) are met
	void startGame(void);
	void launchEngine(BlendXChess::Side side, QString path);
	void loadEngineOptions(UCIEngine* engine);
	void eventCallback(UCIEngine* sender, const UCIEventInfo* eventInfo);

	int fileFromCol(int col) const;
	int rankFromRow(int row) const;
	BlendXChess::Square squareByPoint(QPoint point) const;
	BlendXChess::Square squareByTileCoord(int row, int col) const;
	std::pair<int, int> tileCoordBySquare(BlendXChess::Square sq) const; // row, col
	QPoint tilePointBySquare(BlendXChess::Square sq) const;

	GameType m_gameType; // Type of current game
	BlendXChess::Game m_game; // Game object
	BlendXChess::Side m_userSide; // Side of user (if game type is PlayerVsEngine)
	BlendXChess::Square m_selSq; // Selected square (NOT tile)
	std::map<BlendXChess::Piece, QSvgRenderer> m_svgPieces; // Svg images of pieces
	UCIEngine m_engineProc[BlendXChess::COLOR_CNT]; // Engines for sides
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
