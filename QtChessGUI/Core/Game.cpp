#include "Game.h"

using BlendXChess::opposite;
using BlendXChess::NULL_COLOR;
using BlendXChess::WHITE;
using BlendXChess::BLACK;
using BlendXChess::MoveFormat;
using BlendXChess::GameState;
using BlendXChess::Side;

Game::Game(QObject* parent)
	: QObject(parent), m_userSide(NULL_COLOR), m_gameType(GameType::None),
	m_engineProc {nullptr, nullptr}
{}

Game::~Game() = default;

//void Game::gameLoop()
//{
//	using namespace std::chrono;
//
//	m_game.reset();
//	for (Side side : { WHITE, BLACK })
//		if (isEngineSide(side))
//		{
//			// m_engineProc[side]->initialize();
//			// ... Fill options
//			m_engineProc[side]->sendNewGame();
//		}
//
//	std::unique_lock moveLock(moveMutex);
//
//	while (m_game.getGameState() == BlendXChess::GameState::ACTIVE)
//	{
//		const Side turn = m_game.getPosition().getTurn();
//		Clock& clock = m_clock[turn];
//		if (isEngineTurn())
//		{
//			m_engineProc[turn]->sendPosition(m_game.getPositionFEN());
//			m_engineProc[turn]->go(clock);
//		}
//		else // User turn
//		{
//			userMoved = false;
//			clock.toggle();
//			userMovedCV.wait_for(moveLock, clock.remainingTime(), [this] {return userMoved; });
//			clock.toggle();
//		}
//		if (clock.timeout())
//		{
//			m_game.timeout();
//			break;
//		}
//	}
//}

void Game::closeGame(void)
{
	/*if (m_gameType == GameType::PlayerVsEngine)
		closeEngine(opposite(m_userSide));
	else if (m_gameType == GameType::EngineVsEngine)
	{
		closeEngine(WHITE);
		closeEngine(BLACK);
	}*/
	m_engineProc[WHITE] = m_engineProc[BLACK] = nullptr;

	m_game.clear();
	m_gameType = GameType::None;
}

void Game::startPVP(void)
{
	closeGame();
	m_gameType = GameType::PlayerVsPlayer;
	startGame();
}

void Game::startWithEngine(BlendXChess::Side userSide, UCIEngine* engine)
{
	closeGame();
	if (userSide == NULL_COLOR)
		userSide = Side(QDateTime::currentDateTime().time().msec() & 1); // random
	m_gameType = GameType::PlayerVsEngine;
	m_userSide = userSide;
	m_engineProc[opposite(m_userSide)] = engine;
	connect(engine, &UCIEngine::engineSignal, this, &Game::engineEventCallback);

	m_engGameStarted[WHITE] = m_engGameStarted[BLACK] = false; // In fact, redundant
	engine->sendNewGame();
}

void Game::startEngineVsEngine(UCIEngine* whiteEngine, UCIEngine* blackEngine)
{
	closeGame();
	m_gameType = GameType::EngineVsEngine;
	m_userSide = NULL_COLOR;
	m_engineProc[WHITE] = whiteEngine;
	m_engineProc[BLACK] = blackEngine;
	connect(whiteEngine, &UCIEngine::engineSignal, this, &Game::engineEventCallback);
	connect(blackEngine, &UCIEngine::engineSignal, this, &Game::engineEventCallback);

	m_engGameStarted[WHITE] = m_engGameStarted[BLACK] = false;
	whiteEngine->sendNewGame();
	blackEngine->sendNewGame();
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
	UCIEngine* const engine = m_engineProc[side];
	engine->sendPosition(m_game.getPositionFEN());
	engine->sendGo();
}

bool Game::doMove(const std::string& move)
{
	Clock* clock = &m_clock[currentTurn()]; // Before move

	if (!m_game.DoMove(move, MoveFormat::FMT_UCI))
		return false;
	clock->pause();

	if (auto gs = m_game.getGameState(); gs != GameState::ACTIVE)
	{
		emit gameFinishedSignal();
		return false;
	}
	else
		emit positionChangedSignal();

	clock = &m_clock[currentTurn()]; // After move

	if (m_gameType == GameType::PlayerVsEngine)
	{
		const Side engineSide = opposite(m_userSide);
		if (engineSide == currentTurn())
			goEngine(currentTurn());
	}
	else if (m_gameType == GameType::EngineVsEngine)
		goEngine(currentTurn());
	clock->resume();
	return true;
}

void Game::loadPGN(std::istream& inGame)
{
	m_game.loadGame(inGame);
	emit positionChangedSignal();
}

//void Game::closeEngine(BlendXChess::Side side)
//{
//	UCIEngine*& engine = m_engineProc[opposite(m_userSide)];
//	if (engine == nullptr)
//		return;
//	engine->close();
//	disconnect(engine, &UCIEngine::engineSignal, this, &Game::engineEventCallback);
//	engine = nullptr;
//}

void Game::startGame(void)
{
	// TODO sendNewGame should be followed by isready-readyok as potentially long operation
	m_game.reset();
	if (m_gameType == GameType::PlayerVsEngine)
	{
		if (m_userSide == BLACK)
		{
			m_engineProc[WHITE]->sendPosition("startpos");
			m_engineProc[WHITE]->sendGo(7);
		}
	}
	else if (m_gameType == GameType::EngineVsEngine)
	{
		m_engineProc[WHITE]->sendPosition("startpos");
		m_engineProc[WHITE]->sendGo();
	}
	for (Side side : {WHITE, BLACK})
		connect(&m_clock[side], &Clock::timeout,
			[this, side] {playerTimeout(side); });
	m_clock[WHITE].resume();
}

void Game::playerTimeout(BlendXChess::Side side)
{
	Q_ASSERT(side == currentTurn());
	if (isEngineTurn())
		m_engineProc[currentTurn()]->sendStop();
	m_game.timeout();
	emit gameFinishedSignal();
}

void Game::engineEventCallback(const EngineEvent* eventInfo)
{
	UCIEngine* const engine = dynamic_cast<UCIEngine*>(sender());
	if (engine == nullptr)
		return; // Strange condition, should not happen. Maybe throw?

	Side engineSide;
	if (engine == m_engineProc[WHITE])
		engineSide = WHITE;
	else if (engine == m_engineProc[BLACK])
		engineSide = BLACK;
	else
		return; // Unknown sender

	switch (eventInfo->type)
	{
	//case EngineEvent::Type::UciOk:
	//	emit engineInitSignal(engine->getEngineInfo());
	//	if (engine->getLaunchType() == UCIEngine::LaunchType::Play)
	//		emit loadEngineOptions(engine);
	//	break;
	case EngineEvent::Type::NewGameStarted:
		m_engGameStarted[engineSide] = true;
		if (m_gameType == GameType::PlayerVsEngine)
			startGame(); // Sender could be only one engine, so start immediately
		else if (m_gameType == GameType::EngineVsEngine)
		{ // Check that both engines finished ucinewgame before starting the game
			if (m_engGameStarted[WHITE] && m_engGameStarted[BLACK])
				startGame();
		}
		break;
	case EngineEvent::Type::BestMove:
		if (engineSide != currentTurn())
			return;
		if (!doMove(eventInfo->bestMove))
			return;
		break;
	case EngineEvent::Type::Info:
		emit searchInfoSignal(engineSide, eventInfo->infoDetails);
		break;
	case EngineEvent::Type::Error:
		emit engineErrorSignal(engineSide, QString::fromStdString(eventInfo->errorText));
		break;
	}
}