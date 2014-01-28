#pragma once
#include "common.hpp"

namespace Platform
{

void init();
void deinit();
void run();

void panic(const char* fmt, ...);

string readFile(const string& path);

}

namespace Log
{

void debug(const char* fmt, ...);
void info(const char* fmt, ...);
void warn(const char* fmt, ...);
void error(const char* fmt, ...);

}
