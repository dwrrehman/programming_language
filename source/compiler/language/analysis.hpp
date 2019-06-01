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

std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file);

struct data {
    struct file file;
    llvm::Module* module;
    llvm::Function* function;
    llvm::IRBuilder<>& builder;     
};

struct flags {
    bool should_allow_undefined_signatures = false;    
    bool should_generate_code = false;
    bool is_at_top_level = false; 
    bool is_parsing_type = false;
    
    struct flags& allow_undefined() { should_allow_undefined_signatures = true; return *this; }
    struct flags& dont_allow_undefined() { should_allow_undefined_signatures = false; return *this; }    
    struct flags& generate_code() { should_generate_code = true; return *this; }
    struct flags& dont_generate_code() { should_generate_code = false; return *this; }    
    struct flags& at_top_level() { is_at_top_level = true; return *this; }
    struct flags& not_at_top_level() { is_at_top_level = false; return *this; }    
    struct flags& parsing_a_type() { is_parsing_type = true; return *this; }    
    struct flags& not_parsing_a_type() { is_parsing_type = false; return *this; }
    
    flags(bool given_should_allow_undefined_signatures = false,
          bool given_should_generate_code = false, 
          bool given_is_at_top_level = false,
          bool given_is_parsing_type = false
          ):  
    should_allow_undefined_signatures(given_should_allow_undefined_signatures), 
    should_generate_code(given_should_generate_code), 
    is_at_top_level(given_is_at_top_level), 
    is_parsing_type(given_is_parsing_type) {}
    
}; 

using stack = std::vector<std::vector<expression>>;

expression csr(
               const expression& given, expression*& expected, expression& fdi,               
               const size_t depth, const size_t max_depth, size_t& pointer,               
               stack& stack, data& data, flags flags, bool& error);
void adp(abstraction_definition& given, stack& stack, data& data, flags flags, bool& error);
expression res(expression given, expression& expected_type, stack& stack, data& data, flags flags, bool& error);

#endif /* analysis_hpp */



/*

 jobs:

 - scope checking
 - scoping and name resolution

- put in calls to calls the destructor when a variable goes out of scope!;
- - user-defined compiler (symbol table oriented) triggers for calling a function.


- ie, the compiler should make everything have a constructor and destructor. (even integers? yes they simply get push and popped off the stack.)

 - ie, we really should be making our compiler care about when a object goes out of scope, delete it.

 however, because we want to be able to transfer ownership, we need to essentially say that:

    v = [1,2,3,4]        ; the compiler should be able to infer that this is a vector.

                        ; based on the signature, appears to return (), defines a


 heres the signature of that function.

 (((sig) _signature) = ((v) vector)) () {}

 (vector) {
    (public) {
        _define _caller
    }
    () public _parent {

    }
 }




















 ok. it seems that this whole _level# and _scope# thing are actually just special cases of a very powerful function call, callled:

 "f"


so, basically, the idea is that you will be able to specify a scope, relative to a abstraction level, and this is how you will be able to specify any scope/abstraction.




 so the idea is that when the user writes:


                _level1                ; ie, the parent abstraction,



 what they really mean is:




                f abs=1 scope=0                     ;ie, the scope of the abstraction.











 but wait.... thats distinctly not what we mean.....







 maybe if we are trying to get a scope realtive to an abstraction,



 but when we speicfy a _parent or _self, what we really want isnt a scope.   its an abstraction signature.



 thats the difference.





 ie, we might say, there are two functions,


    one which gives you an abstraction, when you give it:           _scope, _abstraction

 and another, which gives you a scope, when you give it:            _abstraction, _scope


 interesting huh? lets call these:








                _abstraction 0 1        ; the current scope's parent.

                _scope 1 0              ; the current scope of the parent.




    so, we see that these are actually quite different beasts.


    most notably, these two functions actually correspond to the scopage of the two core fundmental
        aspects of n3zqx2l, namely;


            - abstraction : code
and
            - application : data



 so, lets actually make it so the signature names reflect that:


                _abstraction 0 1              ; the current scope's parent.

                _application 1 0              ; the current scope of the parent.


    ah yes. thats better.









































TODO:
           add in a new builtin:



                        _recognize <_signature>                                      ; runs at compile time.





        note, this is how we are going to do compiletime turing-complete signatures. using the return type as our mechnaism for parameter types.













 -------------- useful reminder for when we need the compiler to define a bunch of abstractions for the api.











 std::vector<std::string> builtins = {

 "_evaluation", "_compiletime", "_runtime",

 "_precedence", "_associativity",

 "_expression",
  "_llvm_string",

 };




 */
