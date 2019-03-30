//
//  preprocessor.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "preprocessor.hpp"
#include "lists.hpp"
#include "error.hpp"
#include "debug.hpp"
#include "compiler.hpp"

#include <unordered_map>
#include <iostream>
#include <vector>
#include <exception>





/// this function is parser of metaprograms, only.














// simply call the "frontend()" function of compiler.hpp, when parsing the body of the metaprogram.
// we also need to set a flag in for the frontend to tell it that we are parsing a meta program.

// we also need to evauluate the pattern as well. the eval of the
// we also need to evaluate the resulting code using lli, and then do the replacement, based on the textual output from the body.




/*

 so, to really wring this out,


 1. first, we must find a macro signature "::{"
 2. next, we must parse everything up until we find a











 and then, we realized, we are working at the wrong level of abstraction.












 what we really need, is to build "::{", ":::", and "::}" into the grammar of our language.
 ie, this should be a ebnf statement, which contains translation units:

 metaprogram
 = ::{ translationunit ::: translationunit ::}

 and then a translationunit is a list of expressions, or metapograms, at the top level. (a metaprogram can only appear at the top level, because its not actually an expression.












 ie, we do not have a preprocessor in our language. we just dont. we dont need one.



 we simply have a recursive compiler!







 we will call the frontend() function, right before parsing.
 we are actually going to write a seperate parser, which makes
 a copy of the file, and then simply parses for all the macros
 in it, ignoring the rest. it then simply keeps a datastructure
 which is called "macro" which will have two strings,
 the pattern n code, and the body n code.

 we then, in trying to parse the given file,

 will first simply parse the macro.pattern_n_code, using the FRONTEND(),
 then execute the resulting llvm ir, and see what the resultant pattern string is.

 then, we will simply parse the macro.body_n_code, using the FRONTEND(),
 then execute the resultign llvm ir, and see what the resultant body string is.





 and so we need:

 struct macro {
 std::string pattern = "";
 std::string body = "";
 };


 or, in n3zqx2l

 (macro)
 _ pattern = ""
 _ body = ""















 our semantic analyzer is the one which will actually detect a metaprogram in our language,





 */



/**

 (this is how you would write a "macro" (metaprogram) in n3zqx2l:)


 ; this is a metaprogram:

 ::{
 ; this is the pattern of the metaprogram.
 :::
 ; this is the body of the metaprogram.
 ::}


 */


struct preprocessed_file preprocess(struct file file) {

    return {};
}



void macro_replace(std::string pattern, std::string body, std::string &text) {

}
