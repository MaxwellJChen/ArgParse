#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <sstream>
#include "../argparse.h"

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

    int argc = 5;
    char* argv[] = {"argparse", "bar", "baz", "foo", "500"};
    ap.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "1000\n");
}

TEST_F(ArgParseTests, MultiArgumentTest) {
    void (*func)(int, float, double) = [](int x, float y, double z) {
        std::cout<<x + y * z<<std::endl;
    };

    ArgParse ap;
    ap.add_command<3, int, float, double>({"bar", "baz", "foo"}, func);

    int argc = 7;
    char* argv[] = {"argparse", "bar", "baz", "foo", "10", "12.3", "30.5013"};
    ap.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "385.166\n");
}

TEST_F(ArgParseTests, CustomTypeTest) {
    struct test_t {
        int a;
    };

    void (*func)(test_t) = [](test_t t) {
        std::cout<<t.a * 2<<std::endl;
    };

    ArgParse ap;
    ap.add_conversion<test_t>([](std::string s) {
        return test_t { stoi(s) };
    });
    ap.add_command<1, test_t>({"test"}, func);

    int argc = 3;
    char* argv[] = {"argparse", "test", "500"};
    ap.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "1000\n");
}

TEST_F(ArgParseTests, AliasTest) {
    ArgParse ap;

    void (*func)(int) = [](int x) {
        std::cout<<x * 2<<std::endl;
    };

    ap.add_command<1, int>({"bar", "baz", "foo"}, func);
    ap.add_alias({"bar"}, "b");
    ap.add_alias({"bar", "baz"}, "b");
    ap.add_alias({"bar", "baz", "foo"}, "f");


    int argc = 5;
    char* argv[] = {"argparse", "b", "b", "f", "500"};
    ap.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "1000\n");
}

TEST_F(ArgParseTests, NoArgumentsTest) {
    ArgParse ap;

    void (*func)(void) = []() {
        std::cout<<"test"<<std::endl;
    };

    ap.add_command<0>({"bar", "baz", "foo"}, func);


    int argc = 4;
    char* argv[] = {"argparse", "bar", "baz", "foo"};
    ap.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "test\n");
}

TEST_F(ArgParseTests, MultipleFunctionsTest) {
    ArgParse ap;

    void (*func1)(int) = [](int x) {
        std::cout<<x * 2<<std::endl;
    };
    void (*func2)(int, float, double) = [](int x, float y, double z) {
        std::cout<<x + y * z<<std::endl;
    };
    void (*func3)(void) = []() {
        std::cout<<"test"<<std::endl;
    };

    ap.add_command<1, int>({"foo", "bar", "func1"}, func1);
    ap.add_command<3, int, float, double>({"foo", "bar", "func2"}, func2);
    ap.add_command<0>({"func3"}, func3);

    int argc = 5;
    char* argv1[] = {"argparse", "foo", "bar", "func1", "500"};
    ap.execute_command(argc, argv1);
    EXPECT_EQ(output_buffer.str(), "1000\n");
    output_buffer.str("");
    output_buffer.clear();

    argc = 7;
    char* argv2[] = {"argparse", "foo", "bar", "func2", "10", "12.3", "30.5013"};
    ap.execute_command(argc, argv2);
    EXPECT_EQ(output_buffer.str(), "385.166\n");
    output_buffer.str("");
    output_buffer.clear();

    argc = 2;
    char* argv3[] = {"argparse", "func3"};
    ap.execute_command(argc, argv3);

    EXPECT_EQ(output_buffer.str(), "test\n");
}

TEST_F(ArgParseTests, MissingFunctionTest) {
    ArgParse ap;

    void (*func)(void) = []() {
        std::cout<<"test"<<std::endl;
    };

    ap.add_command<0>({"bar", "baz"}, func);
    
    int argc = 2;
    char* argv[] = {"argparse", "bar"};
    ap.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "command not found\n");
}

TEST_F(ArgParseTests, MissingCommandTest) {
    ArgParse ap;

    void (*func)(void) = []() {
        std::cout<<"test"<<std::endl;
    };

    ap.add_command<0>({"bar", "baz"}, func);
    
    int argc = 4;
    char* argv[] = {"argparse", "foo", "bar", "baz"};
    ap.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "command not found\n");
}

TEST_F(ArgParseTests, InvalidArgsTest) {
    ArgParse ap;

    void (*func)(long double) = [](long double s) {
        std::cout<<s<<std::endl;
    };

    ap.add_command<1, long double>({"test"}, func);
    ap.add_invalid_args_message({"test"}, "updated message");

    int argc = 2;
    char* argv[] = {"argparse", "test", "10"};
    ap.execute_command(argc, argv);
    
    EXPECT_EQ(output_buffer.str(), "updated message\n");
}