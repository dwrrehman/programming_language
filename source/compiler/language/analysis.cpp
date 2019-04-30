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
#include "lists.hpp"

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <algorithm>

#include "csr.hpp"


static void print_vector(const std::vector<std::string> &N) {
    std::cout << "[";
    for (auto n : N) {
        std::cout << n << ", ";
    }
    std::cout << "]\n\n";
}



static std::vector<std::vector<std::string>> get_subranges(std::vector<std::string> v) {

    std::vector<std::vector<std::string>> subranges = {};

    for (size_t front = 0; front < v.size(); front++) {
        for (size_t back = v.size(); back > front; back--) {
            std::vector<std::string> N(v.begin() + front, v.begin() + back);
            subranges.push_back(N);
        }
    }
    std::sort(subranges.begin(), subranges.end(), [](auto i, auto j) {return i.size() > j.size();});

    return subranges;
}

static void test_subranges() {
    const std::vector<std::string> v = {"hello", "there", "from", "space"};
    auto list = get_subranges(v);
    for (const auto l : list) {
        print_vector(l);
    }
}



/*




 (a) {

 }
        <-------------------- starting here.
 (f x a) {
    x
 }

 to parse this, lets assume we already parsed (a).

 the steps are:

 init front and back.

 start with full range, as first subrange.
 csr it.

 then shift back to be closer to front.
 ie, "back--"

 then do csr again, with the resultant vector.



 */




translation_unit analyze(translation_unit tree, struct file file) {

    const std::vector<std::string> v = {"hello", "there", "from", "space"};

    for (size_t front = 0; front < v.size(); front++) {
        for (size_t back = v.size(); back > front; back--) {
            std::vector<std::string> N(v.begin() + front, v.begin() + back);
            //csr({}, {});
        }
    }

    return {};
}
