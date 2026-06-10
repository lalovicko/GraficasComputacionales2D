#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <map>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <utility>
#include <queue>

#include <SFML/Graphics.hpp>

// MACRO for safe release of resources
#define SAFE_PTR_RELEASE(x) if(x != nullptr) { delete x; x = nullptr; }

#define MESSAGE(classObj, method, state)                      \
{                                                             \
    std::ostringstream os_;                                   \
    os_ << classObj << "::" << method << " : "                \
        << "[CREATION OF RESOURCE" << ": " << state "] \n";\
    std::cerr << os_.str();                                   \
}

#define ERROR(classObj, method, errorMSG)                         \
{                                                                 \
    std::ostringstream os_;                                       \
    os_ << "ERROR : " << classObj << "::" << method << " : "      \
        << "  Error in data from params [" << errorMSG"] \n"; \
    std::cerr << os_.str();                                       \
    exit(1);                                                      \
}

enum
    ShapeType {
	EMPTY = 0,
	CIRCLE = 1,
	RECTANGLE = 2,
	TRIANGLE = 3,
	POLYGON = 4,
	LINE = 5
};