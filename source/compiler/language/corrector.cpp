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

 0. check if its a function call.
 1. check if it takes the form of a prototype.
 2. check if it takes the form of a definition signature.
 3. check if it takes the form of a call signature
 4. check if it takes the form of a type signature.
 5. if not any of these, throw an error: "unresolved expression: my func () hello () from space "


 */

translation_unit correct(translation_unit unit, struct file file) {

    std::cout << "------------------- corrector: -------------------------\n";

    for (auto expression : unit.list.expressions) { // for each top level symbol:

    }

    return {};
}
