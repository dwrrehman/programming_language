//
//  analysis.cpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <vector>
#include <stdlib.h>


#include "analysis.hpp"
#include "parser.hpp"







// note, that we have a very interesting garentee with our lang,

// we will only ever encounter a call to a function which we've already parsed its definition.

// this makes things fairly easy to determine if a identifier X, is part of a function signature or not...
// we just need to check all


/**
 
 the idea is that the revise_function_signatures_based_on definiions() function should possible be ran more than once?
 
 
 it seems that we have to run this for howwever many function calls we have.
 
 
 
 for i in function signatures,
    revise_function_signatures_based_on_definitions
 
 
 and then each iteration, we would gradually find
 
 
 
 
 
 
 
 
 
 however, this can possibly be speeded up, if we find a call to a function which seem to have 2 parameters, "add x to y"
 
 but if we see a call to it in a function later below it, which is something like "add 5 to 6"
 
 then we can actually feed that back into the syste, by saying, essentially, that x might be a ___canidate___ for a parameter. this might make the search laittle better, sorta like a heuristic
 
 
 
 
 
 
 
 
 now, i think the most important= thing i see from this, is that we need a EXPRESSION PARSER,
 
 and i mean really. like one that can handle math, and other other operators, and stuff like that.
 
 
 
 
 
 
 side note: the precedence of a function CANNOT go below 0.
 
 
 
 
 
 lets write the expression parser now!
 
 
 
 
 it takes as input, a stream of identifiers, and converts them into a expression.
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 */







std::vector<struct function_signature> function_signatures = {};
//std::vector<struct type_signature> type_signatures = {};
//std::vector<struct signature> kind_signatures = {};


// Debuggers:

void debug_node_list(const std::vector<node> &list) {
    printf("\n\n\n\n\n\nprinting the node-list:\n\n");
    for (auto n : list) {
        print_node(n, 0);
    }
    printf("\n\n\n\n\n\n\n\n");
}

void debug_children_names(const node &child) {
    for (auto child : child.children) {
        std::cout << "name: " << child.name;
        if (child.name == "terminal") {
            std::cout << " :: " << child.data.value;
        }
        std::cout << std::endl;
    }
}

void print_stringarray(const std::vector<std::string> list) {
    std::cout << "[";
    for (auto s : list) std::cout << s << ", ";
    std::cout << "]" << std::endl;
}

void print_function_signature(struct function_signature sig) {
    std::cout << "------------- function signature: ---------------\n";
    
    std::cout << "is main? " << (sig.is_main ? "yes" : "no") << "\n";
    
    std::cout << "precedence: " << sig.precedence << "\n";
    
    std::cout << "call signature: (";
    for (auto element : sig.call_signature) {
        std::cout << element.name;
        if (element.is_parameter) {
            if (element.type)
                std::cout << ": " << element.type->name;
            else
                std::cout << ":_";
        }
        std::cout << ", ";
    }
    std::cout << ")\n";
    
    std::cout << "qualifier list: ";
    print_stringarray(sig.qualifiers);
    
    if (sig.return_type) {
        std::cout << "return type: " << sig.return_type->name << std::endl;
        std::cout << "heres the full return type node: \n\n--------------------\n";
        print_node(*sig.return_type, 0);
        std::cout << "-------------------\n";
    }
    
    std::cout << "\n\n\n\n";
}


void flatten_parameter_list(node &n, std::vector<node> &list) {
    if (n.name == "parameter_declaration") list.push_back(n);
    else for (auto child : n.children) flatten_parameter_list(child, list);
}

void flatten_qualifier_list(node &n, std::vector<node> &list) {
    if (n.name == "qualifier") list.push_back(n);
    else for (auto child : n.children) flatten_qualifier_list(child, list);
}

void parse_all_expressions(node &n, std::vector<node> &list) {
                
    if (n.name == "expression") {
        std::cout << "we found an EXPRESSION!!!" << std::endl;
        print_node(n, 0);
        
        if (!n.post.expression_has_been_parsed)
            n.post.expression = parse_expression(n, function_signatures);
        
        return;
    }
        
    for (auto child : n.children) {
        parse_all_expressions(child, list);
    }
}

void parse_function_signature(node &n) {
    
    if (n.name == "function_signature") {
        
        struct function_signature new_signature = {{}, {}, {}, false, 0};
        
        for (auto child : n.children) {
            
            if (child.name == "function_call_signature") {
                std::vector<node> list = {};
                flatten_parameter_list(n, list);
                for (auto n : list) {
                    struct signature_element element = {n.children[0].children[0].data.value, false, {}};
                    if (n.children.size() > 1) {
                        element.is_parameter = true;
                        element.type = new node(n.children[2]);
                    }
                    new_signature.call_signature.push_back(element);
                }
                
            } else if (child.name == "return_type_signature") {
                new_signature.return_type = new node(child.children[2]);
                                
            } else if (child.name == "precedence_signature") {
                new_signature.precedence = atoi(child.children[1].data.value.c_str());                
                
            } else if (child.name == "qualifier_list") {
                std::vector<node> list = {};
                flatten_qualifier_list(n, list);
                for (auto q : list)
                    new_signature.qualifiers.push_back(q.children[0].data.value);
            }
        }
        
        if (new_signature.call_signature.size() == 1
            && new_signature.call_signature[0].name == "main"
            && new_signature.call_signature[0].is_parameter == false) {
            new_signature.is_main = true;
        }
        
        function_signatures.push_back(new_signature);
        
    } else for (auto child : n.children) parse_function_signature(child);
}

void revise_signatures_based_on_definition(node tree) {
    if (tree.name == "function_implementation_declaration") {
        std::vector<node> list = {};
        parse_all_expressions(tree.children[1].children[0].children[1], list); // accesses the statementlist block in the function def.
        std::cout << "heres our results! ::";
        debug_node_list(list);
    }
    
    for (auto child : tree.children)
        revise_signatures_based_on_definition(child);
}

void find_and_construct_function_signatures(node tree) {
    
    if (tree.name == "function_implementation_declaration" || tree.name == "function_interface_declaration") {        
        parse_function_signature(tree);
    }
    
    for (auto child : tree.children)
        find_and_construct_function_signatures(child);
}

node analyze(node tree, bool &error) {
    
    find_and_construct_function_signatures(tree);
    
    std::sort(function_signatures.begin(), function_signatures.end() , [](struct function_signature first, struct function_signature second){return first.precedence > second.precedence;});
    
    for (auto sig : function_signatures)
        revise_signatures_based_on_definition(tree);

    
    for (auto sig : function_signatures)
        print_function_signature(sig);
    
    return {};
}
