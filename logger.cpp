/**
 * @file
 * @author David Vitez (AKA: Robotic Forest)
 * @copyright All rights reserved © 2020 David Vitez
 */

#include "logger.h"

#include <ctime>
#include <sstream>
#include <string>
#include <chrono>
#include <functional>

namespace DV {

    // ----------------------------------------------------------------------------------------------------
    // Unix/Linux terminal color management routines.
    // ----------------------------------------------------------------------------------------------------

    /**
     * @brief Unix/Linux specific unicode values for printing colored text to a terminal.
     */
    namespace TerminalColor {
        [[maybe_unused]] static const char* const black      = "\u001B[30m";
        [[maybe_unused]] static const char* const red        = "\u001B[31m";
        [[maybe_unused]] static const char* const green      = "\u001B[32m";
        [[maybe_unused]] static const char* const yellow     = "\u001B[33m";
        [[maybe_unused]] static const char* const blue       = "\u001B[34m";
        [[maybe_unused]] static const char* const magenta    = "\u001B[35m";
        [[maybe_unused]] static const char* const cyan       = "\u001B[36m";
        [[maybe_unused]] static const char* const white      = "\u001B[37m";
        [[maybe_unused]] static const char* const reset      = "\u001B[0m";
        [[maybe_unused]] static const char* const bgBlack    = "\u001B[40m";
        [[maybe_unused]] static const char* const bgRed      = "\u001B[41m";
        [[maybe_unused]] static const char* const bgGreen    = "\u001B[42m";
        [[maybe_unused]] static const char* const bgYellow   = "\u001B[43m";
        [[maybe_unused]] static const char* const bgBlue     = "\u001B[44m";
        [[maybe_unused]] static const char* const bgMagenta  = "\u001B[45m";
        [[maybe_unused]] static const char* const bgCyan     = "\u001B[46m";
        [[maybe_unused]] static const char* const bgWhite    = "\u001B[47m";
    }

    // Simple stream manipulators to change the terminal output colors.
    std::ostream& infoColor(std::ostream& os) { os << TerminalColor::blue; return os; }
    std::ostream& warnColor(std::ostream& os) { os << TerminalColor::yellow; return os; }
    std::ostream& errorColor(std::ostream& os) { os << TerminalColor::red; return os; }
    std::ostream& fatalColor(std::ostream& os) { os << TerminalColor::black << TerminalColor::bgRed; return os; }
    std::ostream& debugColor(std::ostream& os) { os << TerminalColor::green; return os; }
    std::ostream& traceColor(std::ostream& os) { os << TerminalColor::reset; return os; }
    std::ostream& resetColor(std::ostream& os) { os << TerminalColor::reset; return os; }

    // ----------------------------------------------------------------------------------------------------
    // Logger Public Interface Implementation
    // ----------------------------------------------------------------------------------------------------

    Logger::Logger(const char* name, std::ostream& os)
            :_name(name)
    {
        _outputColorText = os.rdbuf() == std::cout.rdbuf();
        _streams.push_back(std::ref(os));
    }

    Logger::~Logger() = default;

    void Logger::addSplit(std::ostream& os)
    {
        // FIXME: The implementation of color output assumes that only one stream will be sent data, and that it matches
        //        std::cout. Once any additional streams get added they will also get the color codes sent to them and
        //        they may not know how to handle them.
        _outputColorText = false; // Disable color output for split streams. Not elegant, but easy.
        _streams.push_back(std::ref(os));
    }

    // ----------------------------------------------------------------------------------------------------
    // Logger Private Interface
    // ----------------------------------------------------------------------------------------------------

    /**
     * @brief Assembles the timestamp and log level tags at the start of a logged message.
     * @param level
     * — The kind of logging being done. (info, warning, error, etc.) This effects text coloring if logging is being
     * done to a terminal.
     */
    void Logger::buildHeader(LogLevel level)
    {
        // Get the current time.
        // By default the time is represented in nanoseconds, but adding in the duration_cast helps future-proof
        // the code a bit.
        auto curTimeNanosecondPrecision = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

        // Format the time as human readable.
        // "%F %T" --> "%Y-%m-%d %H:%M:%S" --> "TZONE 2019-08-23 13:42:58\0" (26 chars)
        char timeStr[26] = {0};
        std::time_t curTimeSecondPrecision = curTimeNanosecondPrecision/1000000000; // Deliberate integer rounding.
        std::strftime(timeStr, sizeof(timeStr), "%Z %F %T", std::localtime(&curTimeSecondPrecision));

        // Finally print the time stamp.
        _buffer << '[' << timeStr << ':' << (curTimeNanosecondPrecision - curTimeSecondPrecision*1000000000) << ']';
        _buffer << ' ';

        // Append logger name and level.
        _buffer << '[';
        _buffer << _name << ':';
        if (_outputColorText) {
            switch (level) {                                                                // Default Colors
                case LogLevel::info:  _buffer << infoColor  << "INFO"  << resetColor; break;// Blue
                case LogLevel::warn:  _buffer << warnColor  << "WARN"  << resetColor; break;// Yellow
                case LogLevel::error: _buffer << errorColor << "ERROR" << resetColor; break;// Red
                case LogLevel::fatal: _buffer << fatalColor << "FATAL" << resetColor; break;// Black on Red
                case LogLevel::debug: _buffer << debugColor << "DEBUG" << resetColor; break;// Green
                case LogLevel::trace: _buffer << traceColor << "TRACE" << resetColor; break;// Default terminal color.
                default: break;
            }
        } else {
            switch (level) {
            case LogLevel::info:  _buffer << "INFO"; break;
            case LogLevel::warn:  _buffer << "WARN"; break;
            case LogLevel::error: _buffer << "ERROR"; break;
            case LogLevel::fatal: _buffer << "FATAL"; break;
            case LogLevel::debug: _buffer << "DEBUG"; break;
            case LogLevel::trace: _buffer << "TRACE"; break;
            default: break;
            }
        }
        _buffer << "]\t";
    }

    /**
     * @brief Copies the content of the buffer to the output stream(s) and clears the buffer.
     */
    void Logger::write() {
        for (auto & streamItem : _streams) {
            streamItem.get() << _buffer.str() << std::endl;
        }
        _buffer.str("");
    }
}
