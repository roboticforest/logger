/**
 * @file
 * @author David Vitez (AKA: Robotic Forest)
 * @copyright All rights reserved (c) 2020 David Vitez
 */

module;

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

export module logger;

export namespace DV {
    namespace Version {
        inline constexpr std::string_view string = DVLOGGER_VERSION_STRING;
        inline constexpr int major = DVLOGGER_VERSION_MAJOR;
        inline constexpr int minor = DVLOGGER_VERSION_MINOR;
        inline constexpr int patch = DVLOGGER_VERSION_PATCH;
        inline constexpr int tweak = DVLOGGER_VERSION_TWEAK;
    }

    class Logger {
    public:
        // ----------------------------------------------------------------------------------------------------
        // Constructors, destructors, and other setup functions.
        // ----------------------------------------------------------------------------------------------------

        explicit Logger(const char* name, std::ostream& os);
        void addSplit(std::ostream& os);

        Logger(Logger&& rhs) = delete; // Move Constructor
        Logger& operator=(Logger&& rhs) = delete; // Move Assignment Operator

        Logger(const Logger& rhs) = delete; // Copy Constructor
        Logger& operator=(const Logger& rhs) = delete; // Copy Assignment Operator

        ~Logger();

        // ----------------------------------------------------------------------------------------------------
        // Public logging functions.
        // ----------------------------------------------------------------------------------------------------

        template <typename... Message> void debug(Message&&... msg) { this->emit(LogLevel::debug, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void error(Message&&... msg) { this->emit(LogLevel::error, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void fatal(Message&&... msg) { this->emit(LogLevel::fatal, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void info(Message&&... msg) { this->emit(LogLevel::info, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void trace(Message&&... msg) { this->emit(LogLevel::trace, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void warn(Message&&... msg) { this->emit(LogLevel::warn, this->assemblePayload(std::forward<Message>(msg)...)); }

    private:
        struct Impl;
        std::unique_ptr<Impl> _impl;

        enum class LogLevel { info, warn, error, fatal, debug, trace };

        template <typename First, typename... Rest>
        std::string assemblePayload(First&& first, Rest&&... rest) {
            std::ostringstream payload;
            payload << std::forward<First>(first);
            ((payload << ' ' << std::forward<Rest>(rest)), ...);
            return payload.str();
        }

        // Non-template backend seam for module/pImpl migration work.
        void emit(LogLevel, std::string_view payload);
    };
}
