

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
