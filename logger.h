#ifndef DV_LOGGER_H
#define DV_LOGGER_H

#include <iostream>
#include <sstream>
#include <string>

namespace DV {

    class Logger {
      public:
        explicit Logger(const char * name, std::ostream & os);
        ~Logger();

        // NOT movable.
        Logger(Logger && rhs) = delete;
        Logger & operator=(Logger && rhs) = delete;

        // NOT copyable.
        Logger(const Logger & rhs) = delete;
        Logger & operator=(const Logger & rhs) = delete;

        template <typename... Message>
        void info(Message... msg) {
            this->assemble(LogLevel::info, msg...);
        }

        template <typename... Message>
        void warn(Message... msg) {
            this->assemble(LogLevel::warn, msg...);
        }

        template <typename... Message>
        void error(Message... msg) {
            this->assemble(LogLevel::error, msg...);
        }

        template <typename... Message>
        void fatal(Message... msg) {
            this->assemble(LogLevel::fatal, msg...);
        }

        template <typename... Message>
        void debug(Message... msg) {
            this->assemble(LogLevel::debug, msg...);
        }

        template <typename... Message>
        void trace(Message... msg) {
            this->assemble(LogLevel::trace, msg...);
        }

      private:
        // TODO: Revisit pImpl. See if everything but the buffer can be hidden.
        const char * _name;
        std::ostream & _out;
        std::stringstream _buffer;

        enum class LogLevel { info, warn, error, fatal, debug, trace };

        template <typename Message>
        void assemble(Message msg) {
            _buffer << msg;
        }
        template <typename First, typename... Rest>
        void assemble(First first, Rest... rest) {
            _buffer << first << ' ';
            this->assemble(rest...);
        }
        template <typename... Message>
        void assemble(LogLevel logLevel, Message... msg) {
            this->buildHeader(logLevel);
            this->assemble(msg...);
            this->write();
        }

        void buildHeader(LogLevel);
        void write();
    };
}

#endif // DV_LOGGER_H
