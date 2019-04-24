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


const int max_depth_level = 8;

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#define prep(l)  for (size_t i = l; i--;) std::cout << ".   ";

// ------------ data structures -------------

struct element;

struct signature {
    std::vector<struct element> elements;
};

struct element {
    std::string name = "";
    signature children = {};
    bool is_parameter = false;
};

// -------------- helpers ---------------------

std::vector<signature> convert(const std::vector<std::string> &string_signatures) {
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
    return result;
}

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

void print_expression(std::vector<std::string> e) {
    print_string_array(e);
}

void print_signature(signature signature) {
    std::cout << "(";
    size_t i = 0;
    for (auto s : signature.elements) {
        if (!s.is_parameter) {
            std::cout << s.name;
        } else {
            std::cout << ":";
            print_signature(s.children);
            std::cout << ":";
        }
        if (i < signature.elements.size() - 1) std::cout << " ";
        i++;
    }
    std::cout << ")";
}


// ----------------------- solver ---------------------------------



int pointer = 0;

int save() {
    return pointer;
}

void revert(int saved) {
    pointer = saved;
}


signature csr_commentless(const std::vector<signature> list, const int depth, const std::vector<std::string> given_expression) {

    int saved = save();

    for (auto signature : list) {

        revert(saved);
        struct signature parent = {};
        int i = 0;

        for (; i < signature.elements.size(); i++) {

            if (signature.elements[i].name == "" && depth < max_depth_level) {
                struct element result = {"", {}, true};
                result.children = csr_commentless(list, depth + 1, given_expression);
                parent.elements.push_back(result);

            } else if (pointer < given_expression.size() && signature.elements[i].name == given_expression[pointer]) {
                parent.elements.push_back(signature.elements[i]);
                pointer++;

            } else if (pointer < given_expression.size()) {
                break; // goto next signature, because this one is not it.
            }
        }
        if (i == signature.elements.size() || pointer >= given_expression.size()) return parent;
    }
    return {{{"Error", {}, false}}}; // ERROR:
}



/*


literal translation:

(csr list depth given expression) -> signature {
    saved = save
    for signature in list,
        parent = empty signature
        revert to saved
        e = 0
        for e < signature element count, e++ {
            if name of signature elements[e] == "" and depth < max depth
                result = default element
                result children = csr list (depth + 1) given expression
                parent elements += result
            else if (pointer < given expression count
                    and name of signature elements[e] == given expression[pointer]),
                parents elements += signature elements[e]
                pointer++
            else if pointer < given expression count {break}
        }
        if e == signature elements count, {return parent}
     }
     return error
}


sudo code tranaslation:


(csr list depth given expression) signature {
    saved = save
    for signature in list,

        parent = empty signature
        revert to saved

        for each signature element in this signature {


            if this signature element is a parameter and depth < max depth

                result = default element
                result children = csr list (depth + 1) given expression
                append result to parent elements

                else if we still have some left and they match,

                    append this signature element to parents elements
                    pointer++

                else if we still have some left,

                    goto next signature

        }

        if there are no more elements to process, {return success}
    }
    return error
}

 
*/






signature csr(const std::vector<signature> list, const int depth, const std::vector<std::string> given_expression) {
    int saved = save();
    for (auto signature : list) {
        struct signature parent = {};
        revert(saved);
        int e = 0;
        for (; e < signature.elements.size(); e++) {
            if (signature.elements[e].name == "" && depth < max_depth_level) {
                struct element result = {"", {}, true};
                result.children = csr(list, depth + 1, given_expression);
                parent.elements.push_back(result);
            } else if (pointer < given_expression.size() && signature.elements[e].name == given_expression[pointer]) {
                parent.elements.push_back(signature.elements[e]);
                pointer++;
            } else if (pointer < given_expression.size()) break;
        }
        if (e == signature.elements.size()) return parent;
    }
    return {{{"Error", {}, false}}};
}


signature fix_csr(signature s) {
    if (s.elements.size() == 1 && s.elements[0].is_parameter) return fix_csr(s.elements[0].children);
    else return s;
}

signature csr_comments(const std::vector<signature> list, const int depth, const std::vector<std::string> given_expression) {

    int saved = save();

    for (auto signature : list) {

        prep(depth); std::cout << "looking for: ";
        print_signature(signature);
        std::cout << std::endl;

        struct signature parent = {};

        int i = 0;
        for (; i < signature.elements.size(); i++) {

            prep(depth); std::cout << "looking for signaure element: " << signature.elements[i].name << " (is_param = " << std::boolalpha << signature.elements[i].is_parameter << ")\n";

            if (pointer < given_expression.size()) {
                prep(depth); std::cout << "expression[pointer] = " << given_expression[pointer];
            }
            std::cout << std::endl;


            if (signature.elements[i].name == "" && depth < max_depth_level) {
                prep(depth); std::cout << "found a parameter element! recursing...\n\n";
                struct element result = {"", {}, true};
                result.children = csr_comments(list, depth + 1, given_expression);
                parent.elements.push_back(result);

            } else if (pointer < given_expression.size() && signature.elements[i].name == given_expression[pointer]) {
                prep(depth); std::cout << "found equal elements! advancing pointer...\n";
                prep(depth); std::cout << "(pointer was " << pointer << ", now its " << pointer + 1 << ")\n";

                parent.elements.push_back(signature.elements[i]);
                pointer++;

                prep(depth); std::cout << "parent is now: ";
                print_signature(parent);
                std::cout << "\n\n\n" << std::endl;

            } else if (pointer < given_expression.size()) {
                prep(depth); std::cout << "going to next signature, this one is not it.\n\n" << std::endl;
                break; // goto next signature, because this one is not it.
            }

        }

        if (i == signature.elements.size()) {
            prep(depth);std::cout << "finished successfully parsing signature: ";
            print_signature(signature);
            std::cout << std::endl;

            prep(depth); std::cout << "returning parent: ";
            print_signature(parent);
            std::cout << "\n\n\n\n";
            return parent;
        }

        revert(saved);
    }
    return {{{"Error", {}, false}}}; // ERROR:
}



std::vector<std::string> convert_into_expression(std::string given_expression) {
    std::vector<std::string> expression = {};
    std::istringstream stream {given_expression};
    while (stream.good()) {
        std::string element = "";
        stream >> element;
        expression.push_back(element);
    }
    return expression;
}

int main() {


    std::vector<std::string> string_signatures = {
        "_ is a number",
        "add _ to _",
        "print _",
        "my self",
        "my",
    };

    const std::string expression = "my self is a number";






    std::sort(string_signatures.begin(), string_signatures.end(), [](auto a, auto b) { return a.size() > b.size(); });
    auto signatures = convert(string_signatures);
    
    // debug:
    std::cout << "printing all known signatures:\n";
    for (auto s : signatures) {
        print_signature(s);
        std::cout << "\n";
    }
    std::cout << "done printing all known sigs\n\n\n\n";
    
    const std::vector<std::string> given = convert_into_expression(expression);
    
    signature solution = csr(signatures, 0, given);
    solution = fix_csr(solution);

    std::cout << "\n\n\n\n\nprinting solution: \n";
    print_signature(solution);
    std::cout << "\n\n";
}
