#include "misc.h"

std::string misc::trim_left(std::string str, const std::string& chars)
{
	if (const size_t beg = str.find_first_not_of(chars); beg != std::string::npos)
		str.erase(0, beg);
	return str;
}

std::string misc::trim_right(std::string str, const std::string& chars)
{
	if (const size_t last = str.find_last_not_of(chars); last != std::string::npos)
		str.erase(last + 1);
	return str;
}
