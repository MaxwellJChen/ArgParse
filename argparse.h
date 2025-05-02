#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <functional>
#include <string>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <any>
#include <utility>

class ArgParse {
private:
    struct argparse_node_t {
        // executes arguments on previously provided function
        std::function<void(std::vector<std::string>)> execute;

        // list of next nodes
        std::vector<std::pair<std::vector<std::string>, argparse_node_t*>> next;
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

    // convert vector of string to desired tuple of correct type
    template<int N, typename ...Args>
    std::tuple<Args...> vector_to_tuple(std::vector<std::string> args) {
        return vector_to_tuple_impl<Args...>(args, std::make_index_sequence<N>{});
    }

    // root node of command tree
    argparse_node_t* root;

    // map of functions to convert string to desired type
    std::unordered_map<std::type_index, std::function<std::any(std::string)>> conversions {
        {typeid(int), [](std::string s) { return stoi(s); }},
        {typeid(float), [](std::string s) { return stof(s); }},
        {typeid(double), [](std::string s) { return stod(s); }},
        {typeid(std::string), [](std::string s) { return s; }}
    };

    // searches next nodes for a given name
    argparse_node_t* find_next(std::string name, argparse_node_t* cur) {
        auto &next = cur->next;
        for(int i = 0; i < next.size(); i++) {
            std::vector<std::string>& names = next[i].first;

            for(std::string& s : names) {
                if(s == name) {
                    return next[i].second;
                }
            }
        }

        return nullptr;
    }

public:
    ArgParse() {
        root = new argparse_node_t();
    }

    ~ArgParse() {
        destructor_helper(root);
    }

    template<int N, typename ...Args>
    void add_command_impl(std::vector<std::string> path, std::function<void(Args...)> func) {
        argparse_node_t* cur = root;
        for(int i = 0; i < path.size(); i++) {
            argparse_node_t* next = find_next(path[i], cur);

            if(!next) {
                next = new argparse_node_t();
                cur->next.push_back({{path[i]}, next});
            }

            cur = next;
        }

        cur->execute = [func, this](std::vector<std::string> args) {
            std::apply(func, vector_to_tuple<N, Args...>(args));
        };
    }

    // add nodes to command tree
    template<int N, typename ...Args>
    void add_command(const std::vector<std::string> path, void (*func)(Args...)) {
        std::function<void(Args...)> wrapped = func;
        add_command_impl<N, Args...>(path, wrapped);
    }

    // executes command based on path and arguments
    void execute_command(const std::vector<std::string> path, const std::vector<std::string> args) {
        argparse_node_t* cur = root;
        for(int i = 0; i < path.size(); i++) {
            cur = find_next(path[i], cur);

            if(!cur) {
                std::cout<<"command not found"<<std::endl;
                return;
            }
        }

        if(!(cur->execute)) {
            std::cout<<"function not found"<<std::endl;
            return;
        }

        cur->execute(args);
    }

    // executes command based on command line input
    void execute_command(int argc, char* argv[]) {
        std::vector<std::string> args(argc - 1);
        for(int i = 1; i < argc; i++) {
            args[i - 1] = std::string(argv[i]);
        }

        argparse_node_t* cur = root;
        int idx = 0;
        while(idx < args.size()) {
            argparse_node_t* next = find_next(args[idx], cur);

            if(!next) {
                break;
            }

            idx++;
            cur = next;
        }

        if(!(cur->execute)) {
            std::cout<<"function not found"<<std::endl;
            return;
        }

        cur->execute(std::vector<std::string>(args.begin() + idx, args.end()));
    }

    template<typename T>
    void add_conversion(std::function<T(std::string)> convert) {
        conversions[typeid(T)] = [convert](std::string s) {
            return std::any(convert(s));
        };
    }

    void add_alias(std::vector<std::string> path, std::string alias) {
        argparse_node_t* cur = root;
        for(int i = 0; i < path.size() - 1; i++) {
            cur = find_next(path[i], cur);

            if(!cur) {
                std::cout<<"path not found"<<std::endl;
                return;
            }
        }

        int idx = path.size() - 1;
        auto &next = cur->next;
        for(int i = 0; i < next.size(); i++) {
            std::vector<std::string>& names = next[i].first;

            for(std::string& s : names) {
                if(s == path[idx]) {
                    names.push_back(alias);
                    return;
                }
            }
        }

        std::cout<<"path not found"<<std::endl;
    }
};

#endif