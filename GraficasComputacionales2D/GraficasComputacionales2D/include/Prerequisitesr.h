#pragma once

// Biblioteca estandar
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// Third party
#include <SFML/Graphics.hpp>

#define SAFE_PTR_RELEASE(x)                 \
    do {                                    \
        if ((x) != nullptr) {               \
            delete (x);                     \
            (x) = nullptr;                  \
        }                                   \
    } while (false)

#define MESSAGE(classObj, method, state)                    \
    do {                                                     \
        std::ostringstream os_;                              \
        os_ << (classObj) << "::" << (method) << " : "      \
            << "[CREATION OF RESOURCE: " << (state) << "]\n"; \
        std::cerr << os_.str();                              \
    } while (false)

#define ERROR(classObj, method, errorMSG)                    \
    do {                                                     \
        std::ostringstream os_;                              \
        os_ << "ERROR : " << (classObj) << "::" << (method) \
            << " : " << (errorMSG) << "\n";                 \
        std::cerr << os_.str();                              \
        std::exit(EXIT_FAILURE);                             \
    } while (false)

enum ShapeType {
    EMPTY = 0,
    CIRCLE = 1,
    RECTANGLE = 2,
    TRIANGLE = 3,
    POLYGON = 4,
    LINE = 5
};
