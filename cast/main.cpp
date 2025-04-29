#include <iostream>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <any>

std::unordered_map<std::type_index, std::function<std::any(std::string)>> mp;

template<typename T>
void reg(std::function<T(std::string)> convert) {
    mp[typeid(T)] = [convert](std::string s) {
        return std::any(convert(s));
    };
};

template<typename T>
std::any convert(std::string s) {
    return mp[typeid(T)](s);
}

int main() {
    reg<int>([](std::string s) {
        return std::stoi(s);
    });

    std::cout<<std::any_cast<int>(convert<int>("20"))<<std::endl;
}