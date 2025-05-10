#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <any>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

class ArgParse {
private:
    struct argparse_node_t {
        // executes arguments on previously provided function
        std::function<void(std::vector<std::string>)> execute;

        // check if arguments can be casted to the correct types
        std::function<bool(std::vector<std::string>)> check;

        // number of arguments in function
        int num_args;

        // list of next nodes
        std::vector<std::pair<std::vector<std::string>, argparse_node_t*>> next;

        // node ends but not a function
        std::string invalid_command = "command not found";

        // invalid arguments to node's function
        std::string invalid_args = "invalid arguments";

        // searches next nodes for a given name
        argparse_node_t* find(std::string name) {
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

        // adds alias to next vector in node
        bool alias(std::string name, std::string alias) {
            for(int i = 0; i < next.size(); i++) {
                std::vector<std::string>& names = next[i].first;
    
                for(std::string& s : names) {
                    if(s == name) {
                        names.push_back(alias);
                        return true;
                    }
                }
            }

            return false;
        }

        void set_invalid_args_message(std::string msg) {
            invalid_args = msg;
        }

        void set_invalid_command_message(std::string msg) {
            invalid_command = msg;
        }
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

    // traverses path and returns node found
    argparse_node_t* traverse_entire(std::vector<std::string> path) {
        argparse_node_t* cur = root;
        for(std::string& name : path) {
            cur = cur->find(name);

            if(!cur) {
                return cur;
            }
        }

        return cur;
    }

    // traverses and returns before nullptr
    std::pair<int, argparse_node_t*> traverse_until(std::vector<std::string> path) {
        argparse_node_t* cur = root;
        int idx;
        
        for(idx = 0; idx < path.size(); idx++) {
            argparse_node_t* next = cur->find(path[idx]);

            if(!next) {
                return {idx, cur};
            }

            cur = next;
        }

        return {idx, cur};
    }

    // drills node into tree
    argparse_node_t* traverse_drill(std::vector<std::string> path) {
        argparse_node_t* cur = root;
        for(int i = 0; i < path.size(); i++) {
            argparse_node_t* next = cur->find(path[i]);

            if(!next) {
                next = new argparse_node_t();
                cur->next.push_back({{path[i]}, next});
            }

            cur = next;
        }

        return cur;
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
        argparse_node_t* cur = traverse_drill(path);

        cur->check = [this](std::vector<std::string> args) {
            if(args.size() != N) {
                return false;
            }

            try {
                vector_to_tuple<N, Args...>(args);
                return true;
            }
            catch(...) {
                return false;
            }
        };

        cur->execute = [func, this](std::vector<std::string> args) {
            std::apply(func, vector_to_tuple<N, Args...>(args));
        };
        cur->num_args = N;
    }

    // add nodes to command tree
    template<typename ...Args>
    void add_command(const std::vector<std::string> path, void (*func)(Args...)) {
        std::function<void(Args...)> wrapped = func;
        static constexpr std::size_t N = sizeof...(Args);
        add_command_impl<N, Args...>(path, wrapped);
    }

    // executes command based on path and arguments
    void execute_command(const std::vector<std::string> path, const std::vector<std::string> args) {
        argparse_node_t* cur = traverse_entire(path);

        if(!cur) {
            std::cout<<"command not found"<<std::endl;
            return;
        }

        if(!(cur->execute)) {
            std::cout<<cur->invalid_command<<std::endl;
            return;
        }

        if(!cur->check(args)) {
            std::cout<<cur->invalid_args<<std::endl;
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

        auto [idx, cur] = traverse_until(args);

        if(!(cur->execute)) {
            std::cout<<cur->invalid_command<<std::endl;
            return;
        }

        args = std::vector<std::string>(args.begin() + idx, args.end());
        if(!cur->check(args)) {
            std::cout<<cur->invalid_args<<std::endl;
            return;
        }

        cur->execute(args);
    }

    // adds a new conversion from string to any type
    template<typename T>
    void add_conversion(std::function<T(std::string)> convert) {
        conversions[typeid(T)] = [convert](std::string s) {
            return std::any(convert(s));
        };
    }

    // add an alias for the last value in a path
    void add_alias(std::vector<std::string> path, std::string alias) {
        argparse_node_t* cur = traverse_entire(std::vector<std::string>(path.begin(), path.end() - 1));
 
        if(!cur) {
            std::cout<<"path not found"<<std::endl;
            return;
        }

        if(!cur->alias(path[path.size() - 1], alias)) {
            std::cout<<"path not found"<<std::endl;
        }        
    }

    // add a message for invalid arguments on a node
    void add_invalid_args_message(std::vector<std::string> path, std::string msg) {
        argparse_node_t* cur = traverse_entire(path);

        if(!cur) {
            std::cout<<"path not found"<<std::endl;
        }

        cur->set_invalid_args_message(msg);
    }

    // add a message for if a node lacks a command
    void add_invalid_command_message(std::vector<std::string> path, std::string msg) {
        argparse_node_t* cur = traverse_entire(path);

        if(!cur) {
            std::cout<<"path not found"<<std::endl;
        }

        cur->set_invalid_command_message(msg);
    }
};

#endif