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
	void startWithEngine(BlendXChess::Side userSide, QString enginePath);
	void startEngineVsEngine(QString whiteEnginePath, QString blackEnginePath);
	// Undo/redo
	void undo();
	void redo();
	// Send 'position' and 'go' commands to appropriate engine if it is set up
	void goEngine(BlendXChess::Side side);
	// Perform a move given in UCI format
	bool doMove(const std::string& move);
	// Load game from given stream
	bool loadPGN(std::istream& inGame);
protected slots:
	// This class' engine event callback (other may be registered as well) 
	void engineEventCallback(const UCIEventInfo* eventInfo);
protected:
	// Start game when all necessary conditions (eg, engines are set up) are met
	void startGame();
	// Launch given engine as given side player
	void launchEngine(BlendXChess::Side side, QString path);

	std::chrono::milliseconds m_clock[2]; // Remaining clock time for both players
	GameType m_gameType; // Type of current game
	BlendXChess::Game m_game; // Game object
	BlendXChess::Side m_userSide; // Side of user (if game type is PlayerVsEngine)
	UCIEngine m_engineProc[BlendXChess::COLOR_CNT]; // Engines for sides
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