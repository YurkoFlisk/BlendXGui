#pragma once

#include <condition_variable>
#include "UCIEngine.h"
#include "Clock.h"

class Game : public QObject
{
	Q_OBJECT

public:
	enum class GameType {
		None, PlayerVsPlayer, PlayerVsEngine, EngineVsEngine
	};

	Game(QObject* parent = nullptr);
	~Game(void);

	inline bool isEngineSide(BlendXChess::Side side) const noexcept;
	inline bool isUserSide(BlendXChess::Side side) const noexcept;
	inline bool isEngineTurn() const noexcept; // Now
	inline bool isUserTurn() const noexcept; // Now
	inline BlendXChess::Side currentTurn() const noexcept; // Now
	inline GameType getGameType() const noexcept;
	inline BlendXChess::Side getUserSide() const noexcept;
	inline const BlendXChess::Game& getGame() const noexcept;
	inline const Clock& getClock(BlendXChess::Side side) const noexcept;

	// Game loop
	//void gameLoop();
	// Close current game
	void closeGame();
	// Prepare game of various types
	void preparePVP();
	void prepareWithEngine(BlendXChess::Side userSide, UCIEngine* engine);
	void prepareEngineVsEngine(UCIEngine* whiteEngine, UCIEngine* blackEngine);
	// Start game when all necessary conditions (eg, engines are set up) are met
	void startGame();
	// Undo/redo
	void undo();
	void redo();
	// Perform a move given in UCI format
	bool doMove(const std::string& move);
	// Load game from given stream
	void loadPGN(std::istream& inGame);
signals:
	void gameFinishedSignal();
	void positionChangedSignal();
	void readyToStart();
	void engineInitSignal(const EngineInfo& engineInfo);
	void engineErrorSignal(BlendXChess::Side side, QString errorText);
	void searchInfoSignal(BlendXChess::Side side, const SearchInfoDetails& info);
	void loadEngineOptions();
protected slots:
	// Timeout of player playing on given side
	void playerTimeout(BlendXChess::Side side);
	// This class' engine event callback (other may be registered as well) 
	void engineEventCallback(const EngineEvent* eventInfo);
protected:
	// Send 'position' and 'go' commands to appropriate engine if it is set up
	void goEngine(BlendXChess::Side side);
	// Close engine
	/*void closeEngine(BlendXChess::Side side);*/

	Clock m_clock[BlendXChess::COLOR_CNT]; // Clock info for both players
	bool m_readyForStart; // Whether game's ready for start
	bool m_engGameStarted[BlendXChess::COLOR_CNT]; // Whether an engine finished processing ucinewgame
	//std::condition_variable userMovedCV; // Notified when user has just moved
	//std::mutex moveMutex; // Mutex acquired when (un-/re-)doing moves
	//bool userMoved; // Identifies whether the user has just moved (used together with userMovedCV)
	GameType m_gameType; // Type of current game
	BlendXChess::Game m_game; // Game object
	BlendXChess::Side m_userSide; // Side of user (if game type is PlayerVsEngine)
	UCIEngine* m_engineProc[BlendXChess::COLOR_CNT]; // Engines for sides
};

inline bool Game::isEngineSide(BlendXChess::Side side) const noexcept
{
	return !isUserSide(side);
}

inline bool Game::isUserSide(BlendXChess::Side side) const noexcept
{
	return m_gameType == GameType::PlayerVsPlayer
		|| (m_gameType == GameType::PlayerVsEngine && side == m_userSide);
}

inline bool Game::isEngineTurn() const noexcept
{
	return !isUserTurn();
}

inline bool Game::isUserTurn() const noexcept
{
	return isUserSide(currentTurn());
}

inline BlendXChess::Side Game::currentTurn() const noexcept
{
	return m_game.getPosition().getTurn();
}

inline Game::GameType Game::getGameType() const noexcept
{
	return m_gameType;
}

inline BlendXChess::Side Game::getUserSide() const noexcept
{
	return m_userSide;
}

inline const BlendXChess::Game& Game::getGame() const noexcept
{
	return m_game;
}

inline const Clock& Game::getClock(BlendXChess::Side side) const noexcept
{
	return m_clock[side];
}
