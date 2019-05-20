#pragma once
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>

void readStr(std::istream& istr, std::string& str);

class UciOption
{
public:
	using ValueType = std::variant<std::string, int, bool>;
	using ComboList = std::vector<std::string>;
	enum class Type {
		None, Check, Spin, Combo, Button, String
	};
	static inline std::unordered_map<std::string, Type> typeByStr = {
		{ "check", Type::Check },
		{ "spin", Type::Spin },
		{ "combo", Type::Combo },
		{ "button", Type::Button },
		{ "string", Type::String }
	};
	static ValueType parseValue(const std::string& str, Type type);
	static std::string valueToString(const ValueType& val, Type type);

	UciOption(void);
	UciOption(std::istream& istr);
	~UciOption(void);

	inline Type getType(void) const noexcept;
	inline int getMin(void) const noexcept;
	inline int getMax(void) const noexcept;
	inline bool getBool(void) const noexcept;
	inline int getInt(void) const noexcept;
	inline std::string getString(void) const noexcept;
	inline bool getDefaultBool(void) const noexcept;
	inline int getDefaultInt(void) const noexcept;
	inline std::string getDefaultString(void) const noexcept;
	inline ComboList getComboVars(void) const noexcept;
	inline std::string toString(void) const; // Value to string representation

	void clear(void);
	void readDefinition(std::istream& istr);
	void readValue(std::istream& istr);
	void setValue(const ValueType& val);
	void setValueFromString(const std::string& str);
private:
	void setValue(ValueType& valInto, const ValueType& val);
	void setValueFromString(ValueType& valInto, const std::string& str); // for defaultValue also
	ValueType value;
	ValueType defaultValue;
	ComboList comboVars;
	int min, max;
	Type type;
};

inline UciOption::Type UciOption::getType(void) const noexcept
{
	return type;
}

inline int UciOption::getMin(void) const noexcept
{
	return min;
}

inline int UciOption::getMax(void) const noexcept
{
	return max;
}

inline bool UciOption::getBool(void) const noexcept
{
	return std::get<bool>(value);
}

inline int UciOption::getInt(void) const noexcept
{
	return std::get<int>(value);
}

inline std::string UciOption::getString(void) const noexcept
{
	return std::get<std::string>(value);
}

inline bool UciOption::getDefaultBool(void) const noexcept
{
	return std::get<bool>(defaultValue);
}

inline int UciOption::getDefaultInt(void) const noexcept
{
	return std::get<int>(defaultValue);
}

inline std::string UciOption::getDefaultString(void) const noexcept
{
	return std::get<std::string>(defaultValue);
}

inline UciOption::ComboList UciOption::getComboVars(void) const noexcept
{
	return comboVars;
}

std::string UciOption::toString(void) const
{
	return valueToString(value, type);
}
