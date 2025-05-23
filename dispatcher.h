#ifndef DISPATCHER_H
#define DISPATCHER_H

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

class Dispatcher {
private:
    using InvalidArgsFunc = std::function<void(std::vector<std::string>&, std::vector<bool>&, std::vector<std::string>&, std::vector<std::string>&)>;
    using InvalidCommandFunc = std::function<void(std::vector<std::string>&, std::vector<std::string>&, std::string&)>;

    struct arg_t {
        std::unordered_map<std::string, std::any> flags; ///< Set of flags which specify a specific argument configuration.

        std::any def; ///< Possible default value if no other values are added.

        std::string name; ///< Name of the argument.

        std::type_index type; ///< Type of the argument.

        template<typename T>
        arg_t(T&&) : type(typeid(T)) { }
    };

    struct dispatch_node_t {
        std::function<void(std::vector<std::any>&)> execute; ///< Wrapper function to execute arguments on previously provided function.

        int num_args; ///< The number of arguments for a function.

        std::vector<arg_t> args; ///< Metadata on function arguments.

        std::vector<std::pair<std::vector<std::string>, dispatch_node_t*>> next; ///< List of next nodes with path names and aliases.

        std::string invalid_command_msg = ""; ///< Invalid command message.

        std::string invalid_args_msg = ""; ///< Invalid arguments message.

        std::function<void(std::vector<std::string>&, std::vector<std::string>&, std::string&)> invalid_command_func; ///< Function to run on invalid command

        std::function<void(std::vector<std::string>&, std::vector<bool>&, std::vector<std::string>&, std::vector<std::string>&)> invalid_args_func; ///< Function to run on invalid arguments

        dispatch_node_t* find_next(std::string& name) {
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

        std::vector<std::string> get_next() {
            std::vector<std::string> res;
            for(auto& p : next) {
                res.push_back(p.first[0]);
            }

            return res;
        }

        std::vector<std::string> get_names() {
            std::vector<std::string> res;
            for(arg_t& arg : args) {
                res.push_back(arg.name);
            }

            return res;
        }

        std::pair<int, std::any> find_flag(std::string& flag) {
            for(int i = 0; i < num_args; i++) {
                if(args[i].flags.find(flag) != args[i].flags.end()) {
                    return {i, args[i].flags[flag]};
                }
            }

            return {-1, std::any()};
        }

        void add_default(int idx, std::any& def) {
            args[idx].def = def;
        }

        bool add_alias(std::string& name, std::string& alias) {
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

        void set_invalid_args_message(std::string& msg) {
            invalid_args_msg = msg;
        }

        void set_invalid_command_message(std::string& msg) {
            invalid_command_msg = msg;
        }
    };

    void destructor_helper(dispatch_node_t* cur) {
        for(auto p : cur->next) {
            destructor_helper(p.second);
        }

        delete(cur);
    }

    template<int N, typename ...Args>
    std::tuple<Args...> convert_args(std::vector<std::any>& args) {
        return convert_args_impl<Args...>(args, std::make_index_sequence<N>{});
    }

    template<typename ...Args, std::size_t ...I>
    std::tuple<Args...> convert_args_impl(std::vector<std::any>& args, std::index_sequence<I...> seq) {
        return std::make_tuple(std::any_cast<Args>(args[I])...);
    }

    dispatch_node_t* root;

    std::unordered_map<std::type_index, std::function<std::any(std::string)>> conversions {
        {typeid(int), [](std::string s) { return stoi(s); }},
        {typeid(float), [](std::string s) { return stof(s); }},
        {typeid(double), [](std::string s) { return stod(s); }},
        {typeid(std::string), [](std::string s) { return s; }}
    };

    std::any convert(std::type_index type, std::string s) {
        if(conversions.find(type) != conversions.end()) {
            try {
                return conversions[type](s);
            }
            catch(...) {
                return std::any();
            }
        }
        throw std::logic_error("Missing proper conversion for argument " + s + '.');
    }

    std::string invalid_args_msg;

    std::string invalid_command_msg;

    static std::string path_to_str(std::vector<std::string>& path, std::string delim = " ") {
        std::string path_str = path[0];
        for(int i = 1; i < path.size(); i++) {
            path_str.append(delim + path[i]);
        }

        return path_str;
    }

    static void index_failed(int idx, int num_args) {
        throw std::logic_error("Provided index " + std::to_string(idx) + " too large for " + std::to_string(num_args) + " arguments.");
    }

    dispatch_node_t* traverse_entire(std::vector<std::string>& path) {
        dispatch_node_t* cur = root;
        for(std::string& name : path) {
            cur = cur->find_next(name);

            if(!cur) {
                throw std::logic_error("Failed to find path: " + path_to_str(path));
            }
        }

        return cur;
    }

    std::pair<int, dispatch_node_t*> traverse_until(std::vector<std::string>& path) {
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

    dispatch_node_t* traverse_drill(std::vector<std::string>& path) {
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

    template<int N, typename ...Args>
    void add_command_impl(std::vector<std::string> path, std::function<void(Args...)> func) {
        dispatch_node_t* cur = traverse_drill(path);

        cur->execute = [func, this](std::vector<std::any> args) {
            std::apply(func, convert_args<N, Args...>(args));
        };
        cur->num_args = N;

        cur->args = { arg_t(Args{ })... };
    }

    std::string trim_flag(std::string s) {
        for(int i = 0; i < s.size(); i++) {
            if(s[i] != '-') {
                s = s.substr(i, s.size() - i);
                return s;
            }
        }

        return s;
    }

    static int levenshtein(std::string& s1, std::string& s2) {
        int N = s1.size(), M = s2.size();
        int dp[N + 1][M + 1];

        for(int i = M; i >= 0; i--) {
            dp[N][i] = M - i;
        }
        for(int i = N; i >= 0; i--) {
            dp[i][M] = N - i;
        }

        for(int i = N - 1; i >= 0; i--) {
            for(int j = M - 1; j >= 0; j--) {
                dp[i][j] = 1e9;
                dp[i][j] = std::min(dp[i][j], dp[i + 1][j + 1] + (s1[i] != s2[j])); // substitution
                dp[i][j] = std::min(dp[i][j], 1 + dp[i][j + 1]); // deletion
                dp[i][j] = std::min(dp[i][j], 1 + dp[i + 1][j]); // insertion
            }
        }

        return dp[0][0];
    }

    static std::vector<std::string> find_close(std::vector<std::string>& names, std::string& s, int threshold) {
        std::vector<std::string> res;
        for(std::string& name : names) {
            if(levenshtein(name, s) <= threshold) {
                res.push_back(name);
            }
        }

        return res;
    }

    InvalidCommandFunc invalid_command_func = [](std::vector<std::string>& path, std::vector<std::string>& next, std::string& name) {
        std::vector<std::string> closest = find_close(next, name, 2);

        std::cout << "Unknown command: " << path_to_str(path) << " \033[31m" << name << "\033[30m\n\n";

        if(!closest.empty()) {
            if(closest.size() == 1) {
                std::cout << "The most similar command is:\n";
            }
            else {
                std::cout << "Similar commands are:\n";
            }

            for(std::string& close : closest) {
                std::cout << '\t' << close << '\n';
            }
            std::cout << '\n';
        }
        else {
            std::cout << "Possible commands are:\n";
            for(std::string& name : next) {
                std::cout << '\t' << name << '\n';
            }
            std::cout << '\n';
        }
    };
    
    InvalidArgsFunc invalid_args_func = [](std::vector<std::string>& names, std::vector<bool>& converted, std::vector<std::string>& path, std::vector<std::string>& input) {
        std::cout << "Invalid arguments: " << path_to_str(path);
        bool first = true;
        for(int i = 0; i < converted.size(); i++) {
            if(!converted[i]) {
                std::cout << " \033[31m" << input[i] << "\033[30m";
            }
            else {
                std::cout << ' ' << input[i];
            }
        }
        std::cout << '\n';

        std::cout << "\nExpected: " << path_to_str(path) << ' ';

        for(int i = 0; i < names.size(); i++) {
            if(names[i].empty()) {
                std::cout << "[arg" << i + 1 << ']';
            }
            else {
                std::cout << '[' << names[i] << ']';
            }

            if(i != names.size() - 1) {
                std::cout << ' ';
            }
        }
        std::cout << '\n' << '\n';
    };

public:
    using InvalidArgsFunc = InvalidArgsFunc;
    using InvalidCommandFunc = InvalidCommandFunc;

    Dispatcher() {
        root = new dispatch_node_t();
    }

    ~Dispatcher() {
        destructor_helper(root);
    }

    template<typename ...Args>
    void add_command(const std::vector<std::string> path, void (*func)(Args...)) {
        std::function<void(Args...)> wrapped = func;
        static constexpr std::size_t N = sizeof...(Args);
        add_command_impl<N, Args...>(path, wrapped);
    }

    void execute_command(int argc, const char* argv[]) {
        // get proper dispatch node
        std::vector<std::string> names(argc - 1);
        for(int i = 1; i < argc; i++) {
            names[i - 1] = std::string(argv[i]);
        }

        auto [idx, cur] = traverse_until(names);

        // check if node can execute
        if(!(cur->execute)) {
            if(cur->invalid_command_func) {
                std::vector<std::string> path = std::vector<std::string>(argv, argv + idx + 1);
                std::vector<std::string> next = cur->get_next();

                std::string name = "";
                if(idx < names.size()) {
                    name = names[idx];
                }

                cur->invalid_command_func(path, next, name);
            }
            else if(!cur->invalid_command_msg.empty()) {
                std::cout << cur->invalid_command_msg << '\n';
            }
            else if(!invalid_command_msg.empty()) {
                std::cout << invalid_command_msg << '\n';
            }
            else {
                std::vector<std::string> path = std::vector<std::string>(argv, argv + idx + 1);
                std::vector<std::string> next = cur->get_next();

                std::string name = "";
                if(idx < names.size()) {
                    name = names[idx];
                }

                invalid_command_func(path, next, name);
            }
            return;
        }
        
        names = std::vector<std::string>(names.begin() + idx, names.end());

        std::vector<std::any> args(cur->num_args);
        std::vector<bool> converted(cur->num_args);
        std::vector<bool> flags(args.size());
        std::vector<std::vector<int>> attempted(cur->num_args);
        bool suc = true;

        // find and update flags
        for(int i = 0; i < names.size(); i++) {
            std::string trimmed = trim_flag(names[i]);
            if(trimmed.size() == names[i].size()) continue;

            auto [idx, value] = cur->find_flag(trimmed);

            if(idx == -1) {
                continue;
            }

            attempted[idx].push_back(i);

            flags[i] = true;
            if(!value.has_value()) {
                i++;

                if(i >= names.size()) {
                    continue;
                }

                flags[i] = true;
                
                args[idx] = convert(cur->args[idx].type, names[i]);
                attempted[idx].push_back(i);
            }
            else {
                args[idx] = value;
            }
        }

        // populate regular arguments
        for(int i = 0, idx = 0; i < names.size(); i++) {
            if(flags[i]) continue;

            while(idx < args.size() && !attempted[idx].empty()) {
                idx++;
            }

            args[idx] = convert(cur->args[idx].type, names[i]);
            attempted[idx].push_back(i);
        }

        // fill in default values
        for(int i = 0; i < args.size() && i < cur->args.size(); i++) {
            if(!args[i].has_value() && cur->args[i].def.has_value()) {
                args[i] = cur->args[i].def;
            }
        }
        
        std::vector<bool> successes(names.size(), false);
        for(int i = 0; i < args.size(); i++) {
            if(args[i].has_value()) {
                for(int j : attempted[i]) {
                    successes[j] = true;
                }
            }
            else {
                suc = false;
            }
        }

        // check arguments
        if(!suc) {
            if(cur->invalid_args_func) {
                std::vector<std::string> cur_names = cur->get_names();
                std::vector<std::string> path = std::vector<std::string>(argv, argv + idx + 1);
                cur->invalid_args_func(cur_names, successes, path, names);
            }
            else if(!cur->invalid_args_msg.empty()) {
                std::cout << cur->invalid_args_msg << '\n';
            }
            else if(!invalid_args_msg.empty()) {
                std::cout << invalid_args_msg << '\n';
            }
            else {
                std::vector<std::string> cur_names = cur->get_names();
                std::vector<std::string> path = std::vector<std::string>(argv, argv + idx + 1);
                invalid_args_func(cur_names, successes, path, names);
            }
            return;
        }

        // execute method
        cur->execute(args);
    }

    void execute_command(int argc, char* argv[]) {
        execute_command(argc, const_cast<const char**>(argv));
    }

    template<typename T>
    void add_conversion(std::function<T(std::string)> convert) {
        conversions[typeid(T)] = [convert](std::string s) {
            return std::any(convert(s));
        };
    }

    void add_alias(std::vector<std::string> path, std::string alias) {
        std::string orig = path[path.size() - 1];
        path.pop_back();

        dispatch_node_t* cur = traverse_entire(path);

        if(!cur->add_alias(path[path.size() - 1], alias)) {
            throw std::logic_error("Failed to alias " + alias + "on " + path[path.size() - 1] + '.');
        }
    }

    void add_positional_flag(std::vector<std::string> path, int idx, std::string flag) {
        dispatch_node_t* cur = traverse_entire(path);

        if(idx >= cur->num_args) {
            index_failed(idx, cur->num_args);
        }

        cur->args[idx].flags[flag];
    }

    template<typename T>
    void add_value_flag(std::vector<std::string> path, int idx, std::string flag, T value) {
        dispatch_node_t* cur = traverse_entire(path);
        
        if(idx >= cur->num_args) {
            index_failed(idx, cur->num_args);
        }

        cur->args[idx].flags[flag] = value;
    }

    template<typename T>
    void add_default(std::vector<std::string> path, int idx, T def) {
        dispatch_node_t* cur = traverse_entire(path);

        if(idx >= cur->num_args) {
            index_failed(idx, cur->num_args);
        }

        std::any wrapper(def);
        cur->add_default(idx, wrapper);
    }

    void add_specific_invalid_args_message(std::vector<std::string> path, std::string msg) {
        dispatch_node_t* cur = traverse_entire(path);

        cur->set_invalid_args_message(msg);
    }

    void add_specific_invalid_command_message(std::vector<std::string> path, std::string msg) {
        dispatch_node_t* cur = traverse_entire(path);

        cur->set_invalid_command_message(msg);
    }

    void add_default_invalid_args_message(std::string msg) {
        invalid_args_msg = msg;
    }

    void add_default_invalid_command_message(std::string msg) {
        invalid_command_msg = msg;
    }

    void add_specific_invalid_args_func(std::vector<std::string> path, InvalidArgsFunc func) {
        dispatch_node_t* cur = traverse_entire(path);

        cur->invalid_args_func = func;
    }

    void add_specific_invalid_command_func(std::vector<std::string> path, InvalidCommandFunc func) {
        dispatch_node_t* cur = traverse_entire(path);

        cur->invalid_command_func = func;
    }

    void add_default_invalid_args_func(InvalidArgsFunc func) {
        invalid_args_func = func;
    }

    void add_default_invalid_command_func(InvalidCommandFunc func) {
        invalid_command_func = func;
    }

    void set_arg_name(std::vector<std::string> path, int idx, std::string name) {
        dispatch_node_t* cur = traverse_entire(path);

        if(idx >= cur->num_args) {
            index_failed(idx, cur->num_args);
        }

        cur->args[idx].name = name;
    }
};

#endif