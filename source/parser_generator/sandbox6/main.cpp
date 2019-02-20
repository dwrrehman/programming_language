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

const std::string main_directory = "/Users/deniylreimn/Documents/projects/programming language/";

const std::string input_grammar_filepath = main_directory + "specification/ebnf grammar for my programming language full.txt";
const std::string output_cpp_filepath = main_directory + "source/parser_generator/sandbox6/parser.cpp";
const std::string input_header_filepath = main_directory + "source/parser_generator/sandbox6/parser_header.cpp";
const std::string input_footer_filepath = main_directory + "source/parser_generator/sandbox6/parser_footer.cpp";

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


static void check_for_error() {
    if (current_node >= nodes.size() || current_rule >= nodes[current_node].rules.size()) {
        std::cout << "Syntax error in EBNF grammar file.\n";
        std::cout << "current node and rule #: node: " << current_node << ", rule: " << current_rule << "\n";
        std::cout << "generator got this far: \n";
        print_all_nodes();
        std::cout << "\n\naborting..." << std::endl;
        exit(1);
    }
}

// helpers:

static void add_element(const std::string &token) {
    check_for_error();
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

static void process_line(bool &inside_node, const std::string &line) {
    
    std::string token = "";
    std::istringstream s {line};
    size_t element_count = 0;
    
    while (s.good()) {
        s >> token;
        element_count++;
        if (!add_node(inside_node, token) && !add_rule(element_count, token))
            add_element(token);
    }
}

static void process_nodes(std::ifstream &grammar, bool &inside_node, std::string &line) {
    
    while (std::getline(grammar, line)) {
        if (line != "" && line[0] == '-') continue;
        else if (line != "") process_line(inside_node, line);
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
        if (n.name == "declaration") {
            file << "deepest_level = 0;\n\t";
        }
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
    std::string line = "";
    
    std::ifstream grammar {input_grammar_filepath};
    process_nodes(grammar, inside_node, line);
    grammar.close();
    
    find_optionals();
    print_all_nodes();
    
    std::ofstream cppfile {output_cpp_filepath, std::ofstream::trunc};
    
    std::ifstream header {input_header_filepath};
    while (std::getline(header, line)) cppfile << line << std::endl;
    header.close();
    
    generate(cppfile);
    
    std::ifstream footer {input_footer_filepath};
    while (std::getline(footer, line)) cppfile << line << std::endl;
    footer.close();

    cppfile.close();
    
    return 0;
}
