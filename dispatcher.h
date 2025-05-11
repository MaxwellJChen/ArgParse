#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <any>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


/**
 * @class Dispatcher
 * @brief Handles CLI routing logic under-the-hood.
 */
class Dispatcher {
private:

    /**
     * @struct arg_t
     * @brief Encodes information about a function argument.
     */
    struct arg_t {
        std::unordered_map<std::string, std::string> flags; ///< Set of flags which specify a specific argument configuration.

        std::string def; ///< Possible default value if no other values are added.
    };

    /**
     * @struct dispatch_node_t
     * @brief All the information for a specific position in a path, including a method to execute, error messages, flags, etc.
     */
    struct dispatch_node_t {
        std::function<void(std::vector<std::string>)> execute; ///< Wrapper function to execute arguments on previously provided function.

        std::function<bool(std::vector<std::string>)> check; ///< Checks if arguments can be converted to the correct types.

        int num_args; ///< The number of arguments for a function.

        std::vector<arg_t> args; ///< Metadata on function arguments.

        std::vector<std::pair<std::vector<std::string>, dispatch_node_t*>> next; ///< List of next nodes with path names and aliases.

        std::string invalid_command = "command not found"; ///< Invalid command message.

        std::string invalid_args = "invalid arguments"; ///< Invalid arguments message.

        /**
         * @brief Searches next nodes for a given name.
         * 
         * @param name The path value indicating the next node.
         * @return A pointer to the next node or nullptr if no next node was found.
         */
        dispatch_node_t* find_next(std::string name) {
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

        /**
         * @brief Searches argument flags for a provided flag.
         * 
         * @param flag The flag to search for.
         * @return The index of the argument with the flag, and a default value if available. 
         *         Returns {-1, ""} if no flag was found.
         */
        std::pair<int, std::string> find_flag(std::string flag) {
            for(int i = 0; i < num_args; i++) {
                if(args[i].flags.find(flag) != args[i].flags.end()) {
                    return {i, args[i].flags[flag]};
                }
            }

            return {-1, ""};
        }

        /**
         * @brief Adds a default value for a certain argument.
         * 
         * @param idx The index of the argument to add the new default value for.
         * @param def The string version of the default value to add.
         */
        void add_default(int idx, std::string def) {
            args[idx].def = def;
        }

        /**
         * @brief Adds an alias to for one of the next nodes.
         * 
         * @param name Indicates the next node to alias.
         * @param alias The alias to add to the next value.
         * @return Whether the alias was successfully added.
         */
        bool add_alias(std::string name, std::string alias) {
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

        /**
         * @brief Sets a custom error message when invalid arguments are encountered.
         * 
         * @param msg The message to replace the current invalid arguments message with.
         */
        void set_invalid_args_message(std::string msg) {
            invalid_args = msg;
        }

        /**
         * @brief Sets a custom error message when this node does not have a command.
         * 
         * @param msg The message to replace the current invalid command message with.
         */
        void set_invalid_command_message(std::string msg) {
            invalid_command = msg;
        }
    };

    /**
     * @brief Helper function for recursive destructor.
     * 
     * @param cur The current dispatch node to delete.
     */
    void destructor_helper(dispatch_node_t* cur) {
        for(auto p : cur->next) {
            destructor_helper(p.second);
        }

        delete(cur);
    }

    /**
     * @brief Converts a given string to another type based on stored type conversions.
     * 
     * @tparam T The type to convert the provided string into.
     * @param s The string to convert.
     * @return The converted value.
     */
    template<typename T>
    T convert(std::string s) {
        return std::any_cast<T>(conversions[typeid(T)](s));
    }

    /**
     * @brief Helper method to convert vector of arguments into tuple of desired type.
     * 
     * @tparam Args Parameter pack of types to cast each argument into.
     * @tparam I Index sequence of positions.
     * @param args Vector of strings to convert.
     * @param seq Index sequence used to specify previous index parameter pack.
     * @return The final tuple after conversions are completed.
     */
    template<typename ...Args, std::size_t ...I>
    std::tuple<Args...> vector_to_tuple_impl(std::vector<std::string> args, std::index_sequence<I...> seq) {
        return std::make_tuple(convert<Args>(args[I])...);
    }

    /**
     * @brief Wrapper method to convert vector of arguments into tuple of desired type.
     * 
     * @tparam N Number of strings in the vector.
     * @tparam Args Types of arguments to convert into.
     * @param args Vector of strings to convert.
     * @return The final tuple after conversions are completed.
     */
    template<int N, typename ...Args>
    std::tuple<Args...> vector_to_tuple(std::vector<std::string> args) {
        return vector_to_tuple_impl<Args...>(args, std::make_index_sequence<N>{});
    }

    dispatch_node_t* root; ///< The root of the command tree.

    /**
     * @brief Stores conversions from a string to a specific type.
     */
    std::unordered_map<std::type_index, std::function<std::any(std::string)>> conversions {
        {typeid(int), [](std::string s) { return stoi(s); }},
        {typeid(float), [](std::string s) { return stof(s); }},
        {typeid(double), [](std::string s) { return stod(s); }},
        {typeid(std::string), [](std::string s) { return s; }}
    };

    /**
     * @brief Traverses the entire path.
     * 
     * @param path The path to traverse.
     * @return The pointer found or a nullptr.
     */
    dispatch_node_t* traverse_entire(std::vector<std::string> path) {
        dispatch_node_t* cur = root;
        for(std::string& name : path) {
            cur = cur->find_next(name);

            if(!cur) {
                return nullptr;
            }
        }

        return cur;
    }

    /**
     * @brief Traverses the path and returns the node corresponding to the last valid name in the path.
     * 
     * @param path The path to traverse.
     * @return The index of the last valid name in the path and node corresponding to it.
     */
    std::pair<int, dispatch_node_t*> traverse_until(std::vector<std::string> path) {
        dispatch_node_t* cur = root;
        int idx;
        
        for(idx = 0; idx < path.size(); idx++) {
            if(path[idx][0] == '-') {
                return {idx, cur};
            }

            dispatch_node_t* next = cur->find_next(path[idx]);

            if(!next) {
                return {idx, cur};
            }

            cur = next;
        }

        return {idx, cur};
    }

    /**
     * @brief Adds a new path to the command tree.
     * 
     * @param path The new path to add to the tree.
     * @return The node corresponding to the last name in the provided path.
     */
    dispatch_node_t* traverse_drill(std::vector<std::string> path) {
        dispatch_node_t* cur = root;
        for(int i = 0; i < path.size(); i++) {
            dispatch_node_t* next = cur->find_next(path[i]);

            if(!next) {
                next = new dispatch_node_t();
                cur->next.push_back({{path[i]}, next});
            }

            cur = next;
        }

        return cur;
    }

    /**
     * @brief Implementation for creating a new node in the command tree.
     * 
     * @tparam N The number of arguments in the function to add.
     * @tparam Args The types of the arguments in the added function.
     * @param path The path to add the new function to.
     * @param func The function to add at the path.
     */
    template<int N, typename ...Args>
    void add_command_impl(std::vector<std::string> path, std::function<void(Args...)> func) {
        dispatch_node_t* cur = traverse_drill(path);

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

        cur->args.resize(N);
    }

public:
    /**
     * @brief Constructor for Dispatcher class.
     */
    Dispatcher() {
        root = new dispatch_node_t();
    }

    /**
     * @brief Destructor for dispatcher class.
     */
    ~Dispatcher() {
        destructor_helper(root);
    }

    /**
     * @brief Adds a function to the command tree.
     * 
     * @tparam Args Parameter pack of types of input function arguments.
     * @param path The path to add the new function to.
     * @param func The function to add at the end of the path.
     */
    template<typename ...Args>
    void add_command(const std::vector<std::string> path, void (*func)(Args...)) {
        std::function<void(Args...)> wrapped = func;
        static constexpr std::size_t N = sizeof...(Args);
        add_command_impl<N, Args...>(path, wrapped);
    }

    /**
     * @brief Executes a command based on typical command line input.
     * 
     * @param argc The number of provided strings.
     * @param argv The array of provided strings.
     */
    void execute_command(int argc, const char* argv[]) {
        std::vector<std::string> names(argc - 1);
        for(int i = 1; i < argc; i++) {
            names[i - 1] = std::string(argv[i]);
        }

        auto [idx, cur] = traverse_until(names);

        if(!(cur->execute)) {
            std::cout << cur->invalid_command << std::endl;
            return;
        }
        
        names = std::vector<std::string>(names.begin() + idx, names.end());


        std::vector<std::string> args(cur->num_args, "");

        std::vector<bool> flags(args.size());

        // find and update flags
        for(int i = 0; i < names.size(); i++) {
            int pref = 0;
            bool is_flag = false;

            std::string name = names[i];
            while(pref < name.size() && name[pref] == '-') {
                pref++;
                is_flag = true;
            }

            if(!is_flag) continue;

            name = name.substr(pref, name.size() - pref);

            auto [idx, value] = cur->find_flag(name);

            if(idx == -1) {
                std::cout << "flag not found" << std::endl;
                return;
            }

            flags[i] = true;
            if(value.empty()) {
                i++;
                flags[i] = true;
                args[idx] = names[i];
            }
            else {
                args[idx] = value;
            }
        }

        // populate regular arguments
        for(int i = 0, idx = 0; i < names.size(); i++) {
            if(flags[i]) continue;

            while(idx < args.size() && !args[idx].empty()) {
                idx++;
            }

            args[idx] = names[i];
        }

        // fill in any defaults
        for(int i = 0; i < args.size() && i < cur->args.size(); i++) {
            if(args[i].empty()) {
                args[i] = cur->args[i].def;
            }
        }

        if(!cur->check(args)) {
            std::cout << cur->invalid_args << std::endl;
            return;
        }

        cur->execute(args);
    }

    /**
     * @brief Executes a command based on typical command line input.
     * 
     * @param argc The number of provided strings.
     * @param argv The array of provided strings.
     */
    void execute_command(int argc, char* argv[]) {
        execute_command(argc, const_cast<const char**>(argv));
    }

    /**
     * @brief Adds a new conversion from string to any type.
     * 
     * @tparam T The type to convert the string into.
     * @param convert Method specifying how the conversion should be done.
     */
    template<typename T>
    void add_conversion(std::function<T(std::string)> convert) {
        conversions[typeid(T)] = [convert](std::string s) {
            return std::any(convert(s));
        };
    }

    /**
     * @brief Adds an alias for the last name in the provided path.
     * 
     * @param path The path of strings to traverse down.
     * @param alias The alias to add to the final node in the path.
     */
    void add_alias(std::vector<std::string> path, std::string alias) {
        dispatch_node_t* cur = traverse_entire(std::vector<std::string>(path.begin(), path.end() - 1));
 
        if(!cur) {
            std::cout << "path not found" << std::endl;
            return;
        }

        if(!cur->add_alias(path[path.size() - 1], alias)) {
            std::cout << "path not found" << std::endl;
        }        
    }

    /**
     * @brief Add a flag based on 0-indexed arguments for a command.
     * 
     * @param path The path specifying the node to update.
     * @param idx The index of the argument to update with the flag.
     * @param flag The value of the flag to add.
     */
    void add_positional_flag(std::vector<std::string> path, int idx, std::string flag) {
        dispatch_node_t* cur = traverse_entire(path);

        if(!cur) {
            std::cout << "path not found" << std::endl;
            return;
        }

        if(idx >= cur->num_args) {
            std::cout << "index too large" << std::endl;
            return;
        }

        cur->args[idx].flags[flag] = "";
    }

    /**
     * @brief Add a flag based on 0-indexed arguments for a command
     * 
     * @param path The path specifying the node to update.
     * @param idx The index of the argument to update with the flag.
     * @param flag The value of the flag to add.
     * @param value The value the flag corresponds to.
     */
    void add_value_flag(std::vector<std::string> path, int idx, std::string flag, std::string value) {
        dispatch_node_t* cur = traverse_entire(path);

        if(!cur) {
            std::cout << "path not found" << std::endl;
            return;
        }

        if(idx >= cur->num_args) {
            std::cout << "index too large" << std::endl;
            return;
        }

        cur->args[idx].flags[flag] = value;
    }

    /**
     * @brief Adds a default value for a specific argument.
     * 
     * @param path The path specifying the node to update.
     * @param idx The index of the argument to update.
     * @param def The new default value to update the argument with.
     */
    void add_default(std::vector<std::string> path, int idx, std::string def) {
        dispatch_node_t* cur = traverse_entire(path);

        if(!cur) {
            std::cout << "path not found" << std::endl;
            return;
        }

        if(idx >= cur->num_args) {
            std::cout << "index too large" << std::endl;
            return;
        }

        cur->add_default(idx, def);
    }

    /**
     * @brief Adds a message for invalid arguments on a node.
     * 
     * @param path The path to add the invalid arguments message to.
     * @param msg The message to add.
     */
    void add_invalid_args_message(std::vector<std::string> path, std::string msg) {
        dispatch_node_t* cur = traverse_entire(path);

        if(!cur) {
            std::cout << "path not found" << std::endl;
        }

        cur->set_invalid_args_message(msg);
    }

    /**
     * @brief Adds a message for if a node lacks a command.
     * 
     * @param path The path to add the invalid arguments message to.
     * @param msg The message to add.
     */
    void add_invalid_command_message(std::vector<std::string> path, std::string msg) {
        dispatch_node_t* cur = traverse_entire(path);

        if(!cur) {
            std::cout << "path not found" << std::endl;
        }

        cur->set_invalid_command_message(msg);
    }
};

#endif