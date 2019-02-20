

/// Hand made EBNF nodes:

bool identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    return failure(save, self);
}

bool number(params){
    declare_node();
    if (begin(save, self) && terminal(number_type, "", p)) return success(parent, self);
    return failure(save, self);
}

bool string(params){
    declare_node();
    if (begin(save, self) && terminal(string_type, "", p)) return success(parent, self);
    return failure(save, self);
}

bool free_identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    for (auto keyword : overridable_keywords) {
        if (begin(save, self) && terminal(keyword_type, keyword, p)) return success(parent, self);
        if (begin(save, self) && terminal(operator_type, keyword, p)) return success(parent, self);
    }
    return failure(save, self);
}

bool type_free_identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    for (auto keyword : overridable_keywords) {
        if (keyword == "{" || keyword == "}") continue;
        if (b && terminal(keyword_type, keyword, p)) return success(parent, self);
        if (b && terminal(operator_type, keyword, p)) return success(parent, self);
    }
    return failure(save, self);
}

bool kind_free_identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    for (auto keyword : overridable_keywords) {
        if (keyword == "[" || keyword == "]") continue;
        if (b && terminal(keyword_type, keyword, p)) return success(parent, self);
        if (b && terminal(operator_type, keyword, p)) return success(parent, self);
    }
    return failure(save, self);
}

bool qualifier(params) {
    declare_node();
    for (auto qualifier : qualifiers)
        if (b && keyword_(qualifier)) return success(parent, self);
    return failure(save, self);
}

bool required_newlines(params) {
    declare_node();
    if (begin(save, self) && operator_("\n") && required_newlines(p)) return success(parent, self);
    if (begin(save, self) && operator_("\n")) return success(parent, self);
    return failure(save, self);
}

bool newlines(params) {
    declare_node();
    if (begin(save, self) && operator_("\n") && newlines(p)) return success(parent, self);
    optional();
}

bool documentation(params) {
    declare_node();
    if (begin(save, self) && terminal(documentation_type, "", p) && newlines(p)) return success(parent, self);
    optional();
}


bool terminated_statement(params) {
    declare_node();
    if (b && statement(p) && tokens[pointer+1].type == operator_type && (tokens[pointer+1].value == "}")) return success(parent, self);
    if (b && statement(p) && required_newlines(p)) return success(parent, self);
    if (b && statement(p) && operator_(";")) return success(parent, self);
    return failure(save, self);
}

node parse(std::string text, std::vector<struct token> tokens, bool &error) {
    print_lex(tokens);
    node tree = {};
    bool parse_successful = false;
    
    if (!program(tokens, tree) || pointer != tokens.size()) {
        std::cout << "Error: parsing failed." << std::endl;
        
        int i = 0;
        for (auto n : deepest_stack_trace) {
            print_node(n, i++);
        }
        
    } else parse_successful = true;
    
    print_parse(tree);
    std::cout << "level = " << level << "\n";
    
    if (parse_successful && !level) {
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "\n\n\n\t\tsuccessfully parsed.\n\n\n" << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
    } else {
        std::cout << "\n\n\n\t\tPARSE FAILURE.\n\n\n\n" << std::endl;
        
        std::cout << "Expected a \"" << deepest_node.name << "\", Found a \"" << tokens[deepest_pointer - 1].value;
        std::cout << "\", of type: " << convert_token_type_representation(tokens[deepest_pointer - 1].type) << std::endl << std::endl;
        
        auto & t = tokens[deepest_pointer - 1];
        std::vector<int> offsets = {-2, -1, 0, 1, 2};
        std::string line = "";
        std::istringstream s {text};
        std::vector<std::string> lines = {};
        while (std::getline(s, line)) lines.push_back(line);
        
        for (auto offset : offsets) {
            
            size_t index = 0;
            if ((int) t.line - 1 + offset >= 0 && (int) t.line - 1 + offset < lines.size()) {
                index = t.line - 1 + offset;
            } else continue;
            
            std::cout << "\t" << GRAY << t.line + offset << RESET GREEN "  |  " RESET << lines[index] << std::endl;
            
            if (!offset) {
                std::cout << "\t";
                for (int i = 0; i < t.column + 5; i++) std::cout << " ";
                std::cout << BRIGHT_RED << "^";
                if (t.value.size() > 1) for (int i = 0; i < t.value.size() - 1; i++) std::cout << "~";
                std::cout << RESET << std::endl;
            }
        }
        
        std::cout << std::endl;
        error = true;
    }
    
    return tree;
}
