#include "logger.h"

#include <ctime>
#include <sstream>
#include <string>

namespace DV {

    // TODO: Prevent color format codes from being inserted into the output stream when the
    // stream is not attached to a terminal. Do this during logger construction.
    // TEMP CODE! Proof of concept! This code works!
    /*
    if ( _out.rdbuf() == std::cout.rdbuf() ) {
        _out << "CONSOLE STREAM:\t";
    } else {
        _out << "FILE STREAM:\t";
    }
    */

    // TODO: Setup variables for storing these values for select uses. For example: create an
    // infoColor variable that can be set during logger construction and use that instead of using
    // these strings directly from this namespace.
    namespace ConsoleColor {
        static const char * const black     = "\u001B[30m";
        static const char * const red       = "\u001B[31m";
        static const char * const green     = "\u001B[32m";
        static const char * const yellow    = "\u001B[33m";
        static const char * const blue      = "\u001B[34m";
        static const char * const magenta   = "\u001B[35m";
        static const char * const cyan      = "\u001B[36m";
        static const char * const white     = "\u001B[37m";
        static const char * const reset     = "\u001B[0m";
        static const char * const bgBlack   = "\u001B[40m";
        static const char * const bgRed     = "\u001B[41m";
        static const char * const bgGreen   = "\u001B[42m";
        static const char * const bgYellow  = "\u001B[43m";
        static const char * const bgBlue    = "\u001B[44m";
        static const char * const bgMagenta = "\u001B[45m";
        static const char * const bgCyan    = "\u001B[46m";
        static const char * const bgWhite   = "\u001B[47m";
    }

////////////////////////////////////////////////////////////////////////////////
/// Logger Public Interface Implementation
////////////////////////////////////////////////////////////////////////////////

    Logger::Logger(const char * name, std::ostream & os) : _name(name), _out(os) {}
    Logger::~Logger() {}

////////////////////////////////////////////////////////////////////////////////
/// Logger Private Interface
////////////////////////////////////////////////////////////////////////////////

    void Logger::buildHeader(LogLevel level) {

        // Usage of std::chrono does not improve the following code. std::chrono is built more for timing things than
        // for handling dates. In fact, std::chrono:system_clock:now() returns the exact same value as std::time()
        // which should be the number seconds that have passed since a set epoch. Then if you want to print out that
        // time the only option is to convert it to a std::time_t and use the C formatting functions to print
        // something nice.

        // As for the use of clock_t for measuring sub-second times, on most (if not all) modern systems this should
        // be a long int of 64 bits in size, and the CLOCKS_PER_SEC macro should have a value of 1,000,000
        // (microseconds). If you do the math, that means that this signed type will overflow into the negative in
        // just under 230,000 years. On older systems this may be an issue (maybe not) as the clock_t could be a 32
        // bit signed type with CLOCKS_PER_SEC set to 1,000 (milliseconds) so this type would overflow into the
        // negative in just under 25 days of run time for whatever application it is used in.

        // Get the current time.
        std::time_t curTime  = std::time(nullptr);
        std::clock_t curTick = std::clock();

        // "%F %T" --> "%Y-%m-%d %H:%M:%S" --> "2019-08-23 13:42:58\0" (20 chars)
        char timeStr[20] = {0};
        std::strftime(timeStr, sizeof(timeStr), "%F %T", std::localtime(&curTime));

        _buffer << '[' << timeStr << ':' << curTick << ']';
        _buffer << ' ';

        // Append logger name and level.
        _buffer << '[';
        _buffer << _name << ':';
        switch(level) {
            case LogLevel::info:
                _buffer << ConsoleColor::blue << "INFO" << ConsoleColor::reset;
                break;
            case LogLevel::warn:
                _buffer << ConsoleColor::yellow << "WARN" << ConsoleColor::reset;
                break;
            case LogLevel::error:
                _buffer << ConsoleColor::red << "ERROR" << ConsoleColor::reset;
                break;
            case LogLevel::fatal:
                _buffer << ConsoleColor::black << ConsoleColor::bgRed << "FATAL"
                        << ConsoleColor::reset;
                break;
            case LogLevel::debug:
                _buffer << ConsoleColor::green << "DEBUG" << ConsoleColor::reset;
                break;
            case LogLevel::trace: _buffer << "TRACE"; break;
            default: break;
        }
        _buffer << "]\t";
    }

    // TODO: Is this thread safe? Do I need to lock a mutex? Supposedly cout is already thread safe,
    // but every example I've found says that you need to lock a mutex around output operations.
    void Logger::write() { _out << _buffer.rdbuf() << std::endl; }

}
