#include "argparse.h"
#include <string>

ArgParse::ArgParse() {
    root = new argparse_node_t<void>();
    root->end = false;
}

ArgParse::~ArgParse() {

}

template<typename Ret, typename ...Args>
void ArgParse::add_command(std::vector<std::string>& names, std::function<Ret(Args...)>& func) {
    
}