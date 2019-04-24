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


















// note: the descion to not allow top level expressions was
// chosen to eleminte the burden of the compiler, having the make
// functions, which RUN BEFORE main(). that gets really compilcated and hard to
// reason about, fast, so we just disallow it. of course,
//
// unless the user says: "nostril do ..."




















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








/* -------------- useful reminder for when we need the compiler to define a bunch of abstractions for the api.


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
 "_expression", "_expression_list", "_symbol",
 "_string", "_character", "_documentation", "_llvm",
 "_identifier", "_builtin",
 };*/
