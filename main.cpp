#include <functional>
#include <string>
#include <array>
#include <tuple>
#include <iostream>
#include "argparse.h"
#include "foo.h"

// function to add to ArgParse database
void foo(int x, float y, double z) {
    std::cout<<x + y * z<<std::endl;
}

void bar(int x) {
    std::cout<<x * 2<<std::endl;
}

void baz() {
    std::cout<<"baz"<<std::endl;
}
  
int main() {
    ArgParse ap;

    ap.add_command<3, int, float, double>({"bar", "baz", "foo"}, foo);
    ap.execute_command({"bar", "baz", "foo"}, {"10", "12.3", "30.5013"});

    ap.add_command<1, int>({"bar"}, bar);
    ap.execute_command({"bar"}, {"20"});

    ap.add_command<0>({"baz", "asdlfajsldkfjalksdfjaklsjdflkajsldkfj"}, baz);
    ap.execute_command({"baz", "asdlfajsldkfjalksdfjaklsjdflkajsldkfj"}, {});

    ap.add_command<0>({"class"}, Foo::foo);
    ap.execute_command({"class"}, {});
}