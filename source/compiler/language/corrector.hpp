//
//  corrector.hpp
//  language
//
//  Created by Daniel Rehman on 1903192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef corrector_hpp
#define corrector_hpp

#include "parser.hpp"
#include "nodes.hpp"

translation_unit correct(translation_unit unit, struct file file);

#endif /* corrector_hpp */

/**

 -------------------- STAGES ------------------

 parsing - corrector:

 x:stage 1:   indent level raising (ILR) indent correction phase;

 x:stage 2    turn indent to block transformations (TIB) indent correction phase;

           x: stage 3:   expression to abstraction definition (ETA) correction phase;

           x: stage 4:   expression to variable definition (ETV) correction phase;



x: possible stage 5:      turn top level abstraction call expression(s (of form: subexpr, stuff, block) into abstraction definitions. (they cant possibly be a fucntion call, becuase its the top level.
 aka, top level expression to abstraction definition:      (TEA);

        -> this is because only abstraction definitions/declarations can appear at the top level.


 analysis:

 stage 5:    scope and visibility analysis (SVA) phase

 stage 6:    namespace signature subsitution (NSS) phase

                            x:stage 7:    type inference and checking (TIC) phase;

                            x:stage 8:    call signature resolution (CSR) phase;

 stage 9:    numeric value subsitution (NVS) phase








 post analysis:

 stage 10:   wrapper type expansion (WTE) phase

 stage 11:   signature ABI transformation (SAT) phase



 
 code generation:

 CGN



 optimization:

 OPT



 stage ...:    compiletime abstraction evaluation (CAE) phase

 linking:

 LNK







 ------- HOW TO DO EXP_TO_ABS (ETA stage) -------

 0. check if its a function call.
 1. check if it takes the form of a prototype.
 2. check if it takes the form of a definition signature.
 3. check if it takes the form of a call signature
 4. check if it takes the form of a type signature.
 5. if not any of these, throw an error: "unresolved expression: my func () hello () from space "




 note: everything is assumed to be a function call at first.

 characteristics of each:


 0. a function call will never have any colons. thats the best we can do rright now.

 1. a prototype will always have a colon, at least for the signature type.
 1. it may also have colons in the call signature portion.

 2. it will always either have a block, or









 what we need to allow for is that:

 x: (a b) c = (x: a y: b) c : _runtime {


 the algorithm of spotting abstraction definitions is as so:


 note: we always need to allow for variables
 to be on their own line, alwaus.

 heres a note though:


 we only need to mak abstractions alloweed to be in
 expressions (in the middleof expressions)


 variables cant be in the middle of expressions,
 they are always on their own line.

 another reason we need to allow for abstraction definitions
 (but not prototypes) to be anywwehere in a function definition,
 is for when you want to pass a user defined
 lambda into a another function call.


 heres another idea:


 the algorithm for this is as follows:


 walk a expression:


 if the current symbol is a subexpression,

 then walk untill you find the first block.
 then, everything from the subexpression and the block mightttt be
 a abstraction definition, IF AND ONLY IF there is a colon between
 the two end points.



 heres a question:


 do we need variables as their own statement, always?


 i think so, actually. this is because of the nature of the variable definition:


 its always an expression, followed by a colon, followed by a ;

 however there are some restrictions on what you can have on the left hand side: (ie, the identifier side)

 these restrictions are:

 no strings of any kinds:
 - no doc strings, no string literals, no llvm strings, and no character strings.

 this is mandatory, i think.

 although... why? why not have those things...?  we will see, i guess.

 you cannot have parenthesis on this side. if you do, then you are actually defining an abstraction.


 however, there are some interesting cavieats.

 if you do something like this:      (this is allowable:)


 (f): (c) = (my func) {
 ; body here
 }

 note: that "f" here, is a abstraction, but is being defined inside of a variable definition.



 this is because a variable can be "of type \"abstraction\""



 however, because a abstraction can take on multiple forms:


 (x) c : asdf {        ; very easy, this is probably what we will implement first.
 print hi
 }

 (x): asdf
 print hi           ; also very easy.


 (x) c {                ; this one is impossible, until after CSR.

 }

 (x)                            ; this one is pretty difficult, but not after csr.
 print hi



 IMPOSSIBLE:

 (x) print hi                    ; this is actually impossible. like actually.

 (x): print hi                  ; this is impossible, actually. for functions, the block cannot be implied, because its right next to an expression, the type signature.

 (x) c : asdf print hi          ; this is impossible for the same reason


 */
