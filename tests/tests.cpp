#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <sstream>
#include "../dispatcher.h"

class DispatcherTests : public ::testing::Test {
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

TEST_F(DispatcherTests, SingleArgumentTest) {
    Dispatcher d;

    void (*func)(int) = [](int x) {
        std::cout<<x * 2<<std::endl;
    };

    d.add_command({"bar", "baz", "foo"}, func);

    int argc = 5;
    const char* argv[] = {"Dispatcher", "bar", "baz", "foo", "500"};
    d.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "1000\n");
}

TEST_F(DispatcherTests, MultiArgumentTest) {
    void (*func)(int, float, double) = [](int x, float y, double z) {
        std::cout<<x + y * z<<std::endl;
    };

    Dispatcher d;
    d.add_command({"bar", "baz", "foo"}, func);

    int argc = 7;
    const char* argv[] = {"Dispatcher", "bar", "baz", "foo", "10", "12.3", "30.5013"};
    d.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "385.166\n");
}

TEST_F(DispatcherTests, CustomTypeTest) {
    struct test_t {
        int a;
    };

    void (*func)(test_t) = [](test_t t) {
        std::cout<<t.a * 2<<std::endl;
    };

    Dispatcher d;
    d.add_conversion<test_t>([](std::string s) -> test_t {
        return test_t { stoi(s) };
    });
    d.add_command({"test"}, func);

    int argc = 3;
    const char* argv[] = {"Dispatcher", "test", "500"};
    d.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "1000\n");
}

TEST_F(DispatcherTests, AliasTest) {
    Dispatcher d;

    void (*func)(int) = [](int x) {
        std::cout<<x * 2<<std::endl;
    };

    d.add_command({"bar", "baz", "foo"}, func);
    d.add_alias({"bar"}, "b");
    d.add_alias({"bar", "baz"}, "b");
    d.add_alias({"bar", "baz", "foo"}, "f");


    int argc = 5;
    const char* argv[] = {"Dispatcher", "b", "b", "f", "500"};
    d.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "1000\n");
}

TEST_F(DispatcherTests, NoArgumentsTest) {
    Dispatcher d;

    void (*func)(void) = []() {
        std::cout<<"test"<<std::endl;
    };

    d.add_command({"bar", "baz", "foo"}, func);


    int argc = 4;
    const char* argv[] = {"Dispatcher", "bar", "baz", "foo"};
    d.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "test\n");
}

TEST_F(DispatcherTests, MultipleFunctionsTest) {
    Dispatcher d;

    void (*func1)(int) = [](int x) {
        std::cout<<x * 2<<std::endl;
    };
    void (*func2)(int, float, double) = [](int x, float y, double z) {
        std::cout<<x + y * z<<std::endl;
    };
    void (*func3)(void) = []() {
        std::cout<<"test"<<std::endl;
    };

    d.add_command({"foo", "bar", "func1"}, func1);
    d.add_command({"foo", "bar", "func2"}, func2);
    d.add_command({"func3"}, func3);

    int argc = 5;
    const char* argv1[] = {"Dispatcher", "foo", "bar", "func1", "500"};
    d.execute_command(argc, argv1);
    EXPECT_EQ(output_buffer.str(), "1000\n");
    output_buffer.str("");
    output_buffer.clear();

    argc = 7;
    const char* argv2[] = {"Dispatcher", "foo", "bar", "func2", "10", "12.3", "30.5013"};
    d.execute_command(argc, argv2);
    EXPECT_EQ(output_buffer.str(), "385.166\n");
    output_buffer.str("");
    output_buffer.clear();

    argc = 2;
    const char* argv3[] = {"Dispatcher", "func3"};
    d.execute_command(argc, argv3);

    EXPECT_EQ(output_buffer.str(), "test\n");
}

TEST_F(DispatcherTests, MissingFunctionTest) {
    Dispatcher d;

    void (*func)(void) = []() {
        std::cout<<"test"<<std::endl;
    };

    d.add_command({"bar", "baz"}, func);
    
    int argc = 2;
    const char* argv[] = {"Dispatcher", "bar"};
    d.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "Unknown command: Dispatcher bar \"\"\n\nPossible commands are:\n\tbaz\n\n");
}

TEST_F(DispatcherTests, MissingCommandTest) {
    Dispatcher d;

    void (*func)(void) = []() {
        std::cout<<"test"<<std::endl;
    };

    d.add_command({"bar", "baz"}, func);
    
    int argc = 4;
    const char* argv[] = {"Dispatcher", "foo", "bar", "baz"};
    d.execute_command(argc, argv);

    EXPECT_EQ(output_buffer.str(), "Unknown command: Dispatcher \"foo\"\n\nPossible commands are:\n\tbar\n\n");
}

TEST_F(DispatcherTests, InvalidArgsTest) {
    Dispatcher d;

    void (*func)(int) = [](int s) {
        std::cout<<s<<std::endl;
    };

    d.add_command({"test"}, func);
    d.add_target_invalid_args_message({"test"}, "updated message");

    int argc = 3;
    const char* argv[] = {"Dispatcher", "test", "asdlkfjhaslkdjfas"};
    d.execute_command(argc, argv);
    
    EXPECT_EQ(output_buffer.str(), "updated message\n");
}

TEST_F(DispatcherTests, FlagTest) {
    Dispatcher d;

    void (*func)(int, int) = [](int x, int y) {
        std::cout<<x + y<<std::endl;
    };

    d.add_command({"test"}, func);
    d.add_positional_flag({"test"}, 1, "y");

    int argc = 5;
    const char* argv[] = {"Dispatcher", "test", "-y", "20", "10"};
    d.execute_command(argc, argv);
    
    EXPECT_EQ(output_buffer.str(), "30\n");
}

TEST_F(DispatcherTests, DefaultTest) {
    Dispatcher d;

    void (*func)(int, int) = [](int x, int y) {
        std::cout<<x + y<<std::endl;
    };

    d.add_command({"test"}, func);
    d.add_default({"test"}, 1, 300);

    int argc = 3;
    const char* argv[] = {"Dispatcher", "test", "10"};
    d.execute_command(argc, argv);
    
    EXPECT_EQ(output_buffer.str(), "310\n");
}

TEST_F(DispatcherTests, DefaultFlagTest) {
    Dispatcher d;

    void (*func)(int, int) = [](int x, int y) {
        std::cout<<x + y<<std::endl;
    };

    d.add_command({"test"}, func);
    d.add_positional_flag({"test"}, 1, "y");
    d.add_default({"test"}, 0, 20);

    int argc = 4;
    const char* argv[] = {"Dispatcher", "test", "-y", "10"};
    d.execute_command(argc, argv);
    
    EXPECT_EQ(output_buffer.str(), "30\n");
}

TEST_F(DispatcherTests, ValueFlagTest) {
    Dispatcher d;

    void (*func)(int, int) = [](int x, int y) {
        std::cout<<x + y<<std::endl;
    };

    d.add_command({"test"}, func);
    d.add_value_flag({"test"}, 1, "y", 500);

    int argc = 4;
    const char* argv[] = {"Dispatcher", "test", "-y", "10"};
    d.execute_command(argc, argv);
    
    EXPECT_EQ(output_buffer.str(), "510\n");
}

TEST_F(DispatcherTests, CustomInvalidCommandFuncTest) {
    Dispatcher d;

    void (*func)(int, int) = [](int x, int y) {
        std::cout<<x + y<<std::endl;
    };

    d.add_command({"test"}, func);
    d.add_target_invalid_command_func({}, [](std::vector<std::string>&, std::vector<std::string>&, std::string&) -> void {
        std::cout << "custom\n";
    });

    int argc = 1;
    const char* argv[] = {"Dispatcher"};
    d.execute_command(argc, argv);
    
    EXPECT_EQ(output_buffer.str(), "custom\n");
}