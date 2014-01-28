#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cstdarg>
#include <json11/json11.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

using std::string;
using json11::Json;
using namespace glm;
typedef unsigned uint;

string vlformat(const char* format, va_list vl);

void logDebug(const char* format, ...);
void logInfo(const char* format, ...);
void logWarning(const char* format, ...);
void logError(const char* format, ...);
void panic(const char* format, ...);

string readFile(const string& path);
