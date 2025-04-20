#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <variant>
#include <functional>
#include <string>
#include <vector>

class ArgParse {
private:
    template <typename T>
    struct argparse_arg_t {
        bool optional; // whether argument has default value
        T def; // default value for 
        std::vector<std::string> flags; // name of flags
    };

    template <typename Ret, typename ...Args>
    struct argparse_node_t {
        std::vector<std::string> names; // name and aliases
        bool end; // whether there is a function ending at this node
        std::function<Ret(Args...)> func; // actual function to store
        std::tuple<argparse_arg_t<Args>...> args; // flags for function arguments
        std::vector<argparse_node_t*> next; // next nodes in tree
    };

    argparse_node_t<void>* root;

public:
    ArgParse();

    ~ArgParse();

    // // add nodes to command tree with all function argument information included
    // template<typename Ret, typename ...Args>
    // void add_command(std::vector<std::string>& names, std::function<Ret(Args...)>* func, std::vector<std::string>& flags, std::vector<bool> optional, std::tuple<Args...> def);

    // add nodes to command tree 
    template<typename Ret, typename ...Args>
    void add_command(std::vector<std::string>& names, std::function<Ret(Args...)>& func);

    // // update information for function argument
    // template <typename T>
    // void add_arg(char* names[], int idx, char* flags, bool optional, T def);

    // // add flags for a function argument
    // void add_flags(char* names[], int idx, char* flags);

    // // add default value for a function argument
    // template <typename T>
    // void add_default(char* names[], int idx, bool optional, T def);
};

#endif