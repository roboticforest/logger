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

    /**
     * @brief A simple logging tool.
     * @details This is a very simple logging tool built around std::ostream, which means it can write log entries to
     * the console/terminal, to a file, or any other specialization of standard output streams.
     * @details To use the logger, create an instance, passing in a reference to a std::ostream it should use, then call
     * the appropriate logging functions like info() or warn(). You can pass in any number of arguments to the logging
     * functions in any order you wish. Every argument must be printable via the usual stream output overloads (which
     * makes custom print formats easy to create), and each argument will automatically be separated with spaces.
     * @see @ref log_levels
     */
    class Logger {
    public:
        // ----------------------------------------------------------------------------------------------------
        // Constructors, destructors, and other setup functions.
        // ----------------------------------------------------------------------------------------------------

        /** @brief Build/version information for this logger library. */
        struct Version {
            static inline constexpr std::string_view string = DVLOGGER_VERSION_STRING;
            static inline constexpr int major = DVLOGGER_VERSION_MAJOR;
            static inline constexpr int minor = DVLOGGER_VERSION_MINOR;
            static inline constexpr int patch = DVLOGGER_VERSION_PATCH;
            static inline constexpr int tweak = DVLOGGER_VERSION_TWEAK;
        };

        /**
         * @brief Constructs the logger.
         * @param name
         * The name of the logger as it will appear in the header of each log entry.
         * @param os
         * The output stream the logger will use.
         */
        explicit Logger(const char* name, std::ostream& os);

        /**
         * @brief Tees output.
         * @details Adds a reference to an output stream for log entries to be sent to. This duplicates the logger's
         * output to multiple locations.
         * @param os
         * The new additional output stream to send log entries to.
         */
        void addSplit(std::ostream& os);

        Logger(Logger&& rhs) = delete; // Move Constructor
        Logger& operator=(Logger&& rhs) = delete; // Move Assignment Operator

        Logger(const Logger& rhs) = delete; // Copy Constructor
        Logger& operator=(const Logger& rhs) = delete; // Copy Assignment Operator

        ~Logger();

        // ----------------------------------------------------------------------------------------------------
        // Public logging functions.
        // ----------------------------------------------------------------------------------------------------

        /**
         * @name Primary Logging Functions
         * @brief These are the primary logging functions. Each call prints one log entry.
         * @details Each logging function takes an arbitrary list of arguments and converts them into one text payload.
         * Level semantics are documented in @ref log_levels.
         * @param msg
         * The message arguments to record as one log entry payload.
         */
        ///@{
        template <typename... Message> void debug(Message&&... msg) { this->emit(LogLevel::debug, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void error(Message&&... msg) { this->emit(LogLevel::error, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void fatal(Message&&... msg) { this->emit(LogLevel::fatal, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void info(Message&&... msg) { this->emit(LogLevel::info, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void trace(Message&&... msg) { this->emit(LogLevel::trace, this->assemblePayload(std::forward<Message>(msg)...)); }
        template <typename... Message> void warn(Message&&... msg) { this->emit(LogLevel::warn, this->assemblePayload(std::forward<Message>(msg)...)); }
        ///@}

    private:
        struct Impl;
        std::unique_ptr<Impl> _impl;

        /**
         * @brief Specifies the type of log entry being created.
         */
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
