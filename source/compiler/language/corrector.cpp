//
//  corrector.cpp
//  language
//
//  Created by Daniel Rehman on 1903192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "corrector.hpp"
#include "nodes.hpp"
#include "parser.hpp"
#include "arguments.hpp"

#include "debug.hpp"



/**

nostril do file.n


use .io
print "hello, world!"



 -------------------- STAGES ------------------


parsing - corrector:

    stage 1:   indent raising (IR) phase

    stage 2:   expression to abstraction (EA) correction phase

    stage 3:   expression to variable (EV) correction phase


analysis:

    stage 4:    scope and visibility analysis (SVA) phase

    stage 5:    type inference and checking (TIC) phase

    stage 6:    namespace signature subsitution (NSS) phase

    stage 7:    call signature resolution (CSR) phase

    stage 8:    numeric value subsitution (NVS) phase


code generation:

    ...



optimization:

    ...

 stage ...:    compiletime abstraction evaluation (CAE) phase

linking:

    ...






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





 */

translation_unit correct(translation_unit unit, struct file file) {

    std::cout << "------------------- corrector: -------------------------\n";


    // note: because its top level, it cant be a function call, a cs, or a ts.
    // it can only be a prototype or a definition, or a variable assignment.

    for (auto expression : unit.list.expressions) { // for each top level symbol:


    }


    return {};
}
