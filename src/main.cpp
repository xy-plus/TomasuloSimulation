#include <fstream>
#include <iostream>
#include <streambuf>

using namespace std;

#include "tomasulo.hpp"

int main() {
    Tomasulo tmsl;
    ifstream t("./TestCase/Fabo.nel");
    string nel((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());
    tmsl.set_nel(nel);
    int i = 0;
    while (!tmsl.has_done()) {
        tmsl.run(1);
        // tmsl.print_clock();
        // tmsl.print_amrs();
        // tmsl.print_lb();
        // tmsl.print_reg();
        // tmsl.print_fu();
        // tmsl.print_debug();
    }
    tmsl.print_log();
}
