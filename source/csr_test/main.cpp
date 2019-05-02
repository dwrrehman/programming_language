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

size_t max_expression_depth = 7; // the larger the number, the slower the algorithm, on error inputs.
//                                 ("10" will be the default in the compiler.)







/**



    problem expressions:

        - ((x is good) is a number)

            solution: ()
            (but its erroenous)...




        - (x + +)

        solution: (((x) + ()) + ())



*/


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








// ----------------------- csr solver -----------------------

/*


 --------------------  things to add to csr: --------------------------


         IMPORTANT:

 almost done:   1    - x: subsignatures: grouping of signatures.;

                2    - types, incoorperated into signatures.


-------------------------------------------------------------------------

note:

            after we get subexpressions working perfectly,
            we will move onto doing types, and then we are going to try to insert this code into the compiler, somehow.

            we will probably start by subsituting in the data types in replace of the testing dummy-data-types we have in csr currently.


            however, to make it actually viable as a compiler computational unit,


        we crucially need:



      1.      - implicitly called functions                       (this seems like it will be dead easy, after types are implemented.)


      2.      - function defined identfiers/abstractions/expressions.                 (...this is the hard part, i think)                                            - - this is through binding a arbitrary expression (not necc enclosed in parens) to a parameter of type, "_expression".




-------------------------------------------------------------------------



   EASY:



   1.      - add the compiler builtin signatures to the list, without the user having to define them.
         - literals: blocks, strings, llvm-strings

                - ie, add the compiler defined abstractions/datatypes/signatures, to the list, which are present at all times.





   2.      - regex signatures: numeric literals.            we are NOT going to use regex.

            - to do numeric literals, we need signature abstractions. badly. they will provide the mechanism to abritrarily handle a literal (which has a particular signature-like form), like this.


-------------------------------------------------------------------------

 HARD:

        - implicit abstraction calls, only with type classes.

        - function defined variables, (signatures that arent defined yet, being passed into a function. (coping with undefined signatures.)



-------------------------------------------------------------------------







            _ is good

            _ is a number

            x



            (x) is a number is good

             ^







attempt 1 - subexpression integration.

signature csr(const std::vector<signature> list, const size_t depth, const signature given, const size_t max_depth, size_t& pointer) {
    if (depth > max_depth) return {{}, true};
    const size_t saved = pointer;
    for (auto signature : list) {
        struct signature solution = {};
        pointer = saved;
        bool failed = false;
        for (auto element : signature.elements) {
            if (element.is_parameter && pointer < given.elements.size() && given.elements[pointer].is_parameter) {
                size_t local_pointer = 0;
                struct signature subexpression = csr(list, 0, given.elements[pointer].children, max_depth, local_pointer);
                if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) { failed = true; break; }
                solution.elements.push_back({"", subexpression, true});
                pointer++;

            } else if (element.is_parameter && depth < max_depth) {
                struct signature subexpression = csr(list, depth + 1, given, max_depth, pointer);
                if (subexpression.erroneous) { failed = true; break; }
                solution.elements.push_back({"", subexpression, true});

            } else if (pointer < given.elements.size() && element.name == given.elements[pointer].name && !given.elements[pointer].is_parameter) {
                solution.elements.push_back(element);
                pointer++;

            } else { failed = true; break; }
        } if (!failed) {
            return solution;
        }
    } return {{}, true};
}


 //            if (pointer < given.elements.size() && given.elements[pointer].is_parameter) {
 //                std::cout << "found a subexpression!: ";
 //                debug_signature(given.elements[pointer].children);
 //                std::cout << "\n";
 //            }


 // = csr(list, given.elements[pointer].children, 0, max_depth, local_pointer);


*/

signature csr(const std::vector<signature> list, const signature given, const size_t depth, const size_t max_depth, size_t& pointer) {
    if (depth > max_depth) return {{}, true};
    const size_t saved = pointer;
    for (auto signature : list) {
        struct signature solution = {};
        pointer = saved;
        bool failed = false;
        for (auto element : signature.elements) {
            if (pointer >= given.elements.size()) return solution;
            if (element.is_parameter) {
                auto subexpression = csr(list, given, depth + 1, max_depth, pointer);
                if (subexpression.erroneous) {
                    if (pointer < given.elements.size() && given.elements[pointer].is_parameter) {
                        size_t local_pointer = 0, current_depth = 0;
                        struct signature subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            subexpression = csr(list, given.elements[pointer].children, 0, current_depth, local_pointer);
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
        } if (!failed) return solution;
    } return {{}, true};
}

/*


        ----- working csr with subexpression         - with debug info -        ----------------------


bool debug = false;

signature csr(const std::vector<signature> list, const signature given, const size_t depth, const size_t max_depth, size_t& pointer) {

    if (depth > max_depth) {
        if (debug) {
            prep(depth); std::cout << "reached maximum depth.\n\n\n";
        }
        return {{}, true};
    }

    if (debug) {
        prep(depth); std::cout << "------ CALLED CSR ------- \n";
        prep(depth); std::cout << "given list = {";
        for (auto l : list) {
            print_signature(l);
            std::cout << ", ";
        }
        std::cout << "}\n\n";
    }

    const size_t saved = pointer;
    for (auto signature : list) {

        if (debug) {
            prep(depth); std::cout << "- TRYING signature: ";
            print_signature(signature);
            std::cout << "\n\n";
        }

        struct signature solution = {};
        pointer = saved;
        bool failed = false;
        for (auto element : signature.elements) {


            if (pointer >= given.elements.size()) {
                if (debug) {
                    prep(depth); std::cout << "we hit a wall, lets just return the solution...\n";
                }
                return solution;
            }

            if (debug) {

                prep(depth); std::cout << "testing element: ";
                if (!element.is_parameter) {
                    print_signature({{element}, false});
                } else {
                    std::cout << "{PARAM}";
                }
                std::cout << "\n";

                if (pointer < given.elements.size()) {
                    prep(depth); std::cout << "@pointer: ";
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


            if (element.is_parameter) {

                if (debug) {
                    prep(depth); std::cout << "traversing nested call...\n";
                    prep(depth); std::cout << "current depth = " << depth << "\n";
                }

                auto subexpression = csr(list, given, depth + 1, max_depth, pointer);
                if (subexpression.erroneous) {
                    if (debug) {
                        prep(depth); std::cout << "failed to parse nested call.\n\n";
                    }

                    if (pointer < given.elements.size() && given.elements[pointer].is_parameter) {

                        if (debug) {
                            prep(depth); std::cout << "trying as a parameter subexpression instead.\n";
                            prep(depth); std::cout << "found this subexpression : ";
                            print_signature(given.elements[pointer].children);
                            std::cout << "\n";

                            prep(depth); std::cout << "traversing subexpr...\n";
                        }


                        size_t local_pointer = 0, current_depth = 0;
                        struct signature subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            if (debug) {
                                prep(depth); std::cout << "subexpr: trying depth = " << current_depth << std::endl;
                            }
                            subexpression = csr(list, given.elements[pointer].children, 0, current_depth, local_pointer);
                            if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) {
                                current_depth++;
                            } else break;
                        }
                        if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) {
                            if (debug) {
                                prep(depth); std::cout << "failed to parse subexpression.\n\n";
                            }
                            failed = true; break;
                        }

                        if (debug) {
                            prep(depth); std::cout << "successfully parsed subexpression.\n";
                        }
                        solution.elements.push_back({"", subexpression, true});
                        pointer++;
                        if (debug) {
                            prep(depth); std::cout << "pointer was " << pointer - 1 << ", now its " << pointer << "\n\n";
                        }
                        continue;
                    } else {
                        failed = true; break;
                    }
                }

                if (debug) {
                    prep(depth); std::cout << "successfully parsed nested call.\n\n";
                }
                solution.elements.push_back({"", subexpression, true});



            } else if (pointer < given.elements.size() && element.name == given.elements[pointer].name) {
                if (debug) {
                    prep(depth); std::cout << "found matching element: \"" << element.name << "\"\n";
                }
                solution.elements.push_back(element);
                pointer++;
                if (debug) {
                    prep(depth); std::cout << "pointer was " << pointer - 1 << ", now its " << pointer << "\n\n";
                }

            } else {
                if (debug) {
                    prep(depth); std::cout << "failed to match: " << element.name << "\n\n";
                }
                failed = true; break;
            }


        }
        if (!failed) {
            if (debug) {
                prep(depth); std::cout << "[successfully parsed solution]\n";

                prep(depth); std::cout << "found solution: ";
                print_signature(solution);
                std::cout << "\n\n\n";
            }
            return solution;
        }
    }
    if (debug) {
        prep(depth); std::cout << "[failed to parse any solution.]\n\n\n";
    }
    return {{}, true};
}

*/

/*              past csr aglorithmn:   most basic working form.



        we really need to go back to this, and try to add in subexpressions again, and get it right this time.



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
*/



int main() {

    std::vector<std::string> string_signatures = {
        "_ is a number",
        "_ is good",
        "print _",
        //"print",
        //"my self",
        //"my good",
        "x",
        //"_ + _",
    };


    std::vector<struct signature> signatures = convert_all(string_signatures);
    signatures.push_back({});

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
            std::cout << "\n";

            size_t pointer = 0;
            signature solution = {};
            size_t max_depth = 0;
            while (max_depth <= max_expression_depth) {
                std::cout << "trying depth = " << max_depth << std::endl; // debug
                pointer = 0;
                solution = csr(signatures, given, 0, max_depth, pointer);
                if (solution.erroneous || pointer < given.elements.size()) {
                    max_depth++;
                }
                else break;
            }

            std::cout << "\nsolution: ";
            print_signature(solution);
            std::cout << "\n";
            if (pointer < given.elements.size()) {
                std::cout << "(but its erroenous)...\n";
            }

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
