#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <sstream>
#include "../argparse.h"


void bar(int x, float y, double z) {
    std::cout<<x + y * z<<std::endl;
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

class ArgParseTests : public ::testing::Test {
protected:
    std::stringstream input_buffer;
    std::stringstream output_buffer;
    std::streambuf* original_cin;
    std::streambuf* original_cout;

    void SetUp() override {
        original_cin = std::cin.rdbuf(input_buffer.rdbuf());
        original_cout = std::cout.rdbuf(output_buffer.rdbuf());
    }

    void TearDown() override {
        std::cin.rdbuf(original_cin);
        std::cout.rdbuf(original_cout);
        input_buffer.str("");
        input_buffer.clear();
        output_buffer.str("");
        output_buffer.clear();
    }
};


TEST_F(ArgParseTests, SingleArgumentTest) {
    ArgParse ap;

    void (*func)(int) = [](int x) {
        std::cout<<x * 2<<std::endl;
    };

    ap.add_command<1, int>({"bar", "baz", "foo"}, func);
    ap.execute_command({"bar", "baz", "foo"}, {"500"});

    EXPECT_EQ(output_buffer.str(), "1000\n");
}