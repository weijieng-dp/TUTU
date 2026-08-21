#pragma once
/*!
@file       Logger.h
@author     Ng Wei Jie (weijie.ng) 100%
@date       03/2/2026
@brief		
            Declares the Logger class used for collecting, formatting,
            and managing runtime log messages for debugging and editor display.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include <string>
#include <vector>
#include <stdarg.h>

class Logger
{
public:


    enum class LogLevel { Info, Warning, Error, Debug };
private:
    struct LogEntry {
        std::string message;
        LogLevel level;
        int count = 1;
    };


public:
    /*!
     * \brief
     *    Adds a new log entry to the logger.
     *    If the message already exists and stacking is enabled,
     *    the existing entry count may be incremented.
     *
     * \param
     *    msg - The log message to be recorded.
     * \param
     *    lvl - The severity level of the log message.
     */
    void AddLog(const std::string msg, LogLevel lvl = LogLevel::Info);

    /*!
    * \brief
    *    Retrieves the internal list of log entries.
    *    This allows external systems to iterate and display logs.
    *
    * \return
    *    [std::vector<LogEntry>&] Reference to the stored log entries.
    */
    std::vector<LogEntry>& GetLogs();

    /*!
    * \brief
    *    Formats a variadic string using printf-style arguments.
    *    This is typically used to construct formatted log messages.
    *
    * \param
    *    msg - The format string.
    *
    * \return
    *    [std::string] The formatted output string.
    */
    std::string FormatString(const char* msg, ...);
    bool autoScroll = true;
    bool Stacked = true;
private:
    std::vector<LogEntry> entries;
};

