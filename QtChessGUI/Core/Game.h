#pragma once

#include "Core/UCIEngine.h"

class Game : public QObject
{
	Q_OBJECT

public:
	enum class GameType {
		None, PlayerVsPlayer, PlayerVsEngine, EngineVsEngine
	};

	Game(QObject* parent = nullptr);
	~Game(void);

	inline bool isUserTurn() const noexcept;
	inline GameType getGameType() const noexcept;
	inline BlendXChess::Side getUserSide() const noexcept;
	inline const BlendXChess::Game& getGame() const noexcept;

	// Close current game
	void closeGame();
	// Start game of various types
	void startPVP();
	void startWithEngine(BlendXChess::Side userSide, UCIEngine* engine);
	void startEngineVsEngine(UCIEngine* whiteEngine, UCIEngine* blackEngine);
	// Undo/redo
	void undo();
	void redo();
	// Send 'position' and 'go' commands to appropriate engine if it is set up
	void goEngine(BlendXChess::Side side);
	// Perform a move given in UCI format
	bool doMove(const std::string& move);
	// Load game from given stream
	void loadPGN(std::istream& inGame);
signals:
	void gameFinishedSignal();
	void positionChangedSignal();
	void engineInitSignal(const EngineInfo& engineInfo);
	void engineErrorSignal(BlendXChess::Side side, QString errorText);
	void searchInfoSignal(BlendXChess::Side side, const SearchInfoDetails& info);
	void loadEngineOptions();
protected slots:
	// This class' engine event callback (other may be registered as well) 
	void engineEventCallback(const EngineEvent* eventInfo);
protected:
	// Close engine
	void closeEngine(BlendXChess::Side side);
	// Start game when all necessary conditions (eg, engines are set up) are met
	void startGame();

	std::chrono::milliseconds m_clock[2]; // Remaining clock time for both players
	GameType m_gameType; // Type of current game
	BlendXChess::Game m_game; // Game object
	BlendXChess::Side m_userSide; // Side of user (if game type is PlayerVsEngine)
	UCIEngine* m_engineProc[BlendXChess::COLOR_CNT]; // Engines for sides
};

inline bool Game::isUserTurn() const noexcept
{
	return m_gameType == GameType::PlayerVsPlayer
		|| m_game.getPosition().getTurn() == m_userSide;
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