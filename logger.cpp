/**
 * @file
 * @author David Vitez (AKA: Robotic Forest)
 * @copyright All rights reserved (c) 2020 David Vitez
 */

module;

#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

module logger;

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

    struct Logger::Impl {
        const char* name;
        std::vector<std::reference_wrapper<std::ostream>> streams;
        std::stringstream buffer;
        std::mutex write_mutex;
        bool output_color_text;

        explicit Impl(const char* logger_name, std::ostream& primary_stream)
            : name(logger_name),
              output_color_text(primary_stream.rdbuf() == std::cout.rdbuf())
        {
            streams.push_back(std::ref(primary_stream));
        }

        void addSplit(std::ostream& os)
        {
            // FIXME: The implementation of color output assumes that only one stream will be sent data, and that it
            //        matches std::cout. Once any additional streams get added they will also get color codes sent to
            //        them and they may not know how to handle them.
            output_color_text = false;  // Disable color output for split streams. Not elegant, but easy.
            streams.push_back(std::ref(os));
        }

        void buildHeader(LogLevel level)
        {
            // Get the current time.
            // By default, the time is represented in nanoseconds, but adding in the duration_cast helps future-proof
            // the code a bit.
            auto curTimeNanosecondPrecision = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();

            // Format the time as human readable.
            // "%F %T" --> "%Y-%m-%d %H:%M:%S" --> "TZONE 2019-08-23 13:42:58\0" (26 chars)
            char timeStr[26] = {0};
            std::time_t curTimeSecondPrecision = curTimeNanosecondPrecision / 1000000000; // Deliberate integer rounding.
            std::strftime(timeStr, sizeof(timeStr), "%Z %F %T", std::localtime(&curTimeSecondPrecision));

            // Finally, print the time stamp.
            buffer << '['
                << timeStr
                << ':'
                << std::setw(9) << std::right << std::setfill('0')
                << (curTimeNanosecondPrecision - curTimeSecondPrecision * 1000000000)
                << ']';
            buffer << ' ';

            // Append logger name and level.
            buffer << '[';
            buffer << name << ':';
            if (output_color_text) {
                switch (level) {                                                                   // Default Colors
                    case LogLevel::info:  buffer << infoColor  << "INFO"  << resetColor; break; // Blue
                    case LogLevel::warn:  buffer << warnColor  << "WARN"  << resetColor; break; // Yellow
                    case LogLevel::error: buffer << errorColor << "ERROR" << resetColor; break; // Red
                    case LogLevel::fatal: buffer << fatalColor << "FATAL" << resetColor; break; // Black on Red
                    case LogLevel::debug: buffer << debugColor << "DEBUG" << resetColor; break; // Green
                    case LogLevel::trace: buffer << traceColor << "TRACE" << resetColor; break; // Default terminal color.
                    default: break;
                }
            } else {
                switch (level) {
                case LogLevel::info:  buffer << "INFO"; break;
                case LogLevel::warn:  buffer << "WARN"; break;
                case LogLevel::error: buffer << "ERROR"; break;
                case LogLevel::fatal: buffer << "FATAL"; break;
                case LogLevel::debug: buffer << "DEBUG"; break;
                case LogLevel::trace: buffer << "TRACE"; break;
                default: break;
                }
            }
            buffer << "]\t";
        }

        void write()
        {
            for (auto& stream_item : streams) {
                stream_item.get() << buffer.str() << std::endl;
            }
            buffer.str("");
        }
    };

    // ----------------------------------------------------------------------------------------------------
    // Logger Public Interface Implementation
    // ----------------------------------------------------------------------------------------------------

    Logger::Logger(const char* name, std::ostream& os)
        : _impl(std::make_unique<Impl>(name, os))
    {}

    Logger::~Logger() = default;

    void Logger::addSplit(std::ostream& os)
    {
        _impl->addSplit(os);
    }

    // ----------------------------------------------------------------------------------------------------
    // Logger Private Interface
    // ----------------------------------------------------------------------------------------------------

    void Logger::emit(LogLevel level, std::string_view payload)
    {
        std::lock_guard<std::mutex> lock(_impl->write_mutex);
        _impl->buildHeader(level);
        _impl->buffer << payload;
        _impl->write();
    }
}
