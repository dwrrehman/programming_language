//
//  main.cpp
//  sandbox6
//
//  Created by Daniel Rehman on 1902166.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>

#include "lists.hpp"


// parameters:

const std::string input_grammar_filepath = "/Users/deniylreimn/Documents/code/cpp/sandboxes/sandbox6/sandbox6/grammar.txt";
const std::string output_cpp_filepath = "/Users/deniylreimn/Documents/code/cpp/sandboxes/sandbox6/sandbox6/parser.cpp";
const std::string input_header_filepath = "/Users/deniylreimn/Documents/code/cpp/sandboxes/sandbox6/sandbox6/parser_header.cpp";


// data structures:

struct element {
    std::string value = "";
    bool is_keyword = false;
    bool is_operator = false;
};

struct rule {
    std::vector<struct element> elements = {};
};

struct node {
    std::string name = "";
    std::vector<struct rule> rules = {};
    bool optional = false;
};

std::vector<struct node> nodes = {};
int current_node = -1;
int current_rule = -1;


// debug:

void print_all_nodes() {
    std::cout << "printing all nodes: " << std::endl;
    for (auto n : nodes) {
        std::cout << "{\n\tNAME = " << n.name << std::endl;
        std::cout << "\trules: " << std::endl;
        int i = 0;
        for (auto r : n.rules) {
            std::cout << "\t\t[" << i << "]: ";
            for (auto e : r.elements) {
                if (e.is_keyword)
                    std::cout << "KW:\"" << e.value <<"\" ";
                else if (e.is_operator)
                    std::cout << "OP:\"" << e.value <<"\" ";
                else
                    std::cout << e.value << " ";
            }
            std::cout << "\n";
            i++;
        }
        std::cout << "\toptional? " << (n.optional ? "yes":"no") << std::endl;
        std::cout << "}\n\n";
    }
}


// helpers:

static void add_element(const std::string &token) {
    nodes[current_node].rules[current_rule].elements.push_back({
        token,
        std::find(keywords.begin(), keywords.end(), token) != keywords.end(),
        std::find(operators.begin(), operators.end(), token) != operators.end()
    });
}

static bool add_rule(size_t element_count, const std::string &token) {
    if (element_count == 1 && (token == "=" || token == "|")) {
        nodes[current_node].rules.push_back({});
        current_rule++;
        return true;
    }
    return false;
}

static bool add_node(bool &inside_node, const std::string &token) {
    if (!inside_node) {
        inside_node = true;
        nodes.push_back({token, {}, false});
        current_node++;
        current_rule = -1;
        return true;
    }
    return false;
}


static void find_optionals() {
    for (auto& n : nodes) {
        for (auto r : n.rules) {
            if (r.elements.size() == 1 && r.elements[0].value == "E") {
                n.optional = true;
                continue;
            }
        }
    }
}

static void process_line(bool &inside_node, const std::string &line, std::string &token) {
    std::istringstream s {line};
    size_t element_count = 0;
    
    while (s.good()) {
        s >> token;
        element_count++;
        if (!add_node(inside_node, token)
            && !add_rule(element_count, token))
            add_element(token);
    }
}

static void process_nodes(std::ifstream &grammar, bool &inside_node, std::string &line, std::string &token) {
    while (std::getline(grammar, line)) {
        if (line != "" && line[0] == '-') continue;
        else if (line != "") process_line(inside_node, line, token);
        else if (inside_node) inside_node = false;
    }
}

static void generate(std::ofstream &file) {
    
    file << "\n\n//this is a automatically generated parser in cpp, for my language.\n\n\n";
    for (auto n : nodes) {
        file << "bool " << n.name << "(params);\n\n";
    }
    file << "\n\n\n//EBNF Parse Nodes:\n\n";
    for (auto n : nodes) {
        file << "bool " << n.name << "(params) {\n\tdeclare_node();\n\t";
        for (auto r : n.rules) {
            
            if (r.elements.size() == 1 && r.elements[0].value == "E") continue;
            file << "if (b && ";
            
            int i = 0;
            for (auto e : r.elements) {
                if (e.is_keyword) file << "keyword_(\"" << e.value << "\")";
                else if (e.is_operator) file << "operator_(\"" << e.value << "\")";
                else file << e.value << "(p)";
                if (++i != r.elements.size()) {
                    file << " && ";
                }
            }
            
            file << ") return success(parent, self);\n\t";
        }
        file << (n.optional ? "optional();" : "return failure(save, self);") << "\n}\n\n";
    }
    
}

int main(int argc, const char * argv[]) {
    
    bool inside_node = false;
    std::string line = "", token = "";
    
    std::ifstream grammar {input_grammar_filepath};
    process_nodes(grammar, inside_node, line, token);
    grammar.close();
    
    find_optionals();
    print_all_nodes();
    
    std::ifstream header {input_header_filepath};
    std::ofstream cppfile {output_cpp_filepath, std::ofstream::trunc};
    while (std::getline(header, line)) cppfile << line << std::endl;
    header.close();
    
    generate(cppfile);
    cppfile.close();
    
    return 0;
}
