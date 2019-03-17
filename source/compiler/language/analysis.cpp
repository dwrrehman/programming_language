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

struct action_tree analyze(program tree) {

    /*

        jobs:

            - scope checking
            - type inference
            - type checking
            - ICTC checking
            - UD order chcecking
            - UD lifetime checking
            - insert implied code: destructors, ...?

        ie,

            - scoping and name resolution
            - type inference, filling in missing types
            - strong type checking, (nominative), conforming to implcit conversion type classes
            - enforcing user defined function call ordering
            - enforcing user defined lifetime rules.
            - put in calls to calls the destructor when a variable goes out of scope!


     */



    return {};
}
