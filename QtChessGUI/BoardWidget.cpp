#include "BoardWidget.h"

using namespace BlendXChess;

BoardWidget::BoardWidget(QWidget* parent)
	: QWidget(parent), m_tileSize(64), m_whiteDown(true), m_selSq(Sq::NONE),
	m_borderWidth(30)
{
	m_game.reset();
	m_boardLUCorner = QPoint(m_borderWidth, m_borderWidth);
	m_tileQSize = QSizeF(m_tileSize, m_tileSize);
	m_boardSize = QSize(FILE_CNT * m_tileSize, RANK_CNT * m_tileSize);
	setFixedSize(
		m_boardSize.width() + 2 * m_borderWidth,
		m_boardSize.height() + 2 * m_borderWidth);
	m_borderLength = m_boardSize.width();
	m_whiteTileImage.load("Images/Board/DefaultTileWhite.png");
	m_blackTileImage.load("Images/Board/DefaultTileBlack.png");
	m_whiteTileImage = m_whiteTileImage.scaled(m_tileQSize.toSize());
	m_blackTileImage = m_blackTileImage.scaled(m_tileQSize.toSize());
	m_svgPieces[W_PAWN].load(QString("Images/Pieces/Cburnett/whitePawn.svg"));
	m_svgPieces[W_KNIGHT].load(QString("Images/Pieces/Cburnett/whiteKnight.svg"));
	m_svgPieces[W_BISHOP].load(QString("Images/Pieces/Cburnett/whiteBishop.svg"));
	m_svgPieces[W_ROOK].load(QString("Images/Pieces/Cburnett/whiteRook.svg"));
	m_svgPieces[W_QUEEN].load(QString("Images/Pieces/Cburnett/whiteQueen.svg"));
	m_svgPieces[W_KING].load(QString("Images/Pieces/Cburnett/whiteKing.svg"));
	m_svgPieces[B_PAWN].load(QString("Images/Pieces/Cburnett/blackPawn.svg"));
	m_svgPieces[B_KNIGHT].load(QString("Images/Pieces/Cburnett/blackKnight.svg"));
	m_svgPieces[B_BISHOP].load(QString("Images/Pieces/Cburnett/blackBishop.svg"));
	m_svgPieces[B_ROOK].load(QString("Images/Pieces/Cburnett/blackRook.svg"));
	m_svgPieces[B_QUEEN].load(QString("Images/Pieces/Cburnett/blackQueen.svg"));
	m_svgPieces[B_KING].load(QString("Images/Pieces/Cburnett/blackKing.svg"));
}

BoardWidget::~BoardWidget()
{}

void BoardWidget::restart(Side userSide)
{
	if (userSide == NULL_COLOR)
		userSide = Side(QDateTime::currentDateTime().time().msec() & 1);
	m_whiteDown = (userSide == WHITE);
	m_game.reset();
}

const BlendXChess::Game& BoardWidget::game(void) const
{
	return m_game;
}

void BoardWidget::undo(void)
{
	if (m_game.UndoMove())
		repaint();
}

void BoardWidget::redo(void)
{
	if (m_game.RedoMove())
		repaint();
}

void BoardWidget::loadPGN(std::istream& inGame)
{
	try
	{
		m_game.loadGame(inGame);
	}
	catch (const std::exception & exc)
	{
		QMessageBox::critical(this, "Error", exc.what());
		return;
	}
}

void BoardWidget::paintEvent(QPaintEvent* eventInfo)
{
	QWidget::paintEvent(eventInfo);
	QPainter painter;
	painter.begin(this);
	// Draw the border
	for (int col = 0; col < 8; ++col)
	{
		painter.drawText(QRect(
			m_borderWidth + col * m_tileSize, 0, m_tileSize, m_borderWidth),
			QString(fileToAN(fileFromCol(col))), QTextOption(Qt::AlignCenter)
		);
		painter.drawText(QRect(
			m_borderWidth + col * m_tileSize, m_boardLUCorner.y() + m_boardSize.height(),
			m_tileSize, m_borderWidth),
			QString(fileToAN(fileFromCol(col))), QTextOption(Qt::AlignCenter)
		);
	}
	for (int row = 0; row < 8; ++row)
	{
		painter.drawText(QRect(
			0, m_borderWidth + row * m_tileSize, m_borderWidth, m_tileSize),
			QString(rankToAN(rankFromRow(row))), QTextOption(Qt::AlignCenter)
		);
		painter.drawText(QRect(
			m_boardLUCorner.x() + m_boardSize.width(), m_borderWidth + row * m_tileSize,
			m_borderWidth, m_tileSize),
			QString(rankToAN(rankFromRow(row))), QTextOption(Qt::AlignCenter)
		);
	}
	// Draw turn indicator
	m_svgPieces[makePiece(m_game.getPosition().getTurn(), KING)].render(
		&painter, QRect(0, 0, m_borderWidth, m_borderWidth));
	// Draw the board
	const Position& board = m_game.getPosition();
	for (int row = 0; row < 8; ++row) // NOT rank
		for (int col = 0; col < 8; ++col) // NOT file
		{
			const Square sq = squareByTileCoord(row, col);
			// Draw a tile
			QPoint tilePos(col * m_tileSize, row * m_tileSize);
			tilePos += m_boardLUCorner;
			painter.drawImage(tilePos, sq.color() == WHITE ? m_whiteTileImage : m_blackTileImage);
			// Draw a piece
			const Piece p = board[sq];
			if (p != PIECE_NULL)
				m_svgPieces[p].render(&painter, QRectF(tilePos, m_tileQSize));
		}
	// Highlight selected square if there is one
	if (m_selSq != Sq::NONE)
	{
		painter.setOpacity(0.25);
		painter.setBrush(Qt::blue);
		painter.setPen(Qt::blue);
		painter.drawRect(QRect(tilePointBySquare(m_selSq), m_tileQSize.toSize()));
	}
	painter.end();
}

void BoardWidget::resizeEvent(QResizeEvent* eventInfo)
{
	QWidget::resizeEvent(eventInfo);
}

void BoardWidget::mousePressEvent(QMouseEvent* eventInfo)
{
	const Square sq = squareByPoint(eventInfo->pos());
	if (eventInfo->button() == Qt::MouseButton::LeftButton)
	{
		if (sq == m_selSq)
			return;
		if (sq == Sq::NONE)
		{
			if (m_selSq != Sq::NONE)
			{
				m_selSq = Sq::NONE;
				repaint();
			}
			return;
		}
		const Position& board = m_game.getPosition();
		if (m_selSq == Sq::NONE)
		{
			if (board[sq] != PIECE_NULL)
			{
				m_selSq = sq;
				repaint();
			}
		}
		else
		{
			Move candidateMove = Move(m_selSq, sq);
			// NOT just DoMove candidateMove, since it (NOW) expects correct
			// move flags, which we haven't set
			if (m_game.DoMove(candidateMove.toUCI(), FMT_UCI))
				m_selSq = Sq::NONE;
			else if (board[sq] != PIECE_NULL)
				m_selSq = sq;
			else
				m_selSq = Sq::NONE;
			repaint();
		}
	}
	else if (eventInfo->button() == Qt::MouseButton::RightButton)
	{

	}
}

int BoardWidget::fileFromCol(int col) const
{
	return m_whiteDown ? col : FILE_CNT - 1 - col;
}

int BoardWidget::rankFromRow(int row) const
{
	return m_whiteDown ? RANK_CNT - 1 - row : row;
}

Square BoardWidget::squareByPoint(QPoint point) const
{
	const QPoint boardPoint = point - m_boardLUCorner;
	// Because integer division is rounded towards zero, we need following check
	if (boardPoint.x() < 0 || boardPoint.y() < 0)
		return Sq::NONE;
	return squareByTileCoord(
		boardPoint.y() / m_tileSize,
		boardPoint.x() / m_tileSize);
}

Square BoardWidget::squareByTileCoord(int row, int col) const
{
	int rank, file;
	if (m_whiteDown)
		rank = RANK_CNT - 1 - row, file = col;
	else
		rank = row, file = FILE_CNT - 1 - col;
	if (!validRank(rank) || !validFile(file))
		return Sq::NONE;
	return Square(rank, file);
}

std::pair<int, int> BoardWidget::tileCoordBySquare(Square sq) const
{
	const int
		rank = sq.rank(),
		file = sq.file();
	int row, col;
	if (m_whiteDown)
		row = 7 - rank, col = file;
	else
		row = rank, col = 7 - file;
	return { row, col };
}

QPoint BoardWidget::tilePointBySquare(Square sq) const
{
	const auto [row, col] = tileCoordBySquare(sq);
	return QPoint(col * m_tileSize, row * m_tileSize) + m_boardLUCorner;
}
