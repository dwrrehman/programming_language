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

size_t max_expression_depth = 3; // the larger the number, the slower the algorithm, on error inputs.
//                                 ("10" will be the default in the compiler.)







/*

        todo:

                we need to perform s reductio before we pass a expressiont  ocsr.


                lets test more subexpressions, after we implement a function in order to perform s reduction.


    s reduction standard for subexpression rediction,

        subexpression referes to the act of reducing: ( x ) ===> x



            this is very important.


 this is done preior t parsing expressions for


 so when type checking, we need to peform s red.

                ie, we need to not only perform sr before csr, but also before performing se-eq.
*/


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






// -------------- formatters ---------------------
//
//std::vector<signature> convert_all(std::vector<std::pair<std::string, size_t>> string_signatures) {
//
////    std::vector<signature> result = {};
////    size_t i = 0;
////
////    for (auto string_signature : string_signatures) {
////        result.push_back({});
////        std::istringstream stream(string_signature.first);
////        std::string element = "";
////        while (stream.good()) {
////            stream >> element;
////            if (isnumber(element[0])) {
////                result[i].elements.push_back({"", {elements, type, false}, true});
////            }
////
////            else result[i].elements.push_back({element, {}, false});
////        }
////        i++;
////    }
//
//    std::sort(result.begin(), result.end(), [](auto a, auto b) { return a.elements.size() > b.elements.size(); });
//
//    return result;
//}

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

            note:       about the type system:


                - when we find a signature.type of {nullptr}, this means

                            it has type "_type".



            _type is the type which is the super type of all types.




        note: the difference "has type" and "is a".

            wait, is there a difference?



                0 has type int.             int has type _type.

                0 is an int.                int is a _type.


        tbh, i think there is no difference.





 what is the type of _type?


        good question.


        I think we should make this an error.


        more specfically, the program should error saying:


        "

            n3zqx2l: file.n:4:5: error: attempting to get the type of "_type"

        "



        alterantively, we could actually try to define it, as _type1, _type2, and so on.
        this would probably take alot of work for very little gain, though.















                    note: when we encounter a parameter which has type     "_type"   (ie, the nullptr)


                            we must accept any or all values/types which want to fill that parameter.


                this reflects the fact that





















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


// ----------------------------------- csr ------------------------------------------



/*





 when we go into a parameter, to parse it, we simply
 set the parent type of the recursive call to csr(...) to be the parameter's type.




 then crucially:


 when we are going through each signature, to figure
 out whether this one is the signature we need to call, we simply
 only start to parse the signature if:


 the return type of the signature *matches* our parent type.
 or
 the return type is "_infered"



 */


bool signatures_match(signature first, signature second);

bool elements_match(element first, element second) {
    if (first.name != second.name) return false;
    else if (first.is_parameter && second.is_parameter) return signatures_match(first.children, first.children);
    else if (!first.is_parameter && !second.is_parameter) return true;
    else return false;
}

bool signatures_match(signature first, signature second) {

    std::cout << "testing to see if ";
    print_signature(first);
    std::cout << " and ";
    print_signature(second);
    std::cout << " match... ";

    if (first.elements.size() != second.elements.size()) {
        std::cout << "sizes dont match.\n";
        return false;
    }
    for (size_t i = 0; i < first.elements.size(); i++) {
        if (!elements_match(first.elements[i], second.elements[i])) {
            std::cout << "found a differing element.\n";
            return false;
        }
    }
    if (first.erroneous || second.erroneous) {
        std::cout << "one or more was erronous.\n";
        return false;
    }
    if ((!first.type && !second.type) || signatures_match(*first.type, *second.type)) {
        std::cout << "they match!\n";
        return true;
     } else {
         std::cout << "they dont match.\n";
          return false;
        }



}


bool debug = true;

signature csr(const std::vector<signature> list, const signature given, const size_t depth, const size_t max_depth, size_t& pointer, struct signature*& type) {

    prep(depth);  std::cout << "parent_type = ";
    if (!type) std::cout << "{{{_anything}}}";
    else print_signature(*type);
    std::cout << "\n";

    if (depth > max_depth) return {{}, nullptr, true};
    if (type && signatures_match(*type, nothing_type)) {

        prep(depth); std::cout << "trying to match _nothing... returning failure.\n";

        return {{}, nullptr, true};
    }
    if (given.elements.empty() || (given.elements.size() == 1
                                   && given.elements[0].is_parameter
                                   && given.elements[0].children.elements.empty())) {

        if (given.elements.size() == 1
            && given.elements[0].is_parameter
            && given.elements[0].children.elements.empty()) pointer++;

        if (type && signatures_match(*type, infered_type)) {
            prep(depth); std::cout << "parent type was found to be infered, filling in...\n";
            type = &unit_type;
            prep(depth); std::cout << "now pt = ";
            print_signature(*type);
            std::cout << "\n";
        }

        if (!type || signatures_match(*type, unit_type)) {
            prep(depth); std::cout << "trying to unit type... success!.\n";
            return {{}, &unit_type};
        }
        else {
            prep(depth); std::cout << "failed to match unit type... returning failure.\n";
            return {{}, nullptr, true};
        }
    }
    const size_t saved = pointer;
    for (auto signature : list) {

        if (debug) {
            prep(depth); std::cout << "- TRYING signature: ";
            print_signature(signature);
            std::cout << "\n\n";
        }

        if (type && !signatures_match(*type, infered_type)
            && (!signature.type || !signatures_match(*type, *signature.type))) {
            prep(depth); std::cout << "skipping this signature, the types dont line up!\n";
            continue;
        }
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

            prep(depth); std::cout << "found a solution of type: ";
            if (solution.type)
                print_signature(*solution.type);
            else {
                std::cout << "{{{_anything}}}";
            }
            std::cout << "\n";

            return solution;
        }
    } return {{}, nullptr, true};
}





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

    signature int_type = {
        {
            {"int", {}, false}
        }, &unit_type, false};

    signature print_type = {
        {
            {"print", {}, false},
            {"", {{}, &unit_type, false}, true}
        }, &int_type, false};

    std::vector<struct signature> signatures = {nothing_type, infered_type, int_type, print_type};

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
            signature* type = &infered_type;      //&unit_type;
            size_t max_depth = 0;
            while (max_depth <= max_expression_depth) {
                std::cout << "trying depth = " << max_depth << std::endl; // debug
                pointer = 0;
                type = &infered_type;     //&unit_type;
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
