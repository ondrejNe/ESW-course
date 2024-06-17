
#ifndef LOGGER_H
#define LOGGER_H

// Begin suppression for GCC and Clang
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wall"
    #pragma GCC diagnostic ignored "-Wextra"
    #pragma GCC diagnostic ignored "-Wpedantic"
    #pragma GCC diagnostic ignored "-Wformat-security"
#endif

// Begin suppression for MSVC
#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#include <functional>
#include <unordered_map>

using namespace std;

// Log level definitions
enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

// Global logger settings
static unordered_map<thread::id, size_t> threadIDMap;
static size_t nextThreadID = 1;
static mutex mapMutex;
static mutex outputMutex;

class PrefixedLogger {
private:
    string          prefix;
    bool            active;
    ostream&        output;
    vector<string>  additionalPrefixes;
public:
    // Constructor with prefix, output stream, and minimum log level
    explicit PrefixedLogger(string prefix, bool active = true, ostream& output = cout)
            : prefix(prefix), active(active), output(output) {}

    // Method to add additional prefixes
    void addPrefix(string newPrefix) {
        additionalPrefixes.push_back(newPrefix);
    }

    void removePrefix(string prefix) {
        additionalPrefixes.erase(remove(additionalPrefixes.begin(), additionalPrefixes.end(), prefix), additionalPrefixes.end());
    }

    // Log a formatted message
    template<typename... Args>
    void log(LogLevel level, string formatString, Args... args) {
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
        ostringstream stream;
        stream << getCurrentTimestamp() << " " << getFullPrefix(level) << ": " << formatMessage(formatString, args...);
        {
            lock_guard<mutex> lock(outputMutex);
            output << "test" << endl;
            output << stream.str() << endl;
            output << "endtest" << endl;
        }
    }

    // Convenience methods for each log level
    template<typename... Args>
    void debug(string formatString, Args... args) { log(DEBUG, formatString, args...); }
    template<typename... Args>
    void info(string formatString, Args... args) { log(INFO, formatString, args...); }
    template<typename... Args>
    void warn(string formatString, Args... args) { log(WARN, formatString, args...); }
    template<typename... Args>
    void error(string formatString, Args... args) { log(ERROR, formatString, args...); }

private:
    // Helper to convert LogLevel to string
    string toString(LogLevel level) {
        switch(level) {
            case DEBUG:  return "\033[36m[DBUG]\033[0m"; // Cyan
            case INFO:   return "\033[32m[INFO]\033[0m"; // Green
            case WARN:   return "\033[33m[WARN]\033[0m"; // Yellow
            case ERROR:  return "\033[31m[ERRR]\033[0m"; // Red
            default:     return "\033[37m[UNKN]\033[0m"; // White
        }
    }

    // Helper to format message according to given arguments using fmt library
    template<typename... Args>
    string formatMessage(string format, Args... args) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), format.c_str(), args...);
        return string(buffer);
    }

    // Helper to get the current timestamp with milliseconds
    string getCurrentTimestamp() {
        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);
        auto milliseconds = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

        ostringstream ss;
        ss << put_time(localtime(&in_time_t), "%Y-%m-%d %X");
        ss << '.' << setw(3) << setfill('0') << milliseconds.count();
        return ss.str();
    }

    // Helper to get the full prefix including additional prefixes
    string getFullPrefix(LogLevel level) {
        ostringstream fullPrefix;
        fullPrefix << prefix;
#ifdef ENABLE_LOGGER_THREAD
        thread::id this_id = this_thread::get_id();
        {
            lock_guard<mutex> lock(mapMutex);
            if (threadIDMap.find(this_id) == threadIDMap.end()) {
                threadIDMap[this_id] = nextThreadID++;
            }
        }
        fullPrefix << "[TID: " << threadIDMap[this_id] << "]";
#endif
        fullPrefix << toString(level);
        for (auto& p : additionalPrefixes) {
            fullPrefix << p;
        }
        return fullPrefix.str();
    }
};

// End suppression for GCC and Clang
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

// End suppression for MSVC
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // LOGGER_H
