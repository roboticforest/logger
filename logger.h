/**
 * @file
 * @author David Vitez (AKA: Robotic Forest)
 * @copyright All rights reserved © 2020 David Vitez
 */

#ifndef DV_LOGGER_H
#define DV_LOGGER_H

#include <iostream>
#include <sstream>
#include <string>
#include <mutex>

/**
 * @brief "DV" is short for David Vitez.
 */
namespace DV {

    /**
     * @example "Basic Usage"
     * Create an instance of logger by giving it a name and a stream to output to.
     * @code
     * DV::Logger log("Terminal Log", std::cout);
     * float pi = 3.14;
     * log.info ("Simple test:", 5, pi, 'a', "b c");
     * @endcode
     * Output:
     * @code
     * >  [TZONE YYYY-MM-DD HH:MM:SS:NS] [Terminal Log:INFO] Simple test: 5 3.14 a b c
     * @endcode
     */
    /**
     * @brief A simple logging tool.
     * @details This is a very simple logging tool built around std::ostream, which means it can write log entries to
     * the console/terminal, to a file, or any other specialization of standard output streams.
     * @details To use the logger simply create an instance then call the appropriate logging functions like info()
     * or warn(). You can pass in any number of arguments to the logging functions in any order you wish. Every
     * argument must be printable via the usual stream output overloads (which makes custom print formats easy to
     * create) and each will automatically be separated by spaces as they are logged.
     */
    class Logger {
    public:

        // ----------------------------------------------------------------------------------------------------
        // Constructors, destructors, and other setup functions.
        // ----------------------------------------------------------------------------------------------------

        // TODO: Implement the ability to handle splitting output!
        //       Make a second constructor that takes in additional streams so that logging output can be tee'd.
        /**
         * @brief Constructs the logger.
         * @param name
         * — The name of the logger as it will appear in the header of each log entry. This can only be set during
         * creation and can not change.
         * @param os
         * — The output stream the logger will use. This can only be set during creation and can not be swapped out
         * later.
         */
        explicit Logger(const char* name, std::ostream& os);
        ~Logger();

        // NOT movable.
        Logger(Logger&& rhs) = delete;
        Logger& operator=(Logger&& rhs) = delete;

        // NOT copyable.
        Logger(const Logger& rhs) = delete;
        Logger& operator=(const Logger& rhs) = delete;

        /**
         * @brief Tees output, adding an additional stream for log entries to be sent to.
         * @param os
         * — The new additional output stream to send log entries to.
         * @return
         * — True if registering the stream was successful.
         */
         // TODO: Implement addSplit().
        bool addSplit(std::ostream& os);

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
        std::mutex _writeMutex;     // For protecting buffering and write operations from threads.

        /**
         * @brief Specifies the type of log entry being created.
         * @details INFO is a general purpose level. Good for progress reports and normal operations.
         * @details WARN is for anything that could be a problem but doesn't necessarily mean the program is unusable.
         * It can also be a general alert, or mark the beginning of risky operations.
         * @details ERROR is for definite problems, such as when a needed resource fails to load, or a function throws
         * an exception.
         * @details FATAL is for anything that causes the entire program to terminate. Using this log level does not
         * shutdown the program like it does in some other loggers. It is simply for reporting. Shutting down the
         * software is the software's problem, not the logger's.
         * @details DEBUG is for any messages that expose detailed under-the-hood types of information, such as pointer
         * addresses, variables names, etc.
         * @details TRACE is for reporting function calls and program flow. This does not actually monitor the call
         * stack or in any way track functions or call depth. Like all other logging levels it is simply for organizing
         * log entries.
         * @note 1. In the future this tool may be expanded so that logging of different levels can be turned off, but
         * currently that is not the case. If that is ever implemented the order specified here could be important as,
         * for example, setting the level to "fatal" would allow for logging of all types of messages except debug and
         * trace.
         * @note 2. In the future this tool may be expanded so that trace maintains a breadcrumb like listing of
         * functions. This has not been looked into very closely yet as it was firmly decided that this would not be a
         * v1.0 feature, but it is being considered for the future.
         */
        enum class LogLevel { info, warn, error, fatal, debug, trace };

        template<typename... Message>
        void assemble(LogLevel logLevel, Message... msg)
        {
            _writeMutex.lock();
            this->buildHeader(logLevel);    // Add a header to the output buffer.
            this->assemble(msg...);         // Add all message parts (via 1 of 2 assembly helpers) to the buffer.
            this->write();                  // Write the finished message buffer to the output stream.
            _writeMutex.unlock();
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

        // For controlling if color codes should be printed. This is managed by the logger internally if it detects
        // that it is printing to a console/terminal or if it is printing to something else.
        bool _outputColorText;
    };
}

#endif // DV_LOGGER_H
