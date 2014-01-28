#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>
using std::string;

#include <json11/json11.hpp>
using json11::Json;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

typedef unsigned uint;

void logDebug(const char* format, ...);
void logInfo(const char* format, ...);
void logWarning(const char* format, ...);
void logError(const char* format, ...);
void panic(const char* format, ...);

string readFile(const string& path);
