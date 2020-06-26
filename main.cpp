#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "logger.h"

using namespace std;

int main() {
    // std::ofstream file("log.txt");
    // DV::Logger logger("Main", file);
    DV::Logger log("Main", std::cout);
    log.info("Program started.");
    log.warn("User may or may not enter a number greater than zero!");

    bool done = false;
    while(!done) {
        cout << "Hello. Please enter a number (zero to quit): " << flush;
        int i = 0;
        cin >> i;
        if(i > 0) {
            log.info("Starting dot printing loop.");
            while(i >= 0) {
                cout << '.' << flush;
                --i;
            }
            cout << endl;
            log.info("Loop done!");
        } else {
            if(i == 0) {
                done = true;
            } else {
                log.error("Var i was not > 0! i ==", i);
            }
        }
    }
    return 0;
}
