#include <functional>
#include <string>
#include <array>
#include <tuple>
#include <iostream>
#include "argparse.h"

void baz(int x, double y, float z) {
    std::cout<<x * y + z<<std::endl;
}
 
int main() {
    ArgParse argparse;
    argparse.add_command<3, int, double, float>({"foo", "bar", "baz"}, baz);
    argparse.execute_command({"foo", "bar", "baz"}, {"10", "11.2", "12.3"});
}