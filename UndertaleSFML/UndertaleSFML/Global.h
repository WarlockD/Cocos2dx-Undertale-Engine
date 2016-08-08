#pragma once
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <unordered_map>
#include <SFML\Graphics.hpp>


namespace console {

};

namespace logging {
	void init_cerr();
	void init_cout();
	bool init();
	void error(const std::string& message);
};