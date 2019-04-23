//
//  analysis.cpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "analysis.hpp"
#include "parser.hpp"
#include "nodes.hpp"

#include <iostream>
#include <vector>
#include <stdlib.h>



///TODO: remove me, and simply look for me when you are resolving call signatures.
std::vector<std::string> builtins = {
    "_visibility", "_within", "_called", "_when",
    "_none", "_scope", "_self", "_parent",

    "_type", "_infered"
    "_caller", "_file", "_module", "_all",
    "_bring", "_import",

    "_evaluation", "_compiletime", "_runtime",
    "_precedence", "_associativity", "_left", "_right",

    "_after", "_before", "_inside",

    // parse tree nodes:
    "_translation_unit",
    "_expression", "_expression_list", "_symbol", "block",
    "_string", "_character", "_documentation", "_llvm",
    "_identifier", "_builtin"
};


struct action_tree analyze(translation_unit tree, struct file file) {

    /*
        jobs:
            - scope checking
            - type inference
            - type checking
            - UD sig-order chcecking
            - UD sig-lifetime checking
            - insert implied code: destructors, ...?

        ie,

            - scoping and name resolution
            - type inference, filling in missing types
            - strong type checking, (nominative), conforming to x:implcit conversion type classes;
            - enforcing user defined function call ordering
            - enforcing user defined lifetime rules.
            - put in calls to calls the destructor when a variable goes out of scope!

     */


    return {};
}
