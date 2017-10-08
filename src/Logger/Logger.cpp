#include "Logger.hpp"

/// Set the default log level and output width
LogLevel minimumLogLevel = LogLevel::Warn;
int outputWidth = 100;
shared_ptr<RelativeTimeProvider> timeProvider = nullptr;
long startTime = 0;

/// Set some constants
const string spacer = "  ";
const uint8_t prefixSize = 14;

string Logger::getLevelPrefix(LogLevel level, string customPrefix) {
    string prefixColor;
    string prefix;
    string textColor;
    switch (level) {
        case Error:
            prefixColor = Red;
            prefix = "       Error";
            break;
        case Warn:
            prefixColor = Orange;
            prefix = "     Warning";
            break;
        case Info:
            prefixColor = Green;
            prefix = "        Info";
            break;
        case Debug:
            prefixColor = Grey;
            prefix = "       Debug";
            textColor = Grey;
            break;
        case Trace:
            prefixColor = DarkGrey;
            prefix = "       Trace";
            textColor = DarkGrey;
            break;
        case Custom:
            prefixColor = Blue;
            prefix = std::move(customPrefix);
            break;
    }
    stringstream tmp;
    tmp << setw(static_cast<int>(prefixSize - spacer.size())) << prefix;
    return prefixColor + Bold + tmp.str() + Color_Reset + textColor + spacer;
}

void Logger::log(LogLevel level, string msg, string file, int line, string func, string customPrefix) {
    if (level < minimumLogLevel) return;
    Logger(level) << msg;
}

void Logger::setOutputStream(ostream *outputStream) { logOutputStream = outputStream; }

int Logger::LogBuf::sync() {
    if (level < minimumLogLevel || str().empty()) return -1;

    /// Print the log level prefix
    string prefix = Logger::getLevelPrefix(level, this->customLevelName);
    *logOutputStream << prefix;

    /// Remove newlines from the message (since they are replaced with the postfix)
    string msg(str());
    msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());

    /// Print the message and keep record of its size
    this->messageLength = msg.size();
    *logOutputStream << msg << Color_Reset;

    /// Clear the message
    str("");

    /// Print the postfix
    this->printPostfix();

    return logOutputStream ? 0 : -1;
}

Logger::LogBuf::~LogBuf() {
    pubsync();
}

void Logger::LogBuf::printPostfix() {

    stringstream postfix;
    if (timeProvider != nullptr) {
        float time = timeProvider->millis() - startTime;
        postfix << spacer << "[" << setw(static_cast<int>(prefixSize - spacer.size()))
                << std::fixed << std::setprecision(5) << time / 1000.0f;
        postfix << "]";
    }
    // TODO Add context block

    auto offset = static_cast<int>(outputWidth - this->messageLength - prefixSize);

    *logOutputStream << DarkGrey << setw(offset) << setfill(' ') << postfix.str();
    *logOutputStream << Color_Reset << endl;
    messageLength = 0;
}
