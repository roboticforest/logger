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
#include <vector>

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
     * @details To use the logger simply create an instance, passing in a reference to a std::ostream it should use,
     * then call the appropriate logging functions like info() or warn(). You can pass in any number of arguments to
     * the logging functions in any order you wish. Every argument must be printable via the usual stream output
     * overloads (which makes custom print formats easy to create) and each argument will automatically be separated
     * with spaces as they are printed.
     */
    class Logger {
    public:

        // ----------------------------------------------------------------------------------------------------
        // Constructors, destructors, and other setup functions.
        // ----------------------------------------------------------------------------------------------------

        /**
         * @name Construction And Setup
         * */
        ///@{
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

        /**
         * @brief Tees output.
         * @details Adds an additional reference to an output stream for log entries to be sent to. This duplicates the
         * logger's output to multiple locations. There is no limit to the number of new output streams that can be
         * registered using this function, though in practice, if duplicating output is needed at all, it is not
         * usually to more than one or two new streams.
         * @param os
         * — The new additional output stream to send log entries to.
         */
        void addSplit(std::ostream& os);
        ///@}

        /**
         * @name Copying And Moving
         * @brief Loggers are meant to be passed around by reference and can not be copied or moved.
         * @details Often loggers are available globally to an entire program, or semi-globally within from within
         * specific subsystems. This logging utility library was designed with that intended use in mind. It is easy to
         * pass around references to loggers though, which is generally a good idea for objects of any decent size or
         * complexity.
         */
        ///@{
        Logger(Logger&& rhs) = delete; // Move Constructor
        Logger& operator=(Logger&& rhs) = delete; // Move Assignment Operator

        Logger(const Logger& rhs) = delete; // Copy Constructor
        Logger& operator=(const Logger& rhs) = delete; // Copy Assignment Operator
        ///@}

        ~Logger(); // Default.

        // ----------------------------------------------------------------------------------------------------
        // Public logging functions.
        // ----------------------------------------------------------------------------------------------------

        /**
         * @name Primary Logging Functions
         * @brief These are the primary logging functions. Each call prints one log entry.
         * @details These functions assemble and print log entries using the stream(s) provided during setup. All the
         * heavy lifting is done here. Working identically, each logging function takes an arbitrary list of arguments
         * and converts them all into a single string of text to be printed. Each call is considered a single entry in
         * the log (and is usually, though not necessarily, one line) and will automatically have a timestamp and header
         * appended to the beginning of the entry. The only difference between each function is which header name tag
         * (and coloring) gets printed. These tags are extremely useful for sorting log data.
         * @details info() is typically for general messages, often even being used in release builds of software, which
         * give broad descriptions of what the software is doing.
         * @details warn() is often useful for announcing risky actions, or minor errors that don't leave the program
         * unstable.
         * @details error() is for recording errors. Usually unexpected things that prevent the program or end user from
         * performing a desired action.
         * @details fatal() is for errors that leave the program in a state that it can't recover from. Note that unlike
         * some other logging utilities, calling fatal() doesn't shut down the program, it only reports log entries with
         * the appropriate tag.
         * @details debug() is for logging detailed under-the-hood types of information, such as pointer addresses,
         * variables names, etc.
         * @details trace() is usually used for recording when function calls happen and program flow notifications.
         * This does not actually monitor the call stack. Like all other logging functions it simply writes out an
         * appropriate log header.
         * @param msg
         * — The message to record as a log entry. This is a list of printable arguments that will be collected
         * together into one text string, each separated with spaces. The arguments can be strings of text, variables,
         * constants, and even user-defined types, so long as each has an overload of the standard stream operator they
         * will be accepted.
         */
        ///@{
        template<typename... Message> void debug (Message... msg) { this->assemble(LogLevel::debug, msg...); }
        template<typename... Message> void error (Message... msg) { this->assemble(LogLevel::error, msg...); }
        template<typename... Message> void fatal (Message... msg) { this->assemble(LogLevel::fatal, msg...); }
        template<typename... Message> void info  (Message... msg) { this->assemble(LogLevel::info,  msg...); }
        template<typename... Message> void trace (Message... msg) { this->assemble(LogLevel::trace, msg...); }
        template<typename... Message> void warn  (Message... msg) { this->assemble(LogLevel::warn,  msg...); }
        ///@}

    private:
        // TODO: Revisit pImpl. Figure out what can and can't be hidden while preserving the variadic templates.
        const char* _name;          // Name of the logger.
        // std::ostream& _out;         // Output stream
        std::vector<std::reference_wrapper<std::ostream>> _streams;    // All output streams (usually 1, maybe 2).
        std::stringstream _buffer;  // Buffer for assembling the finished message to output.
        std::mutex _writeMutex;     // For protecting buffering and write operations from threads.
        bool _outputColorText;      // For stopping color codes from being used when not printing to std::cout.

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
    };
}

#endif // DV_LOGGER_H
