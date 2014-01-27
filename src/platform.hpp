#pragma once

namespace Platform
{

void init();
void deinit();
void run();

void panic(const char* fmt, ...);

}

namespace Log
{

void debug(const char* fmt, ...);
void info(const char* fmt, ...);
void warn(const char* fmt, ...);
void error(const char* fmt, ...);

}
