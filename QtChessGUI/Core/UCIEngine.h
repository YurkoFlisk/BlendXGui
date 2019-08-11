#pragma once

#include <QtCore>
#include <unordered_set>
#include "Engine/ucioption.h"
#include "Engine/engine.h"

using EngineOptions = std::unordered_map<std::string, UciOption>;

UciOption uciOptionFromJSON(QJsonObject obj);
QJsonObject uciOptionToJSON(const UciOption& opt);

struct EngineInfo
{
	static EngineInfo fromJSON(QJsonObject obj);
	QJsonObject toJSON(void) const;

	QString name;
	QString path;
	QString author;
	EngineOptions options;
};

struct InfoDetails
{
	enum class ScoreType {
		Cp, Mate, LowerBound, UpperBound
	};
	int depth;
	int score;
};

struct UCIEventInfo
{
	enum struct Type {
		None, Error, UciOk, ReadyOk, BestMove, Info
	};
	Type type;
	InfoDetails infoDetails;
	std::string bestMove; // UCI string
	std::string ponderMove; // UCI string
	std::string errorText;
};

class UCIEngine
{
public:
	using Callback = std::function<void(UCIEngine*, const UCIEventInfo*)>;
	static inline Callback EmptyCallback = [](UCIEngine*, const UCIEventInfo*) {};
	enum class State {
		NotSet, WaitingUciOk, SettingOptions, WaitingReadyOk, Ready, Searching
	};
	UCIEngine(void);
	UCIEngine(QString path, Callback eventCallback = EmptyCallback);
	~UCIEngine(void);
	inline const EngineInfo& getEngineInfo(void) const noexcept;
	inline State getState(void) const noexcept;
	inline std::string getName(void) const noexcept;
	inline std::string getAuthor(void) const noexcept;
	inline const EngineOptions& getOptions(void) const noexcept;
	void close(void);
	void reset(QString path, Callback eventCallback = EmptyCallback);
	void setOptionFromString(const std::string& name, const std::string& value);
	void setOption(const std::string& name, const UciOption::ValueType& value);
	void sendPosition(const std::string& positionFEN);
	void sendNewGame(void);
	void sendIsReady(void);
	void sendGo(int depth = 10);
	void sendStop(void);
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
	EngineInfo m_info;
	std::string m_name; // from UCI 'id' command
	std::string m_author; // from UCI 'id' command
	State m_state;
	EngineOptions m_options;
	Callback m_eventCallback; // Callback to signalize initial option setting
	UCIEventInfo m_eventInfo; // Pointer to this will be sent to callback after filling needed info in sProcessInput
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
	return m_name;
}

inline std::string UCIEngine::getAuthor(void) const noexcept
{
	return m_author;
}

inline const EngineOptions& UCIEngine::getOptions(void) const noexcept
{
	return m_options;
}