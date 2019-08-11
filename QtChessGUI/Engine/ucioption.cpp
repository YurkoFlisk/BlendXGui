#include "ucioption.h"
#include <istream>

UciOption::UciOption(void)
	: type(Type::None), min(0), max(0)
{}

UciOption::UciOption(std::istream& istr)
	: UciOption()
{
	readDefinition(istr);
}

UciOption::~UciOption(void)
{}

UciOption::ValueType UciOption::parseValueUnchecked(const std::string& str, Type type)
{
	ValueType ret;
	switch (type)
	{
	case Type::Check:
		if (str == "true")
			ret = true;
		else if (str == "false")
			ret = false;
		else
			throw std::invalid_argument("Check value has invalid string representation");
		break;
	case Type::Spin:
		ret = std::stoi(str);
		break;
	case Type::Combo:
	case Type::String:
		ret = str;
		break;
	default: return false;
	}
	return ret;
}

std::string UciOption::valueToString(const ValueType& val, Type type)
{
	switch (type)
	{
	case Type::Check:
		return std::get<bool>(val) ? "true" : "false";
	case Type::Spin:
		return std::to_string(std::get<int>(val));
	case Type::Combo:
	case Type::String:
		return std::get<std::string>(val);
	default:
		throw std::invalid_argument(
			"Invalid value type (note that 'button' would be invalid here)");
	}
}

void UciOption::clear(void)
{
	type = Type::None;
	comboVars.clear();
	// Wrong values for min and max, so we can later check in
	// unified way whether they were (if needed) properly filled
	min = 0;
	max = -1;
}

void UciOption::readDefinition(std::istream& istr)
{
	clear();
	std::string optionInfo, token, defaultStr, var;
	while (istr >> optionInfo)
	{
		if (optionInfo == "type")
		{
			istr >> token;
			if (auto it = typeByStr.find(token); it != typeByStr.end())
				type = it->second;
		}
		else if (optionInfo == "name")
			readStr(istr, name);
		else if (optionInfo == "default")
			readStr(istr, defaultStr);
		else if (optionInfo == "min")
			istr >> min;
		else if (optionInfo == "max")
			istr >> max;
		else if (optionInfo == "var")
		{
			readStr(istr, var);
			comboVars.push_back(var);
		}
	}
	if (type == Type::None)
		throw std::runtime_error("Option must have a type");
	if (type == Type::Spin && min > max)
		throw std::runtime_error("Option of type 'spin' must have valid min and max values");
	if (type == Type::Combo && comboVars.empty())
		throw std::runtime_error("Option of type 'combo' must have some variants");
	// Only now we are sure about default value type, so we parse it here.
	// Note that UCI specification DOESN'T clearly state the order of
	// option infos (type, default value etc), so here there are no assumptions
	// about it. Otherwise, we could just parse defaultValue right after reading it
	defaultValue = parseValue(defaultStr);
}

UciOption::ValueType UciOption::parseValue(const std::string& str) const
{
	const ValueType val = parseValueUnchecked(str, type);
	checkValue(val);
	return val;
}

std::string UciOption::toString(const ValueType& val) const
{
	checkValue(val);
	return valueToString(val, type);
}

void UciOption::checkValue(const UciOption::ValueType& val) const
{
	if (type == Type::Spin)
	{
		if (!std::holds_alternative<int>(val))
			throw std::runtime_error("Spin option should have integer value");
		const int intVal = std::get<int>(val);
		if (!(min <= intVal && intVal <= max))
			throw std::runtime_error("Could not set value of type 'spin' to out-of-bounds value");
	}
	else if (type == Type::Combo)
	{
		if (!std::holds_alternative<std::string>(val))
			throw std::runtime_error("Spin option should have string value");
		if (std::find(comboVars.begin(), comboVars.end(),
			std::get<std::string>(val)) == comboVars.end())
			throw std::runtime_error("Could not set value of type 'combo' to value not set with 'var'");
	}
	else if (type == Type::String)
	{
		if (!std::holds_alternative<std::string>(val))
			throw std::runtime_error("Combo option should have string value");
	}
	else if (type == Type::Check)
	{
		if (!std::holds_alternative<bool>(val))
			throw std::runtime_error("Check option should have bool value");
	}
	else
		throw std::runtime_error("Can't set value for current option type");
}

void readStr(std::istream& istr, std::string& str)
{
	istr >> str; // TEMPORARILY
}