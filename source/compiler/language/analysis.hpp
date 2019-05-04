//
//  analysis.hpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef analysis_hpp
#define analysis_hpp

#include "parser.hpp"
#include "nodes.hpp"

#include <vector>
#include <string>

translation_unit analyze(translation_unit tree, struct file file);

#endif /* analysis_hpp */



/*

 jobs:

 - scope checking
 - scoping and name resolution

- put in calls to calls the destructor when a variable goes out of scope!;
- - user-defined compiler (symbol table oriented) triggers for calling a function.


 -------------- useful reminder for when we need the compiler to define a bunch of abstractions for the api.


 std::vector<std::string> builtins = {

 "_visibility", "_within", "_called", "_when",


 "_scope", "_self", "_parent",


 "_caller", "_file", "_module", "_all",
 "_bring", "_import",



 "_evaluation", "_compiletime", "_runtime",
 "_precedence", "_associativity", "_left", "_right",



x:"_type", "_infered", "_none",;


x: "_after", "_before", "_inside",;

 // parse tree nodes:
 "_translation_unit",
 "_expression", "_expression_list", "_symbol",
 "_string", "_character", "_documentation", "_llvm",
 "_identifier", "_builtin",
 };*/
