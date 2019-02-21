
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
    //if (b && statement(p) && tokens[pointer+1].type == operator_type && (tokens[pointer+1].value == "}")) return success(parent, self);
    //if (b && statement(p) && required_newlines(p)) return success(parent, self);
    //if (b && statement(p) && operator_(";")) return success(parent, self);
    return failure(save, self);
}

node parse(std::string filename, std::string text, std::vector<struct token> tokens, bool &error) {
    
    print_lex(tokens);
    node tree = {};
    
    if (!program(tokens, tree) || pointer != tokens.size() || level) {
        
        int i = 0;
        for (auto n : deepest_stack_trace) print_node(n, i++);
        print_parse(tree);
        print_parse_error(filename, tokens[deepest_pointer].line, tokens[deepest_pointer].column, deepest_node.name, tokens[deepest_pointer].type, tokens[deepest_pointer].value);
        print_source_code(text, {tokens[deepest_pointer]});
        error = true;
        
    } else {
        print_parse(tree);
        std::cout << "\n\n\tsuccess.\n\n" << std::endl;
    }
    
    return tree;
}
