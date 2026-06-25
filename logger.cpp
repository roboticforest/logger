/**
 * @file
 * @author David Vitez (AKA: Robotic Forest)
 * @copyright All rights reserved (c) 2020-2026 David Vitez
 */

module;

#include <chrono>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
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
            const auto current_time = std::chrono::floor<std::chrono::nanoseconds>(std::chrono::system_clock::now());
            const auto seconds_time = std::chrono::floor<std::chrono::seconds>(current_time);
            const auto nanosecond_part = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - seconds_time).count();
            const std::chrono::zoned_time local_time{std::chrono::current_zone(), seconds_time};

            // Finally, print the time stamp.
            std::format_to(std::ostreambuf_iterator<char>(buffer), "[{:%Z %F %T}:{:09}] ", local_time, nanosecond_part);

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
