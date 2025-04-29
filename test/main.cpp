#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <functional>
#include <tuple>
using namespace std;

template<typename T>
T convert(std::string s) {
    return (T) s;
}

template<>
int convert<int>(std::string s) {
    return std::stoi(s);
}

template<>
double convert<double>(std::string s) {
    return std::stod(s);
}

template<>
float convert<float>(std::string s) {
    return std::stof(s);
}

void foo(int i) {
    std::cout<<i<<std::endl;
}

void bar(double d) {
    std::cout<<d<<std::endl;
}

void baz(float f) {
    std::cout<<f<<std::endl;
}

struct node_t {
    std::function<void(std::string)> execute;
    node_t* next;
};

node_t* root;

template<typename T>
void add(std::function<void(T)> func) {
    std::function<void(std::string)> test = [&](std::string s) {
        func(convert<T>(s));
    };

    root->execute = test;
    root->next = new node_t();
}

void test(int x, float y, double z) {
    std::cout<<x * 2 + y - 3 * z<<std::endl;
}

template<typename ...Args, std::size_t ...I>
std::tuple<Args...> vector_to_tuple_impl(std::vector<std::string> args, std::index_sequence<I...>) {
    return std::make_tuple(convert<Args>(args[I])...);
}

template<int N, typename ...Args>
std::tuple<Args...> vector_to_tuple(std::vector<std::string> args) {
    return vector_to_tuple_impl<Args...>(args, std::make_index_sequence<N>{});
}

int main() {
    root = new node_t();
    add<int>(foo);
    root->execute("10");
    root = root->next;

    add<double>(bar);
    root->execute("10.20");
    root = root->next;

    add<float>(baz);
    root->execute("10.1231");
    root = root->next;

    // std::tuple<int, float, double> args {10, 12., 5.2};
    // std::apply(test, args);



    std::vector<std::string> args {"10", "12.", "5.2"};
    std::apply(test, vector_to_tuple<3, int, float, double>(args));
}