#ifndef LUMOS_LOGGER_HPP
#define LUMOS_LOGGER_HPP

#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <api/time.hpp>

#define Color_Reset "\033[0m"
#define Bold         "\033[1m"

#define Blue        "\033[38;5;14m"
#define Red         "\033[38;5;160m"
#define Orange      "\033[38;5;214m"
#define Green       "\033[38;5;10m"
#define Grey        "\033[38;5;244m"
#define DarkGrey    "\033[38;5;239m"

using namespace std;

enum LogLevel {
    Error = 3,
    Warn = 2,
    Custom = 1,
    Info = 0,
    Debug = -1,
    Trace = -2,
};

class global_ostream : public std::ostream {
public:
    global_ostream() {};
};

static global_ostream logOutputStream;
extern LogLevel minimumLogLevel;
extern int outputWidth;
extern shared_ptr<RelativeTimeProvider> timeProvider;
extern long startTime;

class Logger : public std::ostream {
    class LogBuf : public stringbuf {
        LogLevel level;
        string customLevelName;
        unsigned long messageLength;
    public:
        LogBuf(const LogLevel& level, string customLevel) : level(level), customLevelName(customLevel) {}
        ~LogBuf();
        int sync();
        void printPostfix();
    };

    static string getLevelPrefix(LogLevel level, string customPrefix);
public:
    Logger(const LogLevel& level, string customLevel = "") : std::ostream(new LogBuf(level, customLevel)) {}
    ~Logger() { delete rdbuf(); }

    // TODO Add Logger::showModules(bool)
    // TODO Add Logger::showSourcePaths(bool)
    static void setLogLevel(const LogLevel& level) { minimumLogLevel = level; }
    static void setOutputStream(ostream* outputStream);
    static void setOutputWidth(int width) { outputWidth = width; };
    static void setTimeProvider(shared_ptr<RelativeTimeProvider> provider) {
        timeProvider = provider;
        startTime = provider->millis();
    };

    static void log(LogLevel level, string msg, string file, int line, string func, string customPrefix = "");
};

#define __CONTEXT__ __FILE__, __LINE__, __func__
#define err(msg) { Logger::log(LogLevel::Error, msg, __CONTEXT__); }
#define warn(msg) { Logger::log(LogLevel::Warn, msg, __CONTEXT__); }
#define info(msg) { Logger::log(LogLevel::Info, msg, __CONTEXT__); }
#define debug(msg) { Logger::log(LogLevel::Debug, msg, __CONTEXT__); }
#define trace(msg) { Logger::log(LogLevel::Trace, msg, __CONTEXT__); }
#define custom(prefix, msg) { Logger::log(LogLevel::Custom, msg, __CONTEXT__, prefix); }


#endif //LUMOS_LOGGER_HPP
