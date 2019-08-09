#pragma once
#include <string>

namespace misc
{
	inline const std::string DEFAULT_TRIMMING_CHARS = " \n\r";

	std::string trim_left(std::string str,
		const std::string& chars = DEFAULT_TRIMMING_CHARS);
	std::string trim_right(std::string str,
		const std::string& chars = DEFAULT_TRIMMING_CHARS);

	inline std::string trim(std::string str,
		const std::string& chars = DEFAULT_TRIMMING_CHARS)
	{
		return trim_left(trim_right(str, chars), chars);
	}
}