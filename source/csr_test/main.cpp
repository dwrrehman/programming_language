//
//  main.cpp
//  sandbox8
//
//  Created by Daniel Rehman on 1902251.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

// ------------------
/// CSR TEST CODE:
// -------------------

/// description: the csr algorithm is responsible for resolving calls to abstractions, based on subexpression shape, types of abstractions, literals, etc.
/// its fairy involved, and super slow on erroneous input.

size_t max_expression_depth = 8; // the larger the number, the slower the algorithm, on error inputs.
//                                 ("10" will be the default in the compiler.)




// ------------ data structures -------------

struct element;

struct signature {
    std::vector<struct element> elements = {};
    struct signature* type = nullptr;
    bool erroneous = false;
};

struct element {
    std::string name = "";
    signature children = {};
    bool is_parameter = false;
};



/// Global builtin types. these are fundemental to the language.

// nullptr is the _type type.
signature unit_type = {};
signature nothing_type = {{{"_none"}}};
signature infered_type = {{{"_infered"}}};


// builtin types:       _type, _none, (), _infered


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

size_t current = 0;

signature turn_into_expression(std::vector<std::string> given) {
    signature expression = {};
    while (current < given.size() && given[current] != ")") {
        if (given[current] == "(") {
            current++; // "("
            auto s = turn_into_expression(given);
            expression.elements.push_back({"", s, true});
            current++; // ")"
        } else if (given[current] != ")") {
            expression.elements.push_back({given[current], {}, false});
            current++;
        }
    }
    return expression;
}



// --------------------- debuggers -----------------------------

#define prep(l)  for (size_t i = l; i--;) std::cout << ".   ";
#define prp(l)  for (size_t i = l; i--;) std::cout << "#   ";

void print_string_array(std::vector<std::string> array) {
    std::cout << "[";
    size_t i = 0;
    for (auto string : array) {
        std::cout << string;
        if (i < array.size() - 1) std::cout << " ";
        i++;
    }
    std::cout << "]";
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
    if (signature.type != nullptr) {
        std::cout << ":{";
        print_signature(*signature.type);
        std::cout << " }";
    } else {
        std::cout << ":{_type}";

    }
}


void debug_signature(signature signature) {
    if (signature.erroneous) {
        std::cout << "({ERROR})";
    }
    std::cout << "(\n";
    std::cout << "el count = " << signature.elements.size() << "\n";
    for (auto s : signature.elements) {
        if (!s.is_parameter) {
            std::cout << "\t\"" << s.name << "\"\n";
        } else {
            std::cout << "contains signature:\n";
            print_signature(s.children);
            std::cout << "\n";
        }

    }
    std::cout << ")\n\n";
}

void print_defined_signatures(const std::vector<signature, std::allocator<signature> > &signatures) {
    std::cout << "defined signatures:\n";
    for (auto s : signatures) {
        print_signature(s);
        std::cout << "\n";
    }
    std::cout << "\n";
}


/*

 known bug:

        parse _type end

                solution: {ERROR}

 */


void prune_extraneous_subexpressions(signature& given) {
    while (given.elements.size() == 1 && given.elements[0].is_parameter
           && given.elements[0].name == "" && given.elements[0].children.elements.size()) {
        auto temp = given.elements[0].children.elements;
        given.elements = temp;
    }
    for (auto& element : given.elements)
        if (element.is_parameter) prune_extraneous_subexpressions(element.children);
}


bool signatures_match(signature first, signature second);

bool elements_match(element first, element second) {
    if (first.name != second.name) return false;
    else if (first.is_parameter && second.is_parameter) return signatures_match(first.children, first.children);
    else if (!first.is_parameter && !second.is_parameter) return true;
    else return false;
}

bool signatures_match(signature first, signature second) {
    if (first.elements.size() != second.elements.size()) return false;
    for (size_t i = 0; i < first.elements.size(); i++) {
        if (!elements_match(first.elements[i], second.elements[i])) return false;
    }
    if (first.erroneous || second.erroneous) return false;
    if ((!first.type && !second.type) || signatures_match(*first.type, *second.type)) return true;
    else return false;
}

signature csr(const std::vector<signature> list, const signature given, const size_t depth, const size_t max_depth, size_t& pointer, struct signature*& type) {

    if (depth > max_depth) return {{}, nullptr, true};
    if (type && signatures_match(*type, nothing_type)) return {{}, nullptr, true};
    if (given.elements.empty() || (given.elements.size() == 1
                                   && given.elements[0].is_parameter
                                   && given.elements[0].children.elements.empty())) {
        if (given.elements.size() == 1
            && given.elements[0].is_parameter
            && given.elements[0].children.elements.empty()) pointer++;
        if (type && signatures_match(*type, infered_type)) type = &unit_type;
        if (!type || signatures_match(*type, unit_type)) return {{}, &unit_type};
        else return {{}, nullptr, true};
    }
    const size_t saved = pointer;
    for (auto signature : list) {
        if (type && !signatures_match(*type, infered_type) && (!signature.type || !signatures_match(*type, *signature.type))) continue;
        struct signature solution = {};
        pointer = saved;
        bool failed = false;
        for (auto element : signature.elements) {
            if (pointer >= given.elements.size()) { failed = true; break; }
            if (element.is_parameter) {
                auto subexpression = csr(list, given, depth + 1, max_depth, pointer, element.children.type);
                if (subexpression.erroneous) {
                    if (given.elements[pointer].is_parameter) {
                        size_t local_pointer = 0, current_depth = 0;
                        struct signature subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            subexpression = csr(list, given.elements[pointer].children, 0, current_depth, local_pointer, element.children.type);
                            if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) {
                                current_depth++;
                            } else break;
                        }
                        if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) { failed = true; break; }
                        solution.elements.push_back({"", subexpression, true});
                        pointer++; continue;
                    } else { failed = true; break; }
                } solution.elements.push_back({"", subexpression, true});
            } else if (element.name == given.elements[pointer].name) {
                solution.elements.push_back(element);
                pointer++;
            } else { failed = true; break; }
        } if (!failed) {
            if (type && signatures_match(*type, infered_type)) type = signature.type;
            solution.type = signature.type;
            return solution;
        }
    } return {{}, nullptr, true};
}


/*

signature csr_loud(const std::vector<signature> list, const signature given, const size_t depth, const size_t max_depth, size_t& pointer, const size_t sd) {

    if (depth > max_depth) {
        if (debug) {
           prp(sd); prep(depth); std::cout << "reached maximum depth: [depth=" << depth << ", max_depth=" << max_depth << " ]\n\n\n";
        }
        return {{}, nullptr, true};
    }

    if (debug) {
       prp(sd); prep(depth); std::cout << "------ CALLED CSR ------- \n";
       prp(sd); prep(depth); std::cout << "given list = {";
        for (auto l : list) {
            print_signature(l);
            std::cout << ", ";
        }
        std::cout << "}\n\n";
    }

    if (!given.elements.size()) {
        if (debug) {
            prp(sd); prep(depth); std::cout << "we were given a NULL expression! simply returning an empty call...\n";
        }
        return {{}, nullptr, false};
    }

    const size_t saved = pointer;
    for (auto signature : list) {

        if (debug) {
            prp(sd);prep(depth); std::cout << "- TRYING signature: ";
            print_signature(signature);
            std::cout << "\n\n";
        }

        struct signature solution = {};
        pointer = saved;
        bool failed = false;
        for (auto element : signature.elements) {


            if (pointer >= given.elements.size()) {
                if (debug) {
                    prp(sd);prep(depth); std::cout << "we hit a wall in given, lets just fail and break...\n";
                }
                failed = true; break;
            }

            if (debug) {

                prp(sd);prep(depth); std::cout << "testing element: ";
                if (!element.is_parameter) {
                    print_signature({{element}, nullptr, false});
                } else {
                    std::cout << "{PARAM}";
                }
                std::cout << "\n";

                if (pointer < given.elements.size()) {
                    prp(sd);prep(depth); std::cout << "@pointer: ";
                    if (given.elements[pointer].is_parameter) {
                        std::cout << "{SUBEXPR: ";
                        print_signature(given.elements[pointer].children);
                        std::cout << "}";
                    } else {
                        std::cout << "\"" << given.elements[pointer].name << "\"";
                    }
                    std::cout << "\n";
                }
                std::cout << "\n";
            }


            if (element.is_parameter && pointer < given.elements.size()) {

                if (debug) {
                    prp(sd);prep(depth); std::cout << "traversing nested call...\n";
                    prp(sd);prep(depth); std::cout << "current depth = " << depth << "\n";
                }

                auto subexpression = csr_loud(list, given, depth + 1, max_depth, pointer, sd);
                if (subexpression.erroneous) {
                    if (debug) {
                        prp(sd);prep(depth); std::cout << "failed to parse nested call...  checking to see if its a subexpr...\n\n";
                    }

                    if (pointer < given.elements.size() && given.elements[pointer].is_parameter) {

                        if (debug) {
                            prp(sd);prep(depth); std::cout << "trying as a parameter subexpression instead.\n";
                            prp(sd);prep(depth); std::cout << "found this subexpression : ";
                            print_signature(given.elements[pointer].children);
                            std::cout << "\n";

                            prp(sd);prep(depth); std::cout << "traversing subexpr...\n";
                        }


                        size_t local_pointer = 0, current_depth = 0;
                        struct signature subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            if (debug) {
                                prp(sd);prep(depth); std::cout << "subexpr: trying depth = " << current_depth << std::endl;
                            }
                            subexpression = csr_loud(list, given.elements[pointer].children, 0, current_depth, local_pointer, sd + 1);
                            if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) {
                                current_depth++;
                            } else break;
                        }
                        if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) {
                            if (debug) {
                                prp(sd);prep(depth); std::cout << "failed to parse subexpression.\n\n";
                            }
                            failed = true; break;
                        }

                        if (debug) {
                            prp(sd);prep(depth); std::cout << "successfully parsed subexpression.\n";
                        }
                        solution.elements.push_back({"", subexpression, true});
                        pointer++;
                        if (debug) {
                            prp(sd);prep(depth); std::cout << "pointer was " << pointer - 1 << ", now its " << pointer << "\n\n";
                        }
                        continue;
                    } else {
                        if (debug) {
                            prp(sd);prep(depth); std::cout << "it wasnt a subexpression, so nested call has failed.\n\n";
                        }

                        failed = true; break;
                    }
                }

                if (debug) {
                    prp(sd); prep(depth); std::cout << "successfully parsed nested call.\n\n";
                }
                solution.elements.push_back({"", subexpression, true});


            } else if (pointer < given.elements.size() && element.name == given.elements[pointer].name) {
                if (debug) {
                    prp(sd);prep(depth); std::cout << "found matching element: \"" << element.name << "\"\n";
                }
                solution.elements.push_back(element);
                pointer++;
                if (debug) {
                    prp(sd);prep(depth); std::cout << "pointer was " << pointer - 1 << ", now its " << pointer << "\n\n";
                }

            } else {
                if (debug) {
                    prp(sd);prep(depth); std::cout << "failed to match: " << element.name << "\n\n";
                }
                failed = true; break;
            }


        }
        if (!failed) {
            if (debug) {
                prp(sd);prep(depth); std::cout << "[successfully parsed solution]\n";

                prp(sd);prep(depth); std::cout << "found solution: ";
                print_signature(solution);
                std::cout << "\n\n\n";
            }
            return solution;
        }
    }
    if (debug) {
        prp(sd);prep(depth); std::cout << "[failed to parse any solution.]\n\n\n";
    }
    return {{}, nullptr, true};
}

 */


int main() {

    signature int_type = {
        {
            {"int", {}, false}
        }, &unit_type, false};

    signature dog_type = {
        {
            {"dog", {}, false}
        }, &int_type, false};

    signature print_type = {
        {
            {"print", {}, false},
            {"", {{}, &int_type, false}, true}
        }, &int_type, false};

    signature unit_to_int_type = {
        {
            {"", {{}, &unit_type, false}, true}
        }, &int_type, false};

    std::vector<struct signature> signatures = {nothing_type, infered_type, int_type, print_type, dog_type, unit_to_int_type};

    std::sort(signatures.begin(), signatures.end(), [](auto a, auto b) { return a.elements.size() > b.elements.size(); });

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

            current = 0;
            auto given = turn_into_expression(tokens);

            std::cout << "parsing: ";
            print_string_array(tokens);
            std::cout << " and ";
            print_signature(given);
            prune_extraneous_subexpressions(given);
            std::cout << ", aka ";
            print_signature(given);
            std::cout << "\n";

            size_t pointer = 0;
            signature solution = {};
            signature* type = &infered_type;
            size_t max_depth = 0;
            while (max_depth <= max_expression_depth) {
                std::cout << "trying depth = " << max_depth << std::endl; // debug
                pointer = 0;
                type = &infered_type;
                solution = csr(signatures, given, 0, max_depth, pointer, type);
                if (debug) std::cout << "\n\n\n\n";
                if (solution.erroneous || pointer < given.elements.size()) {
                    max_depth++;
                }
                else break;
            }

            std::cout << "\nsolution: ";
            print_signature(solution);
            std::cout << "\n";

            std::cout << "it has type = ";
            if (type) print_signature(*type); else std::cout << "{_type}";
            std::cout << "\n";
            if (pointer < given.elements.size()) {
                std::cout << "(but its erroenous)...\n";
            }

        } else if (command == "add") {

        } else if (command == "show") {
            print_defined_signatures(signatures);

        } else if (command == "help") {
            std::cout << "commands:\n\t - add <signature> end\n\t - show\n\t - parse <expression> end\n\t - quit\n\t - clear\n\t - depth <nat>\n\n";

        } else if (command == "clear") {
            system("clear");

        } else if (command == "depth") {

            std::string mode = "";
            std::cin >> mode;
            if (mode == "set") {
                std::cin >> max_expression_depth;
                std::cout << "set depth: " << max_expression_depth << "\n";
            } else if (mode == "get") {
                std::cout << "maximum depth = " << max_expression_depth << "\n";
            } else {
                std::cout << "unrecognized mode.\n";
            }
        } else {
            std::cout << command << ": command not found.\n";
        }
    }
    std::cout << "quitting...\n";
}
