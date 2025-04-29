#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <functional>
#include <string>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <any>

class ArgParse {
private:
    struct argparse_node_t {
        // executes arguments on previously provided function
        std::function<void(std::vector<std::string>)> execute;

        // list of next nodes
        std::unordered_map<std::string, argparse_node_t*> next;
    };

    // helper function for recursive destructor
    void destructor_helper(argparse_node_t* cur) {
        for(auto p : cur->next) {
            destructor_helper(p.second);
        }

        delete(cur);
    }

    template<typename T>
    std::any convert(std::string s) {
        return conversions[typeid(T)](s);
    }

    template<typename ...Args, std::size_t ...I>
    std::tuple<Args...> vector_to_tuple_impl(std::vector<std::string> args, std::index_sequence<I...>) {
        return std::make_tuple(std::any_cast<Args>(convert<Args>(args[I]))...);
    }

    template<int N, typename ...Args>
    std::tuple<Args...> vector_to_tuple(std::vector<std::string> args) {
        return vector_to_tuple_impl<Args...>(args, std::make_index_sequence<N>{});
    }

    // root node of command tree
    argparse_node_t* root;

    // map of functions to convert string to desired type
    std::unordered_map<std::type_index, std::function<std::any(std::string)>> conversions;

public:
    ArgParse() {
        root = new argparse_node_t();

        // add default conversions
        add_conversion<int>([](std::string s) { return stoi(s); });
        add_conversion<float>([](std::string s) { return stof(s); });
        add_conversion<double>([](std::string s) { return stod(s); });
        add_conversion<std::string>([](std::string s) { return s; });
    }

    ~ArgParse() {
        destructor_helper(root);
    }

    template<int N, typename ...Args>
    void add_command_impl(std::vector<std::string> path, std::function<void(Args...)> func) {
        argparse_node_t* cur = root;
        for(int i = 0; i < path.size(); i++) {
            if(cur->next.find(path[i]) == cur->next.end()) {
                cur->next[path[i]] = new argparse_node_t();
            }
            cur = cur->next[path[i]];
        }

        cur->execute = [func, this](std::vector<std::string> args) {
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
            if(cur->next.find(path[i]) == cur->next.end()) {
                std::cout<<"path does not exist"<<std::endl;
                return;
            }

            cur = cur->next[path[i]];
        }

        if(!(cur->execute)) {
            std::cout<<"function does not exist"<<std::endl;
            return;
        }

        cur->execute(args);
    }

    template<typename T>
    void add_conversion(std::function<T(std::string)> convert) {
        conversions[typeid(T)] = [convert](std::string s) {
            return std::any(convert(s));
        };
    }
};

#endif