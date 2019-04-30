//
//  main.cpp
//  sandbox8
//
//  Created by Daniel Rehman on 1902251.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

// ------------------ treating the call graph as a parsabl ebnf grammar, which can be solved using a dynamically made parser.

/*

 we need to construct a set of functions, which are all essentially of the same form, they have a particular set of signature elements, which are terminals, (a raw string), (from the signature)

 and they all have a (nonoptional) non-terminal "expression" node as their recognizer function for a parameter.

 */




#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>


size_t max_expression_depth = 10;



// ------------ data structures -------------

struct element;

struct signature {
    std::vector<struct element> elements = {};
    bool erroneous = false;
};

struct element {
    std::string name = "";
    signature children = {};
    bool is_parameter = false;
};








// -------------- formatters ---------------------

std::vector<signature> convert_all(std::vector<std::string> string_signatures) {

    std::vector<signature> result = {};
    size_t i = 0;
    
    for (auto string_signature : string_signatures) {
        result.push_back({});
        std::istringstream stream(string_signature);
        std::string element = "";
        while (stream.good()) {
            stream >> element;
            if (element == "_") result[i].elements.push_back({"", {}, true});
            else result[i].elements.push_back({element, {}, false});
        }
        i++;
    }

    std::sort(result.begin(), result.end(), [](auto a, auto b) { return a.elements.size() > b.elements.size(); });

    return result;
}

std::vector<std::string> tokenize(std::string given_expression) {
    std::vector<std::string> expression = {};
    std::istringstream stream {given_expression};
    while (stream.good()) {
        std::string element = "";
        stream >> element;
        expression.push_back(element);
    }
    return expression;
}

 signature turn_into_expression(std::vector<std::string> given) {
    std::vector<std::string> expression = {};
    std::istringstream stream {given_expression};
    while (stream.good()) {
        std::string element = "";
        stream >> element;
        expression.push_back(element);
    }
    return expression;
}





// --------------------- debuggers -----------------------------

#define prep(l)  for (size_t i = l; i--;) std::cout << ".   ";

void print_string_array(std::vector<std::string> array) {
    std::cout << "[";
    size_t i = 0;
    for (auto string : array) {
        std::cout << string;
        if (i < array.size() - 1) std::cout << " ";
        i++;
    }
    std::cout << "]\n";
}


void print_signature(signature signature) {
    if (signature.erroneous) {
        std::cout << "({ERROR})";
        return;
    }
    std::cout << "(";
    size_t i = 0;
    for (auto s : signature.elements) {
        if (!s.is_parameter) {
            std::cout << s.name;
        } else {
            print_signature(s.children);
        }
        if (i < signature.elements.size() - 1) std::cout << " ";
        i++;
    }
    std::cout << ")";
}

void print_defined_signatures(const std::vector<signature, std::allocator<signature> > &signatures) {
    std::cout << "defined signatures:\n";
    for (auto s : signatures) {
        print_signature(s);
        std::cout << "\n";
    }
    std::cout << "\n";
}

// --------------------------------------------------------












/// ----------- solver ------------------

int pointer = 0;

int pointer_save() {
    return pointer;
}

void pointer_revert(int saved) {
    pointer = saved;
}






















/*


 things to add to csr:



 IMPORTANT:

 1    - subsignatures: grouping of signatures.

 2    - types, incoorperated into signatures.



 HARD:

 - implicit abstraction calls, only with type classes.

 - function defined variables, (signatures that arent defined yet, being passed into a function. (coping with undefined signatures.)

 EASY:

 - add the compiler builtin signatures to the list, without the user having to define them.

 - literals: blocks, strings, llvm-strings

 - regex signatures: numeric literals.


 */





signature csr(const std::vector<signature> list, const size_t depth, const std::vector<std::string> given_expression, const size_t max_depth) {
    if (depth >= max_expression_depth) return {{}, true};
    int saved = pointer_save();
    for (auto signature : list) {
        struct signature solution = {};
        pointer_revert(saved);
        bool failed = false;
        for (auto element : signature.elements) {
            if (element.is_parameter && depth < max_depth) {
                auto s = csr(list, depth + 1, given_expression, max_depth);
                struct element result = {"", s, true};
                if (!s.erroneous) solution.elements.push_back(result);
                else { failed = true; break; }
            } else if (pointer < given_expression.size()
                       && element.name == given_expression[pointer]) {
                solution.elements.push_back(element);
                pointer++;
            } else { failed = true; break; }
        } if (!failed) return solution;
    } return {{}, true};
}


























int main() {


    const bool debug = false;


    std::vector<std::string> string_signatures = {
        //"_ is a number",
        "_ is good",
        "print _",
        "print",
        "my self",
        "my good",
        "x",
        "_ + _",
        "_ * _",
        "_ ^ _",
    };
    std::vector<struct signature> signatures = convert_all(string_signatures);

    std::string command = "";
    while (command != "quit") {

        std::cout << "::> ";
        std::cin >> command;

        if (command == "parse" || command == "do" || command == "solve" || command == "csr" || command == "f" || command == "p") {
            std::string expression = "";

            std::string element = "";
            while (element != "end") {
                std::cin >> element;
                if (element != "end") expression += element + " ";
            }

            auto tokens = tokenize(expression);
            tokens.pop_back();
            auto given = turn_into_expression(tokens);


            std::cout << "parsing: ";
            print_string_array(tokens);
            std::cout << " and ";
            print_expression(given);
            std::cout << "\n";


            signature solution = {};

            size_t max_depth = 1;

            while (max_depth <= max_expression_depth) {
                std::cout << "trying depth = " << max_depth << std::endl;
                pointer = 0;
                solution = csr(signatures, 0, given, max_depth);
                if (pointer < given.size() || solution.erroneous) max_depth++;
                else break;
            }

            std::cout << "\nsolution: ";
            print_signature(solution);
            std::cout << "\n";
            if (pointer < given.size()) std::cout << "(but its erroneous) " << given.size() << " : " << pointer << "\n\n";


        } else if (command == "add") {

            std::string element = "", new_signature = "";

            while (element != "end") {
                std::cin >> element;
                if (element != "end") new_signature += element + " ";
            }

            new_signature.pop_back();
            std::cout << "adding " << new_signature << "...\n";
            string_signatures.push_back(new_signature);
            signatures = convert_all(string_signatures);


        } else if (command == "show") {
            print_defined_signatures(signatures);

        } else if (command == "help") {
            std::cout << "commands:\n\t - add <signature> end\n\t - show\n\t - parse <expression> end\n\t - quit\n\t - clear\n\t - depth <nat>\n\n";

        } else if (command == "clear") {
            system("clear");

        } else if (command == "depth") {
            std::cin >> max_expression_depth;
            std::cout << "set depth: " << max_expression_depth << "\n";

        } else {
            std::cout << command << ": command not found.\n";
        }
    }
    std::cout << "quitting...\n";
}

