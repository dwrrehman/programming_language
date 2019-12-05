#ifndef arguments_hpp
#define arguments_hpp

#include <string>
#include <vector>

struct file {
    std::string name = "";
    std::string text = "";
    bool is_main = false;
};

struct arguments {    
    std::vector<file> files = {};
    std::string executable_name = "a.out";
    bool use_repl = false;
    bool empty = false;
    bool interpret = false;    
    bool error = false;
};

arguments get_commandline_arguments(const int argc, const char** argv);

#endif /* arguments_hpp */
