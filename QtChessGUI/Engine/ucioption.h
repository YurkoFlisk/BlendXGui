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

	UciOption();
	UciOption(std::istream& istr);
	~UciOption();

	bool operator==(const UciOption&) const = default;

	// Getters
	inline std::string getName() const noexcept;
	inline Type getType() const noexcept;
	inline std::string getTypeStr() const noexcept;
	inline int getMin() const noexcept;
	inline int getMax() const noexcept;
	inline ValueType getDefault() const;
	inline bool getDefaultBool() const;
	inline int getDefaultInt() const;
	// Converts to string representation if needed
	inline std::string getDefaultString() const; 
	inline ComboList getComboVars() const noexcept;

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

inline UciOption::Type UciOption::getType() const noexcept
{
	return type;
}

inline std::string UciOption::getTypeStr() const noexcept
{
	auto it = std::find_if(typeByStr.begin(), typeByStr.end(), [this](const auto& t) {
		return t.second == type;
	});
	return it == typeByStr.end() ? "" : it->first;
}

inline int UciOption::getMin() const noexcept
{
	return min;
}

inline int UciOption::getMax() const noexcept
{
	return max;
}

inline UciOption::ValueType UciOption::getDefault() const
{
	return defaultValue;
}

inline bool UciOption::getDefaultBool() const
{
	return std::get<bool>(defaultValue);
}

inline int UciOption::getDefaultInt() const
{
	return std::get<int>(defaultValue);
}

inline std::string UciOption::getDefaultString() const
{
	return valueToString(defaultValue, type);
}

inline UciOption::ComboList UciOption::getComboVars() const noexcept
{
	return comboVars;
}