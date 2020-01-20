#pragma once

#include <QtCore>
#include <unordered_set>
#include "Engine/ucioption.h"
#include "Engine/engine.h"
#include "Clock.h"

using EngineOptions = std::unordered_map<std::string, UciOption>;
using EngineOptionValues = std::unordered_map<std::string, UciOption::ValueType>;

UciOption uciOptionFromJSON(QJsonObject obj);
QJsonObject uciOptionToJSON(const UciOption& opt);

struct EngineInfo
{
	static EngineInfo fromJSON(QJsonObject obj);
	QJsonObject toJSON(void) const;

	bool operator==(const EngineInfo& rhs) const = default;

	QString name;
	QString uciname;
	QString path;
	QString author;
	EngineOptions options;
};

struct SearchInfoDetails
{
	enum class ScoreType {
		Cp, Mate
	};

	int depth;
	int score;
	int nps;
	ScoreType scoreType;
	BlendXChess::Move move;
	BlendXChess::Bound scoreBound;
	std::string moveStr;
};

struct EngineEvent
{
	enum struct Type {
		None, Error, UciOk, NewGameStarted, BestMove, Info
	};

	class UCIEngine* sender;
	Type type;
	SearchInfoDetails infoDetails;
	std::string bestMove; // UCI string
	std::string ponderMove; // UCI string
	std::string errorText;
};

class UCIEngine : public QObject
{
	Q_OBJECT
public:
	using ReadyOkCallback = std::function<void(void)>;

	enum class State {
		NotSet, WaitingUciOk, SettingOptions, WaitingReadyOk, Ready, Searching
	};
	enum class LaunchType {
		NotSet, Play, Info
	};

	UCIEngine(void);
	UCIEngine(QString path, LaunchType type);
	~UCIEngine(void);

	inline const EngineInfo& getEngineInfo(void) const noexcept;
	inline State getState(void) const noexcept;
	inline std::string getName(void) const noexcept;
	inline std::string getAuthor(void) const noexcept;
	inline const EngineOptions& getOptions(void) const noexcept;
	inline LaunchType getLaunchType(void) const noexcept;

	void initialize();

	void close(void);
	void reset(QString path, LaunchType type);
	void setOptionFromString(const std::string& name, const std::string& value);
	void setOption(const std::string& name, const UciOption::ValueType& value);
	void sendPosition(const std::string& positionFEN);
	void sendNewGame(void);
	/*void sendIsReady(void);*/
	void sendGo(int depth = 10);
	void sendStop(void);
	void doWhenReady(ReadyOkCallback rok_cb);
signals:
	// Signal for notifying about engine events, which is
	// triggered after appropriate input from the process
	void engineSignal(const EngineEvent* eventInfo);
private:
	// Reading various event info from stream
	void readBestmove(std::istream& iss);
	void readInfo(std::istream& iss);
	// Called when setting options to notify the engine process
	void writeSetOption(const std::string& name, const std::string& value);
	UciOption& getOption(const std::string& name);
	// Called when there's new input from the engine process
	void sProcessInput(void);
	// Called when process sends some info into 
	void sProcessError(void);
	// Data
	ReadyOkCallback m_readyOkCallback; // One-time callback for readyOk
	EngineInfo m_info; // As given by current process
	State m_state;
	LaunchType m_launchType;
	EngineEvent m_eventInfo; // Pointer to this will be sent to callback after filling needed info in sProcessInput
	QProcess m_process;
};

inline const EngineInfo& UCIEngine::getEngineInfo(void) const noexcept
{
	return m_info;
}

inline UCIEngine::State UCIEngine::getState(void) const noexcept
{
	return m_state;
}

inline std::string UCIEngine::getName(void) const noexcept
{
	return m_info.name.toStdString();
}

inline std::string UCIEngine::getAuthor(void) const noexcept
{
	return m_info.author.toStdString();
}

inline const EngineOptions& UCIEngine::getOptions(void) const noexcept
{
	return m_info.options;
}

inline UCIEngine::LaunchType UCIEngine::getLaunchType(void) const noexcept
{
	return m_launchType;
}
