#include "UCIEngine.h"
#include "misc.h"
#include <sstream>

UciOption uciOptionFromJSON(QJsonObject obj)
{
	// We use UciOption's ability to read UCI-style definition of an option
	std::string uciOptionDef;
	for (const auto& key : obj.keys())
	{
		if (key == "vars")
		{
			const QJsonArray comboVars = obj[key].toArray();
			for (const auto& comboVar : comboVars)
				uciOptionDef += " var " + comboVar.toString().toStdString();
		}
		else // toVariant() THEN toString() for number conversion (if needed)
			uciOptionDef += " " + key.toStdString() + " "
			+ obj[key].toVariant().toString().toStdString();
	}
	std::istringstream iss(uciOptionDef);
	return UciOption(iss);
}

QJsonObject uciOptionToJSON(const UciOption& opt)
{
	QJsonObject obj;
	obj["name"] = QString::fromStdString(opt.getName());
	obj["type"] = QString::fromStdString(opt.getTypeStr());
	if (opt.getType() != UciOption::Type::Button)
		obj["default"] = QString::fromStdString(opt.getDefaultString());
	if (opt.getType() == UciOption::Type::Combo)
	{
		QJsonArray comboVars;
		for (const auto& comboVar : opt.getComboVars())
			comboVars.append(QString::fromStdString(comboVar));
		obj["vars"] = comboVars;
	}
	else if (opt.getType() == UciOption::Type::Spin)
	{
		obj["min"] = opt.getMin();
		obj["max"] = opt.getMax();
	}
	return obj;
}

EngineInfo EngineInfo::fromJSON(QJsonObject obj)
{
	EngineInfo engineInfo;
	engineInfo.name = obj["name"].toString();
	engineInfo.path = obj["path"].toString();
	engineInfo.author = obj["author"].toString();

	if (engineInfo.name.isEmpty())
		throw std::runtime_error(QObject::tr(
			"Invalid or missing engine name").toStdString());
	if (engineInfo.path.isEmpty())
		throw std::runtime_error(QObject::tr(
			"Invalid or missing engine path").toStdString());

	const QJsonArray options = obj["cachedOptions"].toArray();
	for (const auto& optionRef : options)
	{
		const QJsonObject optObj = optionRef.toObject();
		if (optObj.empty())
			continue;
		UciOption option = uciOptionFromJSON(optObj);
		if (!engineInfo.options.emplace(option.getName(), option).second)
			throw std::runtime_error(QObject::tr(
				"Option name appeared twice in cachedOptions").toStdString());
	}

	return engineInfo;
}

QJsonObject EngineInfo::toJSON(void) const
{
	QJsonObject obj;
	obj["name"] = name;
	obj["path"] = path;
	if (!author.isEmpty())
		obj["author"] = author;

	QJsonArray optionArr;
	for (const auto& [_, option] : options)
		optionArr.append(uciOptionToJSON(option));
	obj["cachedOptions"] = optionArr;

	return obj;
}

UCIEngine::UCIEngine(void)
	: m_state(State::NotSet), m_eventCallback(EmptyCallback)
{}

UCIEngine::UCIEngine(QString path, Callback eventCallback)
	: m_state(State::NotSet)
{
	reset(path, eventCallback);
}

UCIEngine::~UCIEngine(void)
{}

void UCIEngine::close(void)
{
	if (m_process.state() != QProcess::ProcessState::NotRunning)
	{
		m_process.write("stop\n"); // In case search is in process
		m_process.closeWriteChannel();
		if (!m_process.waitForFinished(2000))
		{
			m_process.kill();
			throw std::runtime_error("Could not finish previous engine process, so killed it");
		}
		m_state = State::NotSet;
		return;
	}
}

void UCIEngine::reset(QString path, Callback eventCallback)
{
	close();
	m_process.start(path, QStringList());
	if (!m_process.waitForStarted(5000))
		throw std::runtime_error("Engine process could not have been started");
	(void)QObject::connect(&m_process, &QProcess::readyReadStandardOutput,
		[this]() {return sProcessInput(); });
	(void)QObject::connect(&m_process, &QProcess::readyReadStandardError,
		[this]() {return sProcessError(); });
	m_process.write("uci\n");
	m_state = State::WaitingUciOk;
	m_eventCallback = eventCallback;
}

void UCIEngine::setOptionFromString(const std::string& name, const std::string& value)
{
	if (m_state != State::SettingOptions)
		throw std::runtime_error("We should set options only in SettingOptions state");
	writeSetOption(name, value);
}

void UCIEngine::setOption(const std::string& name, const UciOption::ValueType& value)
{
	setOptionFromString(name, getOption(name).toString(value));
}

void UCIEngine::sendPosition(const std::string& positionFEN)
{
	if (positionFEN == "startpos")
		m_process.write("startpos\n");
	else
	{
		std::string line("position fen " + positionFEN + "\n");
		m_process.write(line.data());
	}
}

void UCIEngine::sendNewGame(void)
{
	m_process.write("ucinewgame\n");
}

void UCIEngine::sendIsReady(void)
{
	if (m_state != State::SettingOptions)
		return;
	m_process.write("isready\n");
	m_state = State::WaitingReadyOk;
}

void UCIEngine::sendGo(int depth)
{
	std::string line("go depth " + std::to_string(depth) + "\n");
	m_process.write(line.data()); // TEMPORARILY
	m_process.waitForBytesWritten(10000);
}

void UCIEngine::sendStop(void)
{
	m_process.write("stop\n");
}

void UCIEngine::readBestmove(std::istream& iss)
{
	std::string token;
	iss >> m_eventInfo.bestMove;
	if ((iss >> token) && token == "ponder")
		iss >> m_eventInfo.ponderMove;
}

void UCIEngine::readInfo(std::istream& iss)
{
	// TEMPORARY EMPTY
}

void UCIEngine::writeSetOption(const std::string& name, const std::string& value)
{
	std::string line("setoption name " + name + " value " + value + "\n");
	m_process.write(line.data());
}

UciOption& UCIEngine::getOption(const std::string& name)
{
	auto it = m_options.find(name);
	if (it == m_options.end())
		throw std::runtime_error("Could not find the option with specified name");
	return it->second;
}

void UCIEngine::sProcessInput(void)
{
	while (m_process.canReadLine())
	{
		std::string line = m_process.readLine().toStdString(), cmd, token;
		std::istringstream iss(line);
		iss >> cmd;
		if (cmd == "uciok")
		{
			if (m_state != State::WaitingUciOk)
				continue;
			m_state = State::SettingOptions;
			m_eventInfo.type = UCIEventInfo::Type::UciOk;
			m_eventCallback(this, &m_eventInfo);
		}
		else if (cmd == "id")
		{ // should be m_state == State::WaitingUciOk
			iss >> token;
			if (token == "name")
				std::getline(iss, m_name);
			else if (token == "author")
				std::getline(iss, m_author);
		}
		else if (cmd == "option")
		{ // should be m_state == State::WaitingUciOk
			UciOption option(iss);
			if (!m_options.insert({ option.getName(), option }).second)
				; // Duplicate option name
		}
		else if (cmd == "readyok")
		{
			m_state = State::Ready;
			m_eventInfo.type = UCIEventInfo::Type::ReadyOk;
			m_eventCallback(this, &m_eventInfo);
		}
		else if (cmd == "bestmove")
		{
			readBestmove(iss);
			m_state = State::Ready;
			m_eventInfo.type = UCIEventInfo::Type::BestMove;
			m_eventCallback(this, &m_eventInfo);
		}
		else if (cmd == "info")
		{
			readInfo(iss);
			getline(iss, m_eventInfo.errorText); // TEMPORARY
			m_eventInfo.errorText = misc::trim(m_eventInfo.errorText);
			m_eventInfo.type = UCIEventInfo::Type::Info;
			m_eventCallback(this, &m_eventInfo);
		}
	}
}

void UCIEngine::sProcessError(void)
{
	m_eventInfo.type = UCIEventInfo::Type::Error;
	m_eventInfo.errorText = m_process.readAllStandardError().toStdString();
	m_eventCallback(this, &m_eventInfo);
}