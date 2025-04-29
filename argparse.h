#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <functional>
#include <string>
#include <tuple>
#include <iostream>
#include <unordered_map>

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

class ArgParse {
private:
    struct argparse_node_t {
        std::function<void(std::vector<std::string>)> execute;
        std::unordered_map<std::string, argparse_node_t*> next;
    };

    argparse_node_t* root;

public:
    ArgParse() {
        root = new argparse_node_t();
    }

    ~ArgParse() = default;

    template<int N, typename ...Args>
    void add_command_impl(std::vector<std::string> path, std::function<void(Args...)> func) {
        argparse_node_t* cur = root;
        for(int i = 0; i < path.size(); i++) {
            if(cur->next.find(path[i]) == cur->next.end()) {
                cur->next[path[i]] = new argparse_node_t();
            }
            cur = cur->next[path[i]];
        }

        cur->execute = [=](std::vector<std::string> args) {
            std::apply(func, vector_to_tuple<N, Args...>(args));
        };
    }

    // add nodes to command tree
    template<int N, typename ...Args>
    void add_command(std::vector<std::string> path, void (*func)(Args...)) {
        std::function<void(Args...)> wrapped = func;
        add_command_impl<N, Args...>(path, wrapped);
    }

    // executes command based on path and arguments
    void execute_command(std::vector<std::string> path, std::vector<std::string> args) {
        argparse_node_t* cur = root;
        for(int i = 0; i < path.size(); i++) {
            cur = cur->next[path[i]];
        }

        if(!(cur->execute)) {
            std::cout<<"function does not exist"<<std::endl;
            return;
        }

        cur->execute(args);
    }
};

#endif