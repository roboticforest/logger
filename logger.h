#ifndef DV_LOGGER_H
#define DV_LOGGER_H

#include <iostream>
#include <sstream>
#include <string>

namespace DV {

    class Logger {
    public:

        // ----------------------------------------------------------------------------------------------------
        // Constructors and destructors
        // ----------------------------------------------------------------------------------------------------

        // TODO: Make a second constructor that takes in additional streams so that logging output can be tee'd.
        // TODO: Implement the ability to handle teeing output!
        explicit Logger(const char* name, std::ostream& os);
        ~Logger();

        // NOT movable.
        Logger(Logger&& rhs) = delete;
        Logger& operator=(Logger&& rhs) = delete;

        // NOT copyable.
        Logger(const Logger& rhs) = delete;
        Logger& operator=(const Logger& rhs) = delete;

        // ----------------------------------------------------------------------------------------------------
        // Public logging functions.
        // ----------------------------------------------------------------------------------------------------

        template<typename... Message> void debug (Message... msg) { this->assemble(LogLevel::debug, msg...); }
        template<typename... Message> void error (Message... msg) { this->assemble(LogLevel::error, msg...); }
        template<typename... Message> void fatal (Message... msg) { this->assemble(LogLevel::fatal, msg...); }
        template<typename... Message> void info  (Message... msg) { this->assemble(LogLevel::info,  msg...); }
        template<typename... Message> void trace (Message... msg) { this->assemble(LogLevel::trace, msg...); }
        template<typename... Message> void warn  (Message... msg) { this->assemble(LogLevel::warn,  msg...); }

    private:
        // TODO: Revisit pImpl. Figure out what can and can't be hidden while preserving the variadic templates.
        const char* _name;          // Name of the logger.
        std::ostream& _out;         // Output stream
        std::stringstream _buffer;  // Buffer for assembling the finished message to output.

        enum class LogLevel { info, warn, error, fatal, debug, trace };

        template<typename... Message>
        void assemble(LogLevel logLevel, Message... msg)
        {
            this->buildHeader(logLevel);    // Add a header to the output buffer.
            this->assemble(msg...);         // Add all message parts (via 1 of 2 assembly helpers) to the buffer.
            this->write();                  // Write the finished message buffer to the output stream.
        }

        // First message assembly helper for when there are two or more parts.
        template<typename First, typename... Rest>
        void assemble(First first, Rest... rest)
        {
            _buffer << first << ' ';
            this->assemble(rest...);
        }

        // Second message assembly helper for when there is only one part.
        template<typename Message>
        void assemble(Message msg) { _buffer << msg; }

        // First part of message assembly. Adds a header to the message based on the given logging level.
        void buildHeader(LogLevel);

        // Output the fully assembled message to the output stream.
        void write();

        bool _outputColorText;

        //const char* const infoColor();
    };
}

#endif // DV_LOGGER_H
