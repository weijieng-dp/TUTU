/*!
@file       Logger.cpp
@author     Ng Wei Jie (weijie.ng) 100%
@date       03/2/2026
@brief
            Declares the Logger class used for collecting, formatting,
            and managing runtime log messages for debugging and editor display.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "Logger.h"

void Logger::AddLog(const std::string msg, LogLevel lvl)
{
    if (!entries.empty())
    {
        LogEntry& last = entries.back();

        // same message + same level = collapse
        if (last.message == msg && last.level == lvl)
        {
            last.count++;
            return; // don't add a new entry
        }
    }
    std::cout << msg << std::endl;
    entries.push_back({ msg, lvl, 1 });

}

std::vector<Logger::LogEntry>& Logger::GetLogs()
{
	return entries;
}

std::string Logger::FormatString(const char* msg, ...)
{
    char buffer[2048];
    va_list args;

    va_start(args, msg); // Initialize va_list
    vsnprintf(buffer, sizeof(buffer), msg, args); // Format into buffer
    va_end(args); // Clean up va_list

    return std::string(buffer);
}