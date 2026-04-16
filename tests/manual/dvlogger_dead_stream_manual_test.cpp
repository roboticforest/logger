#include <fstream>
#include <iostream>

#include "logger.h"

int main()
{
    std::cout << "Manual UB test: this executable may crash by design." << std::endl;

    auto* file = new std::ofstream("doomed.log");
    DV::Logger file_log("DoomedLogger", *file);
    file_log.info("File created.");

    file->close();
    delete file;

    // Deliberate use-after-delete scenario from legacy stress harness.
    file_log.error("Attempting write after stream deletion.");

    return 0;
}
