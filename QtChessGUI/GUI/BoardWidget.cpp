#include "BoardWidget.h"
#include "EngineInfoWidget.h"
#include "Dialogs/EngineParamsDialog.h"

using namespace BlendXChess;

BoardWidget::BoardWidget(QWidget* parent, EngineInfoWidget* eIW)
	: QWidget(parent), m_tileSize(64), m_whiteDown(true), m_selSq(Sq::NONE),
	m_borderWidth(30), m_userSide(NULL_COLOR), m_gameType(GameType::None),
	m_engineInfoWidget(eIW)
{
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

	startPVP();
}

BoardWidget::~BoardWidget(void)
{}

const BlendXChess::Game& BoardWidget::game(void) const
{
	return m_game;
}

void BoardWidget::closeGame(void)
{
	m_game.clear();
	if (m_gameType == GameType::PlayerVsEngine)
		m_engineProc[opposite(m_userSide)].close();
	else if (m_gameType == GameType::EngineVsEngine)
		for (auto& engine : m_engineProc)
			engine.close();
	m_gameType = GameType::None;
}

void BoardWidget::startPVP(void)
{
	closeGame();
	m_gameType = GameType::PlayerVsPlayer;
	m_whiteDown = true;
	startGame();
}

void BoardWidget::startWithEngine(BlendXChess::Side userSide, QString enginePath)
{
	closeGame();
	if (userSide == NULL_COLOR)
		userSide = Side(QDateTime::currentDateTime().time().msec() & 1); // random
	m_gameType = GameType::PlayerVsEngine;
	m_userSide = userSide;
	m_whiteDown = (userSide == WHITE);
	launchEngine(opposite(userSide), enginePath);
}

void BoardWidget::startEngineVsEngine(QString whiteEnginePath, QString blackEnginePath)
{
	closeGame();
	m_gameType = GameType::EngineVsEngine;
	m_userSide = NULL_COLOR;
	m_whiteDown = true;
	launchEngine(WHITE, whiteEnginePath);
	launchEngine(BLACK, blackEnginePath);
}

void BoardWidget::undo(void)
{
	if (!userMoves() || !m_game.UndoMove())
		return;
	if (!userMoves())
		m_game.UndoMove();
	update();
}

void BoardWidget::redo(void)
{
	if (!userMoves() || !m_game.RedoMove())
		return;
	if (!userMoves())
		m_game.RedoMove();
	update();
}

void BoardWidget::goEngine(BlendXChess::Side side)
{
	if (side == NULL_COLOR)
		return;
	m_engineInfoWidget->clear();
	UCIEngine& engine = m_engineProc[side];
	engine.sendPosition(m_game.getPositionFEN());
	engine.sendGo();
}

bool BoardWidget::doMove(const std::string& move)
{
	if (!m_game.DoMove(move, FMT_UCI))
		return false;
	if (auto gs = m_game.getGameState(); gs != GameState::ACTIVE)
	{
		QMessageBox::information(this, "Game result",
			gs == GameState::WHITE_WIN ? "White won" :
			gs == GameState::BLACK_WIN ? "Black won" :
			gs == GameState::DRAW ? "Draw" : "Undefined");
		return false;
	}
	const Side currentTurn = m_game.getPosition().getTurn();
	if (m_gameType == GameType::PlayerVsEngine)
	{
		const Side engineSide = opposite(m_userSide);
		if (engineSide == currentTurn)
			goEngine(currentTurn);
	}
	else if (m_gameType == GameType::EngineVsEngine)
		goEngine(currentTurn);
	return true;
}

bool BoardWidget::loadPGN(std::istream& inGame)
{
	try
	{
		m_game.loadGame(inGame);
		return true;
	}
	catch (const std::exception & exc)
	{
		QMessageBox::critical(this, "Error", exc.what());
		return false;
	}
}

bool BoardWidget::userMoves(void) const noexcept
{
	return m_gameType == GameType::PlayerVsPlayer
		|| m_game.getPosition().getTurn() == m_userSide;
}

void BoardWidget::paintEvent(QPaintEvent* eventInfo)
{
	QStyleOption opt;
	opt.init(this);
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
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
		else if (userMoves())
		{
			Move candidateMove = Move(m_selSq, sq);
			// NOT just DoMove candidateMove, since it (NOW) expects correct
			// move flags, which we haven't set
			if (doMove(candidateMove.toUCI()))
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

void BoardWidget::startGame(void)
{
	// TODO sendNewGame should be followed by isready-readyok as potentially long operation
	m_game.reset();
	if (m_gameType == GameType::PlayerVsEngine)
	{
		m_engineProc[opposite(m_userSide)].sendNewGame();
		if (m_userSide == BLACK)
		{
			m_engineProc[WHITE].sendPosition("startpos");
			m_engineProc[WHITE].sendGo(7);
		}
	}
	else if (m_gameType == GameType::EngineVsEngine)
	{
		m_engineProc[WHITE].sendNewGame();
		m_engineProc[BLACK].sendNewGame();
		m_engineProc[WHITE].sendPosition("startpos");
		m_engineProc[WHITE].sendGo();
	}
	update();
}

void BoardWidget::launchEngine(BlendXChess::Side side, QString path)
{
	if (side != WHITE && side != BLACK)
		return;
	UCIEngine& engine = m_engineProc[side];
	engine.reset(path, [this](auto&&... params) {eventCallback(params...); });
}

void BoardWidget::loadEngineOptions(UCIEngine* engine)
{
	EngineParamsDialog* engineParamsDialog =
		new EngineParamsDialog(this, engine->getOptions());
	if (engineParamsDialog->exec() != QDialog::Accepted)
		return;
	for (const auto& [optName, _opt_unused] : engine->getOptions())
		engine->setOption(optName, engineParamsDialog->getOptionValue(optName));
	engine->sendIsReady();
}

void BoardWidget::eventCallback(UCIEngine* sender, const UCIEventInfo* eventInfo)
{
	Side senderSide;
	if (sender == &m_engineProc[WHITE])
		senderSide = WHITE;
	else if (sender == &m_engineProc[BLACK])
		senderSide = BLACK;
	else
		return; // Unknown sender
	switch (eventInfo->type)
	{
	case UCIEventInfo::Type::UciOk:
		loadEngineOptions(sender);
		break;
	case UCIEventInfo::Type::ReadyOk:
		if (sender->getState() != UCIEngine::State::Ready)
			break;
		if (m_gameType == GameType::PlayerVsEngine)
			startGame(); // Could be only one engine, so start immediately
		else if (m_gameType == GameType::EngineVsEngine)
		{ // Check that both engines are loaded before starting game
			if (m_engineProc[WHITE].getState() == UCIEngine::State::Ready &&
				m_engineProc[BLACK].getState() == UCIEngine::State::Ready)
				startGame();
		}
		break;
	case UCIEventInfo::Type::BestMove:
		if (senderSide != m_game.getPosition().getTurn())
			return;
		if (!doMove(eventInfo->bestMove))
			return;
		update();
		break;
	case UCIEventInfo::Type::Info:
		m_engineInfoWidget->appendLine(eventInfo->errorText);
		// TEMPORARY
		break;
	case UCIEventInfo::Type::Error:
		QMessageBox::critical(this, "Engine error",
			QString::fromStdString(eventInfo->errorText));
		break;
	}
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
