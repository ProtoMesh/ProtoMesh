#ifndef HoMesh_LOGGER_HPP
#define HoMesh_LOGGER_HPP

#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <api/time.hpp>
#include <utility>

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

static std::ostream* logOutputStream;
extern LogLevel minimumLogLevel;
extern int outputWidth;
extern shared_ptr<RelativeTimeProvider> timeProvider;
extern long startTime;

class Logger : public std::ostream {
    class LogBuf : public stringbuf {
        LogLevel level;
        string customLevelName;
        unsigned long messageLength = 0;
    public:
        LogBuf(const LogLevel& level, string customLevel) : level(level), customLevelName(std::move(customLevel)) {}
        ~LogBuf() override;
        int sync() override;
        void printPostfix();
    };

    static string getLevelPrefix(LogLevel level, string customPrefix);
public:
    explicit Logger(const LogLevel& level, string customLevel = "") : std::ostream(new LogBuf(level,
                                                                                              std::move(customLevel))) {}
    ~Logger() override { delete rdbuf(); }

    // TODO Add Logger::showModules(bool)
    // TODO Add Logger::showSourcePaths(bool)
    static void setOutputStream(ostream* outputStream);
    static void setLogLevel(const LogLevel& level) { minimumLogLevel = level; }
    static void setOutputWidth(int width) { outputWidth = width; };
    static void setTimeProvider(const shared_ptr<RelativeTimeProvider> &provider) {
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


#endif //HoMesh_LOGGER_HPP
