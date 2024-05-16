
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <sstream>

// Log level definitions
enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class PrefixedLogger {
private:
    std::string     prefix;
    std::ostream&   output;
    LogLevel        currentLevel;

public:
    // Constructor with prefix, output stream, and minimum log level
    explicit PrefixedLogger(const std::string& prefix, LogLevel level = INFO, std::ostream& output = std::cout)
            : prefix(prefix), output(output), currentLevel(level) {}

    // Log a formatted message
    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args... args) const {
#ifndef ENABLE_DEBUG
        if (level == DEBUG) return;
#endif
#ifndef ENABLE_INFO
        if (level == INFO) return;
#endif
#ifndef ENABLE_WARN
        if (level == WARN) return;
#endif
#ifndef ENABLE_ERROR
        if (level == ERROR) return;
#endif
        if (level < currentLevel) return;
        std::ostringstream stream;
        stream << prefix << toString(level) << ": " << formatMessage(format, args...);
        output << stream.str() << std::endl;
    }

    // Convenience methods for each log level
    template<typename... Args>
    void debug(const std::string& format, Args... args) const { log(DEBUG, format, args...); }
    template<typename... Args>
    void info(const std::string& format, Args... args) const { log(INFO, format, args...); }
    template<typename... Args>
    void warn(const std::string& format, Args... args) const { log(WARN, format, args...); }
    template<typename... Args>
    void error(const std::string& format, Args... args) const { log(ERROR, format, args...); }

private:
    // Helper to convert LogLevel to string
    std::string toString(LogLevel level) const {
        switch(level) {
            case DEBUG:  return "\033[36m[DEBUG]\033[0m";   // Cyan
            case INFO:   return "\033[32m[INFO ]\033[0m";   // Green
            case WARN:   return "\033[33m[WARN ]\033[0m";   // Yellow
            case ERROR:  return "\033[31m[ERROR]\033[0m";   // Red
            default:     return "\033[37m[UNKNOWN]\033[0m"; // White
        }
    }

    // Helper to format message according to given arguments
    template<typename... Args>
    std::string formatMessage(const std::string& format, Args... args) const {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), format.c_str(), args...);
        return std::string(buffer);
    }
};

#endif //LOGGER_H
