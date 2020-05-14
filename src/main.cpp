#include <fstream>
#include <iostream>
#include <streambuf>

using namespace std;

#include "tomasulo.hpp"

int main() {
    Tomasulo tmsl;
    ifstream t("./TestCase/1.basic.nel");
    string nel((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());
    tmsl.set_nel(nel);
    while (!tmsl.has_done()) {
        tmsl.run(1);
        // tmsl.print_clock();
        // tmsl.print_amrs();
        // tmsl.print_lb();
        // tmsl.print_reg();
        // tmsl.print_fu();
    }
    tmsl.print_log();
    return 0;
}
