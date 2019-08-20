#include "Game.h"

using BlendXChess::opposite;
using BlendXChess::NULL_COLOR;
using BlendXChess::WHITE;
using BlendXChess::BLACK;
using BlendXChess::MoveFormat;
using BlendXChess::GameState;
using BlendXChess::Side;

Game::Game(QObject* parent)
	: QObject(parent), m_userSide(NULL_COLOR), m_gameType(GameType::None)
{
	for (auto& engine : m_engineProc)
		connect(&engine, &UCIEngine::engineSignal,
			this, &Game::engineEventCallback);
}

Game::~Game() = default;

void Game::closeGame(void)
{
	m_game.clear();
	if (m_gameType == GameType::PlayerVsEngine)
		m_engineProc[opposite(m_userSide)].close();
	else if (m_gameType == GameType::EngineVsEngine)
		for (auto& engine : m_engineProc)
			engine.close();
	m_gameType = GameType::None;
}

void Game::startPVP(void)
{
	closeGame();
	m_gameType = GameType::PlayerVsPlayer;
	startGame();
}

void Game::startWithEngine(BlendXChess::Side userSide, QString enginePath)
{
	closeGame();
	if (userSide == NULL_COLOR)
		userSide = Side(QDateTime::currentDateTime().time().msec() & 1); // random
	m_gameType = GameType::PlayerVsEngine;
	m_userSide = userSide;
	launchEngine(opposite(userSide), enginePath);
}

void Game::startEngineVsEngine(QString whiteEnginePath, QString blackEnginePath)
{
	closeGame();
	m_gameType = GameType::EngineVsEngine;
	m_userSide = NULL_COLOR;
	launchEngine(WHITE, whiteEnginePath);
	launchEngine(BLACK, blackEnginePath);
}

void Game::undo(void)
{
	if (!isUserTurn() || !m_game.UndoMove())
		return;
	if (!isUserTurn())
		m_game.UndoMove();
}

void Game::redo(void)
{
	if (!isUserTurn() || !m_game.RedoMove())
		return;
	if (!isUserTurn())
		m_game.RedoMove();
}

void Game::goEngine(BlendXChess::Side side)
{
	if (side == NULL_COLOR)
		return;
	UCIEngine& engine = m_engineProc[side];
	engine.sendPosition(m_game.getPositionFEN());
	engine.sendGo();
}

bool Game::doMove(const std::string& move)
{
	if (!m_game.DoMove(move, MoveFormat::FMT_UCI))
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

bool Game::loadPGN(std::istream& inGame)
{
	try
	{
		m_game.loadGame(inGame);
		return true;
	}
	catch (const std::exception& exc)
	{
		QMessageBox::critical(this, "Error", exc.what());
		return false;
	}
}

void Game::startGame(void)
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
}

void Game::launchEngine(BlendXChess::Side side, QString path)
{
	if (side != WHITE && side != BLACK)
		return;
	UCIEngine& const engine = m_engineProc[side];
	engine.reset(path, UCIEngine::LaunchType::Play);
}

void Game::engineEventCallback(const UCIEventInfo* eventInfo)
{
	UCIEngine* const engine = dynamic_cast<UCIEngine*>(sender());
	if (engine == nullptr)
		return; // Strange condition, should not happen. Maybe throw?

	Side engineSide;
	if (engine == &m_engineProc[WHITE])
		engineSide = WHITE;
	else if (engine == &m_engineProc[BLACK])
		engineSide = BLACK;
	else
		return; // Unknown sender
	switch (eventInfo->type)
	{
	case UCIEventInfo::Type::UciOk:
		updateEngineInfo(engine->getEngineInfo());
		if (engine->getLaunchType() == UCIEngine::LaunchType::Play)
			loadEngineOptions(engine);
		break;
	case UCIEventInfo::Type::ReadyOk:
		if (engine->getState() != UCIEngine::State::Ready)
			break;
		if (m_gameType == GameType::PlayerVsEngine)
			startGame(); // Sender could be only one engine, so start immediately
		else if (m_gameType == GameType::EngineVsEngine)
		{ // Check that both engines are loaded before starting game
			if (m_engineProc[WHITE].getState() == UCIEngine::State::Ready &&
				m_engineProc[BLACK].getState() == UCIEngine::State::Ready)
				startGame();
		}
		break;
	case UCIEventInfo::Type::BestMove:
		if (engineSide != m_game.getPosition().getTurn())
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