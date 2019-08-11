#pragma once
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <algorithm>

void readStr(std::istream& istr, std::string& str);

//class UciOptionInfo
//{
//public:
//	using ValueType = std::variant<std::string, int, bool>;
//
//	virtual ValueType parseValue(std::string str) const = 0;
//	virtual std::string toString(ValueType val) const = 0;
//	static UciOptionInfo* readFrom(std::istream& istr);
//private:
//	virtual void readSpecInfoFrom(std::istream& istr) = 0;
//
//	std::string name;
//};

class UciOption
{
public:
	using ValueType = std::variant<std::string, int, bool>;
	using ComboList = std::vector<std::string>;
	enum class Type {
		None, Check, Spin, Combo, Button, String
	};
	static inline const std::unordered_map<std::string, Type> typeByStr = {
		{ "check", Type::Check },
		{ "spin", Type::Spin },
		{ "combo", Type::Combo },
		{ "button", Type::Button },
		{ "string", Type::String }
	};
	static ValueType parseValueUnchecked(const std::string& str, Type type);
	static std::string valueToString(const ValueType& val, Type type);

	UciOption(void);
	UciOption(std::istream& istr);
	~UciOption(void);

	// Getters
	inline std::string getName(void) const noexcept;
	inline Type getType(void) const noexcept;
	inline std::string getTypeStr(void) const noexcept;
	inline int getMin(void) const noexcept;
	inline int getMax(void) const noexcept;
	inline bool getDefaultBool(void) const noexcept;
	inline int getDefaultInt(void) const noexcept;
	// Converts to string representation if needed
	inline std::string getDefaultString(void) const noexcept; 
	inline ComboList getComboVars(void) const noexcept;

	void clear(void);
	void readDefinition(std::istream& istr);
	ValueType parseValue(const std::string& str) const;
	std::string toString(const ValueType& val) const;
private:
	void checkValue(const UciOption::ValueType& val) const;

	std::string name;
	ValueType defaultValue;
	ComboList comboVars;
	int min, max;
	Type type;
};

inline std::string UciOption::getName() const noexcept
{
	return name;
}

inline UciOption::Type UciOption::getType(void) const noexcept
{
	return type;
}

inline std::string UciOption::getTypeStr(void) const noexcept
{
	auto it = std::find_if(typeByStr.begin(), typeByStr.end(), [this](const auto& t) {
		return t.second == type;
	});
	return it == typeByStr.end() ? "" : it->first;
}

inline int UciOption::getMin(void) const noexcept
{
	return min;
}

inline int UciOption::getMax(void) const noexcept
{
	return max;
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
	return valueToString(defaultValue, type);
}

inline UciOption::ComboList UciOption::getComboVars(void) const noexcept
{
	return comboVars;
}