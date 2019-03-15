//
//  preprocessor.hpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef preprocessor_hpp
#define preprocessor_hpp

#include <string>

// --------------- preprocessor structures ----------------------------

enum pp_token_type {
    pp_null_type,
    pp_keyword_type,
    pp_text_type,
    pp_identifier_type,
    pp_ast_node_type
};

struct pp_token {
    enum pp_token_type type;
    std::string value;
    size_t line;
    size_t column;
};


std::string preprocess(std::string filename, std::string text);

#endif /* preprocessor_hpp */
