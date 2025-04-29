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

struct test_t {
    int a;
};

void test(test_t t) {
    std::cout<<t.a<<std::endl;
}
  
int main(int argc, char* argv[]) {
    ArgParse ap;

    ap.add_command<3, int, float, double>({"bar", "baz", "foo"}, foo);
    ap.execute_command({"bar", "baz", "foo"}, {"10", "12.3", "30.5013"}); // 385.166

    ap.add_command<1, int>({"bar"}, bar);
    ap.execute_command({"bar"}, {"20"}); // 40

    ap.add_command<0>({"baz", "asdlfajsldkfjalksdfjaklsjdflkajsldkfj"}, baz);
    ap.execute_command({"baz", "asdlfajsldkfjalksdfjaklsjdflkajsldkfj"}, {}); // baz

    ap.add_command<0>({"class"}, Foo::foo);
    ap.execute_command({"class"}, {}); // foo

    ap.add_conversion<test_t>([](std::string s) {
        return test_t { 3 * stoi(s) };
    });
    ap.add_command<1, test_t>({"test"}, test);
    ap.execute_command({"test"}, {"30"}); // 90

    ap.add_alias({"baz", "asdlfajsldkfjalksdfjaklsjdflkajsldkfj"}, "a");
    ap.execute_command({"baz", "a"}, {});

    ap.execute_command(argc, argv);
}