
node parse(std::vector<struct token> tokens, bool &error) {
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
        
        std::cout << "current pointer:" << std::endl;
        print_token(tokens[pointer]);
        
        std::cout << "Expected a \"" << deepest_node.name << "\", Found a \"" << tokens[deepest_pointer - 1].value;
        std::cout << "\", of type: " << convert_token_type_representation(tokens[deepest_pointer - 1].type) << std::endl;
        
        std::cout << "\ndeepest pointer: (raw)" << std::endl;
        if (deepest_pointer - 1 >= tokens.size()) {
            std::cout << "...couldnt print unexpected found token. " << std::endl;
            std::cout << "deepest pointer = " << deepest_pointer - 1<< std::endl;
            std::cout << "token count = " << tokens.size() << std::endl;
        } else print_token(tokens[deepest_pointer - 1]);
        
        std::cout << "\n\n\n\n";
        std::cout << "current level: " << level << "\n";
        std::cout << "last error: on level " << deepest_level << "\n";
        print_node(deepest_node, 0);
        
        error = true;
    }
    
    return tree;
}
