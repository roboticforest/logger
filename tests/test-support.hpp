#ifndef DVLOGGER_TEST_SUPPORT_HPP
#define DVLOGGER_TEST_SUPPORT_HPP

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace dvlogger_test {

    /**
     * @brief Parses optional test output forwarding arguments and replays captured output.
     * @details This helper recognizes `--show-output` and `--write-output <path>`,
     * then forwards the captured stream contents to the requested destinations.
     *
     * `--show-output` sends the captured output to std::out.
     * `--write-output <path>` writes the captured output to the specified file.
     */
    class OutputForwarder {
    public:
        /**
         * @brief Creates an output forwarder for a captured test stream.
         * @param argc The command-line argument count passed to the test.
         * @param argv The command-line argument values passed to the test.
         * @param captured_output The filled output stream whose contents may be forwarded.
         */
        OutputForwarder(const int argc, char* argv[], const std::ostringstream& captured_output)
            : _captured_output(captured_output) {
            _parse_arguments(argc, argv);
        }

        /**
         * @brief Forwards the captured output to any requested destinations.
         * @details When `--show-output` is enabled, this writes the captured
         * output to `std::cout`. When `--write-output <path>` is provided, this
         * writes the same captured output to the requested file in binary mode.
         * Any forwarding errors are reported to `std::cerr`.
         */
        void forward_output() const {

            if(_show_output) { _write_data(std::cout); }

            if(!_output_file_path.empty()) {
                std::ofstream output_file(_output_file_path, std::ios::binary);
                if(!output_file.is_open()) {
                    std::cerr << "Could not open test output file: " << _output_file_path << std::endl;
                }

                _write_data(output_file);
                if(!output_file.good()) {
                    std::cerr << "Could not write data to test output file: " << _output_file_path << std::endl;
                }
                output_file.close();
            }
        }

    private:
        std::ostringstream const& _captured_output;
        bool _show_output = false;
        std::string _output_file_path;

        /**
         * @brief Reads supported command-line arguments for output forwarding.
         * @param argc The command-line argument count passed to the test.
         * @param argv The command-line argument values passed to the test.
         * @details This recognizes `--show-output` and `--write-output <path>`.
         * Invalid arguments and missing file paths are reported to `std::cerr`.
         */
        void _parse_arguments(const int argc, char* argv[]) {
            for(int i = 1; i < argc; ++i) {
                const std::string_view arg = argv[i];

                if(arg == "--show-output") {
                    _show_output = true;
                    continue;
                }

                if(arg == "--write-output") {
                    ++i;
                    if(i >= argc) {
                        std::cerr << "Missing file path after --write-output." << std::endl;
                        return;
                    }

                    _output_file_path = argv[i];
                    continue;
                }

                std::cerr << std::string("Unknown argument: ") + std::string(arg) << std::endl;
                return;
            }
        }

        /**
         * @brief Writes captured output bytes to an output stream.
         * @param output_stream The destination stream that receives the output.
         * @details This writes the provided text directly to the destination
         * stream without altering its contents.
         */
        void _write_data(std::ostream& output_stream) const {
            const std::string_view output_text = _captured_output.view();
            output_stream.write(output_text.data(), static_cast<std::streamsize>(output_text.size()));
        }
    };

    /**
     * @brief Reports a failed test expectation.
     * @param condition The condition being evaluated.
     * @param message The failure message to print when the condition is false.
     * @return `true` when the condition passes, otherwise `false`.
     * @details This helper prints to `std::cerr` only when the expectation fails,
     * which lets a test accumulate multiple checks before returning.
     */
    inline bool expect(const bool condition, const std::string& message) {
        if(!condition) {
            std::cerr << message << std::endl;
            return false;
        }
        return true;
    }

    /**
     * @brief Extracts the message text from a formatted log line.
     * @param line The full log line, including the logger prefix.
     * @return The portion of the line after the logger header delimiter, or an
     * empty string when the delimiter is not present.
     * @details This helper looks for the `"]\t"` marker that separates the
     * logger metadata from the message payload in the current output format.
     */
    inline std::string message_payload(const std::string& line) {
        const std::string marker = "]\t";
        const std::size_t pos = line.find(marker);
        if(pos == std::string::npos) {
            return {};
        }
        return line.substr(pos + marker.size());
    }

    /**
     * @brief Splits a block of text into lines.
     * @param text The text to break into individual lines.
     * @param preserve_empty_lines When `true`, blank lines are kept in the result.
     * @return A list of lines with trailing carriage returns removed.
     * @details Each returned line has any trailing `'\r'` removed. Blank lines
     * are omitted by default and are preserved only when requested.
     */
    inline std::vector<std::string> split_lines(const std::string& text, const bool preserve_empty_lines = false) {
        std::vector<std::string> lines;
        std::istringstream input(text);
        std::string line;

        while(std::getline(input, line)) {
            if(!line.empty() && line.back() == '\r') { line.pop_back(); }
            if(line.empty() && !preserve_empty_lines) { continue; }
            lines.push_back(line);
        }
        return lines;
    }


    /**
     * @brief Escapes text so it can be matched literally inside a regular expression.
     * @param text The raw text to escape for regex use.
     * @return A string where regex-special characters and whitespace are backslash-escaped.
     */
    inline std::string regex_escape(const std::string_view text) {
        static const std::regex re(R"([-[\]{}()*+?.,\^$|#\s])");
        return std::regex_replace(std::string(text), re, R"(\$&)");
    }

    /**
     * @brief Checks whether a log line begins with the expected logger header.
     * @param line The complete formatted log line to inspect.
     * @param logger_name The expected logger name inside the header.
     * @param level_name The expected log level label inside the header.
     * @return `true` when the line begins with a matching timestamp, logger name,
     * and level label, otherwise `false`.
     * @details This helper validates the formatted prefix up to the message payload
     * delimiter and requires the full timestamp format emitted by the logger.
     */
    inline bool has_log_prefix(
        const std::string& line,
        std::string_view logger_name,
        std::string_view level_name) {
        const std::string escaped_name = regex_escape(logger_name);
        const std::string escaped_level = regex_escape(level_name);
        const std::regex pattern(
            R"(^\[[^\]]*\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}:\d{9}\] \[)"
            + escaped_name + ":" + escaped_level + "\\]\\t");
        return std::regex_search(line, pattern);
    }
} // namespace dvlogger_test

#endif  // DVLOGGER_TEST_SUPPORT_HPP
