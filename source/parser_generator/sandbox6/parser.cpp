

/// Header for computer generated parser for boogers.

class postinformation {
public:
    
    postinformation(){}
};

/// Parser AST nodes:

class pp_node {
public:
    
    std::string name = "";
    
    bool success = false;
    struct pp_token data = {pp_null_type, "", 0, 0};
    postinformation post = {};
    std::vector<pp_node> children = {};
    
    pp_node(std::string name, struct pp_token data, std::vector<pp_node> children, bool success) {
        this->name = name;
        this->children = children;
        this->success = success;
        this->data = data;
    }
    pp_node(){}
};

class parse_error {
public:
    
    std::string expected = "";
    struct pp_token at = {pp_null_type, "",  0, 0};
    parse_error(std::string expected, struct pp_token data) {
        this->expected = expected;
        this->at = data;
    }
    parse_error(){}
};

class program parse(std::string filename, std::string text, std::vector<struct token> tokens, bool &error);
void print_node(pp_node &node, int level);



#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

static void print_pp_node(pp_node &self, int level) {
    prep(level); std::cout << self.name << " (" << self.children.size() << ")" << std::endl;
    if (self.data.type != pp_null_type) {
        prep(level); std::cout << "type = " << convert_pp_token_type_representation(self.data.type) << std::endl;
    }
    if (self.data.value != "") {
        prep(level); std::cout << "value = " << self.data.value << std::endl;
    }
    int i = 0;
    for (auto childnode : self.children) {
        std::cout << std::endl;
        if (self.children.size() > 1) {prep(level+1); std::cout << "child #" << i++ << ": " << std::endl;}
        print_pp_node(childnode, level+1);
    }
}

static void print_pp_token(struct pp_token t) {
    std::cout << "Error at token: \n\n";
    std::cout << "\t\t---------------------------------\n";
    std::cout << "\t\tline " << t.line << "," << t.column << " : "<< t.value << "           "  <<  "(" << convert_pp_token_type_representation(t.type) << ")\n";
    std::cout << "\t\t---------------------------------\n\n\n";
}

static void print_pp_parse(pp_node &tree) {
    std::cout << "------------ PARSE: ------------- " << std::endl;
    print_pp_node(tree, 0);
}


/// Parsing:

static int pointer = 0;
static std::vector<pp_node> stack_trace = {};

static int deepest_pointer = 0;
static pp_node deepest_node = {};
static std::vector<pp_node> deepest_stack_trace = {};

size_t deepest_level = 0;
size_t level = 0;

#define declare_node()    pp_node self = pp_node(__func__, {}, {}, true);                       \
int save = pointer;                                             \
stack_trace.push_back(self);                                    \
level++;                                                        \

#define params            std::vector<struct pp_token> tokens, pp_node &parent
#define p                 tokens, self

#define optional() level--; stack_trace.pop_back(); return begin(save, self);

#define keyword_(kw)      terminal(pp_keyword_type, kw, p)

#define b                 begin(save, self)

static bool begin(int save, pp_node &self) {
    if (level > deepest_level) {
        deepest_level = level;
        deepest_node = self;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer < pointer)
        deepest_pointer = pointer;
    pointer = save;
    self.children.clear();
    return true;
}

static bool failure(int save, pp_node &self) {
    pointer = save;
    self.children.clear();
    level--;
    stack_trace.pop_back();
    return false;
}
static bool success(pp_node &parent, const pp_node &self) {
    parent.children.push_back(self);
    level--;
    stack_trace.pop_back();
    return true;
}
static bool push_terminal(pp_node &parent, std::vector<struct pp_token> &tokens) {
    parent.children.push_back(pp_node("terminal", tokens[pointer++], {}, true));
    if (level > deepest_level) {
        deepest_level = level;
        deepest_node = parent;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer < pointer)
        deepest_pointer = pointer;
    
    return true;
}

static bool terminal(enum pp_token_type desired_token_type, std::string desired_token_value, params) {
    if (level > deepest_level) {
        deepest_level = level;
        deepest_node = parent;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer < pointer)
        deepest_pointer = pointer;
    
    if (pointer >= tokens.size()) return false;
    if (desired_token_type == pp_identifier_type && tokens[pointer].type == pp_identifier_type) return push_terminal(parent, tokens);
    if (desired_token_type == pp_text_type && tokens[pointer].type == pp_text_type) return push_terminal(parent, tokens);
    
    
    if (desired_token_type == tokens[pointer].type && desired_token_value == tokens[pointer].value) return push_terminal(parent, tokens);
    return false;
}

/// Hand made EBNF nodes interfaces:

bool identifier(params);

bool raw_text(params);


//this is a automatically generated parser in cpp, for my language.


bool program(params);

bool macro_list(params);

bool macro(params);

bool pattern(params);

bool pattern_element(params);

bool replacement(params);

bool statement_list(params);

bool statement(params);

bool block_statement(params);

bool if_statement(params);

bool let_statement(params);

bool assignment_statement(params);

bool emit_statement(params);

bool while_statement(params);

bool function_definition(params);

bool parameter_list(params);

bool parameter(params);

bool function_call(params);

bool expression_list(params);

bool expression(params);

bool and_expr(params);

bool compare_expr(params);

bool sum_expr(params);

bool product_expr(params);

bool unary_expr(params);

bool symbol(params);

bool number(params);




//EBNF Parse Nodes:

bool program(params) {
	declare_node();
	if (b && raw_text(p) && macro_list(p)) return success(parent, self);
	if (b && macro_list(p)) return success(parent, self);
	return failure(save, self);
}

bool macro_list(params) {
	declare_node();
	if (b && macro(p) && raw_text(p) && macro_list(p)) return success(parent, self);
	if (b && macro(p) && macro_list(p)) return success(parent, self);
	optional();
}

bool macro(params) {
	declare_node();
	if (b && keyword_("replace") && pattern(p) && keyword_("with") && replacement(p) && keyword_("end")) return success(parent, self);
	return failure(save, self);
}

bool pattern(params) {
	declare_node();
	if (b && pattern_element(p) && pattern(p)) return success(parent, self);
	optional();
}

bool pattern_element(params) {
	declare_node();
	if (b && raw_text(p)) return success(parent, self);
	if (b && identifier(p)) return success(parent, self);
	return failure(save, self);
}

bool replacement(params) {
	declare_node();
	if (b && raw_text(p) && statement_list(p)) return success(parent, self);
	if (b && statement_list(p)) return success(parent, self);
	optional();
}

bool statement_list(params) {
	declare_node();
	if (b && statement(p) && raw_text(p) && statement_list(p)) return success(parent, self);
	if (b && statement(p) && statement_list(p)) return success(parent, self);
	optional();
}

bool statement(params) {
	declare_node();
	if (b && if_statement(p)) return success(parent, self);
	if (b && let_statement(p)) return success(parent, self);
	if (b && assignment_statement(p)) return success(parent, self);
	if (b && while_statement(p)) return success(parent, self);
	if (b && block_statement(p)) return success(parent, self);
	if (b && emit_statement(p)) return success(parent, self);
	if (b && function_definition(p)) return success(parent, self);
	if (b && function_call(p) && keyword_("end")) return success(parent, self);
	if (b && expression(p) && keyword_("end")) return success(parent, self);
	return failure(save, self);
}

bool block_statement(params) {
	declare_node();
	if (b && keyword_("begin") && replacement(p) && keyword_("end")) return success(parent, self);
	return failure(save, self);
}

bool if_statement(params) {
	declare_node();
	if (b && keyword_("if") && keyword_("(") && expression(p) && keyword_(")") && keyword_("do") && replacement(p) && keyword_("else") && replacement(p) && keyword_("end")) return success(parent, self);
	return failure(save, self);
}

bool let_statement(params) {
	declare_node();
	if (b && keyword_("let") && identifier(p) && keyword_("=") && expression(p) && keyword_("end")) return success(parent, self);
	if (b && keyword_("let") && keyword_("int") && identifier(p) && keyword_("=") && expression(p) && keyword_("end")) return success(parent, self);
	return failure(save, self);
}

bool assignment_statement(params) {
	declare_node();
	if (b && identifier(p) && keyword_("=") && expression(p) && keyword_("end")) return success(parent, self);
	return failure(save, self);
}

bool emit_statement(params) {
	declare_node();
	if (b && keyword_("emit") && identifier(p) && keyword_("end")) return success(parent, self);
	return failure(save, self);
}

bool while_statement(params) {
	declare_node();
	if (b && keyword_("while") && keyword_("(") && expression(p) && keyword_(")") && keyword_("do") && replacement(p) && keyword_("end")) return success(parent, self);
	return failure(save, self);
}

bool function_definition(params) {
	declare_node();
	if (b && keyword_("define") && identifier(p) && keyword_("(") && parameter_list(p) && keyword_(")") && block_statement(p)) return success(parent, self);
	return failure(save, self);
}

bool parameter_list(params) {
	declare_node();
	if (b && parameter(p) && keyword_(",") && parameter_list(p)) return success(parent, self);
	if (b && parameter(p)) return success(parent, self);
	optional();
}

bool parameter(params) {
	declare_node();
	if (b && keyword_("let") && keyword_("int") && identifier(p)) return success(parent, self);
	if (b && keyword_("let") && identifier(p)) return success(parent, self);
	return failure(save, self);
}

bool function_call(params) {
	declare_node();
	if (b && keyword_("call") && identifier(p) && keyword_("(") && expression_list(p) && keyword_(")")) return success(parent, self);
	return failure(save, self);
}

bool expression_list(params) {
	declare_node();
	if (b && expression(p) && keyword_(",") && expression_list(p)) return success(parent, self);
	if (b && expression(p)) return success(parent, self);
	optional();
}

bool expression(params) {
	declare_node();
	if (b && and_expr(p) && keyword_("|") && and_expr(p)) return success(parent, self);
	if (b && and_expr(p)) return success(parent, self);
	return failure(save, self);
}

bool and_expr(params) {
	declare_node();
	if (b && compare_expr(p) && keyword_("&") && compare_expr(p)) return success(parent, self);
	if (b && compare_expr(p)) return success(parent, self);
	return failure(save, self);
}

bool compare_expr(params) {
	declare_node();
	if (b && sum_expr(p) && keyword_("==") && sum_expr(p)) return success(parent, self);
	if (b && sum_expr(p) && keyword_("<") && sum_expr(p)) return success(parent, self);
	if (b && sum_expr(p) && keyword_(">") && sum_expr(p)) return success(parent, self);
	if (b && sum_expr(p)) return success(parent, self);
	return failure(save, self);
}

bool sum_expr(params) {
	declare_node();
	if (b && product_expr(p) && keyword_("+") && product_expr(p)) return success(parent, self);
	if (b && product_expr(p) && keyword_("-") && product_expr(p)) return success(parent, self);
	if (b && product_expr(p)) return success(parent, self);
	return failure(save, self);
}

bool product_expr(params) {
	declare_node();
	if (b && unary_expr(p) && keyword_("*") && unary_expr(p)) return success(parent, self);
	if (b && unary_expr(p) && keyword_("/") && unary_expr(p)) return success(parent, self);
	if (b && unary_expr(p)) return success(parent, self);
	return failure(save, self);
}

bool unary_expr(params) {
	declare_node();
	if (b && keyword_("!") && symbol(p) && symbol(p)) return success(parent, self);
	if (b && symbol(p)) return success(parent, self);
	return failure(save, self);
}

bool symbol(params) {
	declare_node();
	if (b && keyword_("(") && expression(p) && keyword_(")")) return success(parent, self);
	if (b && number(p)) return success(parent, self);
	if (b && function_call(p)) return success(parent, self);
	if (b && identifier(p)) return success(parent, self);
	if (b && raw_text(p)) return success(parent, self);
	return failure(save, self);
}

bool number(params) {
	declare_node();
	if (b && keyword_("int") && identifier(p)) return success(parent, self);
	if (b && keyword_("int") && keyword_("+") && identifier(p)) return success(parent, self);
	if (b && keyword_("int") && keyword_("-") && identifier(p)) return success(parent, self);
	return failure(save, self);
}


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

