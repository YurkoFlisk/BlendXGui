#include "UCIEngine.h"
#include "misc.h"
#include <sstream>

namespace BXC = BlendXChess;

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
		else
			uciOptionDef += " " + key.toStdString() + " " + (obj[key].isDouble() ?
				std::to_string(obj[key].toInt()) : obj[key].toVariant().toString().toStdString());
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
	engineInfo.uciname = obj["uciname"].toString();
	engineInfo.path = obj["path"].toString();
	engineInfo.author = obj["author"].toString();

	if (engineInfo.name.isEmpty())
		throw std::runtime_error(QObject::tr(
			"Invalid or missing engine name").toStdString());
	if (engineInfo.uciname.isEmpty())
		throw std::runtime_error(QObject::tr(
			"Invalid or missing UCI engine name").toStdString());
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
	obj["uciname"] = uciname;
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
	: m_state(State::NotSet), m_launchType(LaunchType::NotSet)
{}

UCIEngine::UCIEngine(QString path, LaunchType launchType)
	: UCIEngine()
{
	reset(path, launchType);
}

UCIEngine::~UCIEngine(void)
{
	close();
}

void UCIEngine::initialize()
{
	m_process.write("uci\n");
	while (true)
	{
		if (!m_process.canReadLine() && !m_process.waitForReadyRead(10000))
			break; // maybe throw?
		std::string line = m_process.readLine().toStdString(), cmd, token;
		std::istringstream iss(line);
		iss >> cmd;
		if (cmd == "uciok")
			break;
		else if (cmd == "id")
		{
			iss >> token;
			if (token == "name")
			{
				std::getline(iss, token);
				m_info.uciname = QString::fromStdString(misc::trim(token));
			}
			else if (token == "author")
			{
				std::getline(iss, token);
				m_info.author = QString::fromStdString(misc::trim(token));
			}
		}
		else if (cmd == "option")
		{ // should be m_state == State::WaitingUciOk
			UciOption option(iss);
			if (!m_info.options.insert({ option.getName(), option }).second)
				; // Duplicate option name
		}
		else
			;
	}
	connect(&m_process, &QProcess::readyReadStandardOutput,
		this, &UCIEngine::sProcessInput);
	connect(&m_process, &QProcess::readyReadStandardError,
		this, &UCIEngine::sProcessError);
}

void UCIEngine::close(void)
{
	if (m_process.state() != QProcess::ProcessState::NotRunning)
	{
		m_process.write("stop\n"); // In case search is in process
		m_process.write("quit\n");
		m_process.closeWriteChannel();
		// Have to disconnect so, in case of a reset, connections
		// will not be active until initialize is called
		disconnect(&m_process, &QProcess::readyReadStandardOutput,
			this, &UCIEngine::sProcessInput);
		disconnect(&m_process, &QProcess::readyReadStandardError,
			this, &UCIEngine::sProcessError);
		if (!m_process.waitForFinished(2000))
		{
			m_process.kill();
			throw std::runtime_error(tr(
				"Could not finish previous engine process, so killed it").toStdString());
		}
		/*m_state = State::NotSet;*/
		m_launchType = LaunchType::NotSet;
		m_info = EngineInfo();
	}
}

void UCIEngine::reset(QString path, LaunchType launchType)
{
	close();
	m_process.start(path, QStringList());
	if (!m_process.waitForStarted(5000))
		throw std::runtime_error(tr(
			"Engine process could not have been started").toStdString());
	
	m_process.write("uci\n");
	m_eventInfo.sender = this;
	/*m_state = State::WaitingUciOk;*/
	m_launchType = launchType;
}

void UCIEngine::setOptionFromString(const std::string& name, const std::string& value)
{
	/*if (m_state != State::SettingOptions)
		throw std::runtime_error(tr(
			"We should set options only in SettingOptions state").toStdString());*/
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
	doWhenReady([this] {
		m_eventInfo.type = EngineEvent::Type::NewGameStarted;
		emit engineSignal(&m_eventInfo);
	});
}

//void UCIEngine::sendIsReady(void)
//{
//	if (m_state != State::SettingOptions)
//		return;
//	m_process.write("isready\n");
//	m_state = State::WaitingReadyOk;
//}

//void UCIEngine::go(Clock& clock, int depth)
//{
//	std::string line("go depth " + std::to_string(depth) + "\n");
//	m_process.write(line.data()); // TEMPORARILY
//	m_process.waitForBytesWritten(10000);
//
//	clock.toggle();
//	while (true)
//	{
//		while (!clock.timeout() && !m_process.canReadLine()) // Maybe redundant, but safe
//			m_process.waitForReadyRead(clock.remainingTime().count());
//		if (clock.timeout())
//			break;
//
//		std::string line = m_process.readLine().toStdString(), cmd;
//		std::istringstream iss(line);
//		iss >> cmd;
//		if (cmd == "bestmove")
//		{
//			readBestmove(iss);
//			break;
//		}
//		else if (cmd == "info")
//		{
//			readInfo(iss);
//			getline(iss, m_eventInfo.errorText); // TEMPORARY
//			m_eventInfo.errorText = misc::trim(m_eventInfo.errorText);
//			emit engineSignal(&m_eventInfo);
//		}
//	}
//}

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

void UCIEngine::doWhenReady(ReadyOkCallback rok_cb)
{
	m_process.write("isready\n");
	m_readyOkCallback = rok_cb;
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
	std::string line;
	getline(iss, line);
	std::istringstream infoss(line);
	auto& info = m_eventInfo.infoDetails;
	std::string infoType, scoreType;
	bool innerInfoType = false;
	while (innerInfoType || infoss >> infoType)
	{
		innerInfoType = false;
		if (infoType == "depth")
			infoss >> info.depth;
		else if (infoType == "nps")
			infoss >> info.nps;
		else if (infoType == "move" || infoType == "pv") // TEMPORARY
			infoss >> info.moveStr;
		else if (infoType == "score")
		{
			info.scoreBound = BXC::BOUND_EXACT;
			while (infoss >> scoreType)
			{
				if (scoreType == "lowerbound")
					info.scoreBound = BXC::BOUND_LOWER;
				else if (scoreType == "upperbound")
					info.scoreBound = BXC::BOUND_UPPER;
				else if (scoreType == "cp") [[likely]]
				{
					infoss >> info.score;
					info.scoreType = SearchInfoDetails::ScoreType::Cp;
				}
				else if (scoreType == "mate")
				{
					infoss >> info.score;
					info.scoreType == SearchInfoDetails::ScoreType::Mate;
				}
				else
					break;
			}
			// Because the infoType was read in the inner loop,
			// we must now avoid reading it in outer loop condition
			infoType = scoreType;
			innerInfoType = true;
		}
		else
			throw std::runtime_error(tr("Invalid search info specification:"
				"unknown token %1").arg(QString::fromStdString(infoType)).toStdString());
	}
}

void UCIEngine::writeSetOption(const std::string& name, const std::string& value)
{
	std::string line("setoption name " + name + " value " + value + "\n");
	m_process.write(line.data());
}

UciOption& UCIEngine::getOption(const std::string& name)
{
	auto it = m_info.options.find(name);
	if (it == m_info.options.end())
		throw std::runtime_error(tr(
			"Could not find the option with specified name").toStdString());
	return it->second;
}

void UCIEngine::sProcessInput(void)
{
	while (m_process.canReadLine())
	{
		std::string line = m_process.readLine().toStdString(), cmd;
		std::istringstream iss(line);
		iss >> cmd;
		m_eventInfo.errorText = "";
		//if (cmd == "uciok")
		//{
		//	if (m_state != State::WaitingUciOk)
		//		continue;
		//	m_eventInfo.type = EngineEvent::Type::UciOk;
		//	emit engineSignal(&m_eventInfo);
		//	if (m_launchType == LaunchType::Play)
		//		m_state = State::SettingOptions;
		//	else if (m_launchType == LaunchType::Info)
		//		close();
		//}
		//else if (cmd == "id")
		//{ // should be m_state == State::WaitingUciOk
		//	iss >> token;
		//	if (token == "name")
		//	{
		//		std::getline(iss, token);
		//		m_info.uciname = QString::fromStdString(token);
		//	}
		//	else if (token == "author")
		//	{
		//		std::getline(iss, token);
		//		m_info.author = QString::fromStdString(token);
		//	}
		//}
		//else if (cmd == "option")
		//{ // should be m_state == State::WaitingUciOk
		//	UciOption option(iss);
		//	if (!m_info.options.insert({ option.getName(), option }).second)
		//		; // Duplicate option name
		//}
		if (cmd == "readyok")
		{	
			if (m_readyOkCallback)
			{
				auto rok_cb = std::move(m_readyOkCallback);
				m_readyOkCallback = nullptr; // Action is one-time
				rok_cb();
			}
			/*m_state = State::Ready;
			m_eventInfo.type = EngineEvent::Type::ReadyOk;
			emit engineSignal(&m_eventInfo);*/
		}
		else if (cmd == "bestmove")
		{
			readBestmove(iss);
			/*m_state = State::Ready;*/
			m_eventInfo.type = EngineEvent::Type::BestMove;
			emit engineSignal(&m_eventInfo);
		}
		else if (cmd == "info")
		{
			try
			{
				readInfo(iss);
			}
			catch (const std::runtime_error& err)
			{
				m_eventInfo.errorText = err.what();
			}
			m_eventInfo.type = EngineEvent::Type::Info;
			emit engineSignal(&m_eventInfo);
		}
	}
}

void UCIEngine::sProcessError(void)
{
	m_eventInfo.type = EngineEvent::Type::Error;
	m_eventInfo.errorText = m_process.readAllStandardError().toStdString();
	emit engineSignal(&m_eventInfo);
}