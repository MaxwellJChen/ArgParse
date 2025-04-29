#include <functional>
#include <string>
#include <array>
#include <tuple>
#include <iostream>
#include "argparse.h"

// function to add to ArgParse database
void foo(int x, float y, double z) {
    std::cout<<x + y * z<<std::endl;
}
  
int main() {
    ArgParse ap;

    // add the command based on specified path
    // must provide number of arguments and argument types at compile time
    ap.add_command<3, int, float, double>({"bar", "baz", "foo"}, foo);

    // traverses the same path again and calls the foo function
    ap.execute_command({"bar", "baz", "foo"}, {"10", "12.3", "30.5013"}); // prints 385.166
}