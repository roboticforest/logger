import logger;

#include <sstream>
#include <string>

int main()
{
    std::ostringstream captured;
    DV::Logger log("InstallSmoke", captured);
    log.info("ok");

    return captured.str().find("InstallSmoke:INFO") == std::string::npos;
}
