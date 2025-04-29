#ifndef CONVERT_H
#define CONVERT_H

#include <vector>
#include <string>
#include <tuple>

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

template<typename ...Args, std::size_t ...I>
std::tuple<Args...> vector_to_tuple_impl(std::vector<std::string> args, std::index_sequence<I...>) {
    return std::make_tuple(convert<Args>(args[I])...);
}

template<int N, typename ...Args>
std::tuple<Args...> vector_to_tuple(std::vector<std::string> args) {
    return vector_to_tuple_impl<Args...>(args, std::make_index_sequence<N>{});
}

#endif