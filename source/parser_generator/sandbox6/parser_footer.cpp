
/// Hand made EBNF nodes:

bool identifier(params){
    declare_node();
    if (begin(save, self) && terminal(pp_identifier_type, "", p)) return success(parent, self);
    return failure(save, self);
}

bool raw_text(params){
    declare_node();
    if (begin(save, self) && terminal(pp_text_type, "", p)) return success(parent, self);
    return failure(save, self);
}

class pp_node pp_parser(std::string filename, std::vector<struct pp_token> tokens, bool &error) {
    
    pp_node tree = {};
    
    if (!program(tokens, tree) || pointer != tokens.size() || level) {
        
        int i = 0;
        for (auto n : deepest_stack_trace) print_pp_node(n, i++);
        print_pp_parse(tree);
        print_parse_error(filename, tokens[deepest_pointer].line,  tokens[deepest_pointer].column, convert_pp_token_type_representation(tokens[deepest_pointer].type), tokens[deepest_pointer].value);
        //print_source_code(text, {tokens[deepest_pointer]});
        error = true;
        
    } else {
        print_pp_parse(tree);
        std::cout << "\n\n\tsuccess.\n\n" << std::endl;
    }
    
    return tree;
}

