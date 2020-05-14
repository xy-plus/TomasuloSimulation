// #include <sys/time.h>
#include <chrono>
// #include <ctime>
#include <fstream>
#include <iostream>
#include <streambuf>

using namespace std;

#include "tomasulo.hpp"

double microtime() {
    return (double(std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count()) /
            double(1000000));
}

int main(int argc, char** argv) {
    Tomasulo tmsl;
    ifstream t(string("./TestCase/") + argv[1]);
    string nel((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());
    tmsl.set_nel(nel);
    auto start = microtime();
    while (!tmsl.has_done()) {
        tmsl.run(1);
#ifdef PRINT_STATE
        tmsl.print_clock();
        tmsl.print_amrs();
        tmsl.print_lb();
        tmsl.print_reg();
        tmsl.print_fu();
#endif
    }
    // cout << "time: " << microtime() - start << "s" << endl;
    tmsl.print_log();
    return 0;
}
