#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct expression {
    size_t value;
    size_t count;
    struct expression* symbols;
    uint32_t line;
    uint32_t column;
};

struct resolved {
    size_t id;
    size_t value;
    size_t count;
    struct resolved* arguments;
};

struct name {
    size_t length;
    struct resolved* signature;
    struct resolved type;
    struct resolved definition;
};

struct frame {
    size_t count;
    size_t* indicies;
};

struct context {
    size_t at;
    size_t best;
    size_t frame_count;
    struct frame* frames;
    size_t name_count;
    struct name* names;
    const char* filename;
    size_t Unused;
};

static char* open_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "n: %s: ", filename); perror("error: ");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    const size_t length = ftell(file);
    char* buffer = (char*) calloc(length + 1, sizeof(char));
    fseek(file, 0, SEEK_SET);
    fread(buffer, sizeof(char), length, file);
    fclose(file);
    return buffer;
}

static struct expression parse(const char* filename, const char* text, size_t* at, uint32_t* line, uint32_t* column) {
    struct expression result = {0};
    while (text[*at] && text[*at] != ')') {
        if (text[*at] == '\n') { ++*line; *column = 1;} else ++*column;
        if (isspace(text[(*at)++])) continue;
        result.symbols = realloc(result.symbols, sizeof(struct expression) * (result.count + 1));
        if (text[*at - 1] == '(') {
            uint32_t l = *line, c = *column - 1;
            result.symbols[result.count++] = parse(filename, text, at, line, column);
            if (!text[*at] || text[(*at)++] != ')') fprintf(stderr, "n3zqx2l: %s:%u:%u: error: expected )\n\n", filename, l, c); ++*column;
        } else result.symbols[result.count++] = (struct expression){text[*at - 1], 0, 0, *line, *column - 1};
    } return result;
}

static void represent(struct resolved given, char* buffer, size_t limit, size_t* at, struct context* context) {
    struct name name = context->names[given.id];
    for (size_t i = 0, a = 0; i < name.length; i++) {
        if (*at + 2 >= limit) return;
        struct resolved s = name.signature[i];
        if (s.value) buffer[(*at)++] = s.value;
        else {
            buffer[(*at)++] = '(';
            represent(given.count ? given.arguments[a++] : s, buffer, limit, at, context);
            buffer[(*at)++] = ')';
        }
    }
    if (name.type.id) {
        buffer[(*at)++] = ' ';
        represent(name.type, buffer, limit, at, context);
    }
}

static struct resolved resolve(struct expression given, struct resolved type, struct context* context, size_t depth, size_t max_depth) {
    if (depth > max_depth) return (struct resolved) {0};
    
    else if (context->at < given.count && !given.symbols[context->at].value) {
        struct expression expr = given.symbols[context->at++];
        struct context sub = *context; sub.at = sub.best = 0;
        struct resolved solution = resolve(expr, type, &sub, 0, max_depth);
        if (sub.at < expr.count) solution.id = 0;
        if (!solution.id) {
            struct expression b = sub.best < given.count ? given.symbols[sub.best] : given;
            char buffer[2048] = {0}; size_t index = 0;
            represent(type, buffer, sizeof buffer,  &index, context);
            printf("n3zqx2l: %s:%u:%u: error: type=%s, unresolved %c\n", context->filename, b.line, b.column, buffer, (char) b.value);
        } return solution;
    }
    
    
    
///if (i < g.s.size() && !g.s[i].t.v)
///  return R(g.s[i++], t, E, S, I, f, m);
//
//if (i < g.s.size() && in(_s, t.i, I))
//  return {t.i, {}, g.s[i++].t, 0};
//
//  N si = i;
//  V Z = S;
//
//  for (N s : Z.back()) {
//    b = fmax(i, b);
//    i = si;
//    S = Z;
//    std::vector<r> A = {};
//    if (ne(E[s].t, t, S))
//      goto c;
//
//    for (N j = 0; j < E[s].s.size(); j++) {
//      if (i >= g.s.size()) {
//        if (A.size() && j == 1)
//          return A[0];
//        else
//          goto c;
//      }
//
//      if (!E[s].s[j].t.v) {
//        r a = u(g, E[E[s].s[j].i].t, i, b, D + 1, m, E, S, I, f);
//        if (a.error) // ||ne(E[s].t,t,S)
//          goto c;    /// TODO: CURRENT ERROR RIGHT HERE!
//        A.push_back({a});
//      } else if (E[s].s[j].t.v != g.s[i].t.v)
//        goto c;
//      else
//        i++;
//    }
//    if (in(_d, s, I)) {
//      auto ccc = A[0].t.v;
//      if (ccc - 42 < _C && ccc >= 42)
//        I[ccc - 42].push_back(E.size());
//
//      //            d({{0, {}, {'K', 0, 0, 0}}}, {_i}, {}, E, S);
//    }
//    return {s, A, {}, 0};
//  c:
//    continue;
//  }
//
  return (struct resolved) {0};
}

static void debug_expression(struct expression e) {
    if (e.value) printf("%c", (char) e.value);
    else {
        printf("(");
        for (size_t i = 0; i < e.count; i++) debug_expression(e.symbols[i]);
        printf(")");
    }
}

static void debug_context(struct context* context) {
    printf("\n---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++) {
        printf("\t ----- FRAME # %lu ---- \n\t\tidxs: { ", i);
        for (size_t j = 0; j < context->frames[i].count; j++) printf("%lu ", context->frames[i].indicies[j]);
        puts("}");
    }
    
    printf("\nmaster: {\n");
    for (size_t i = 0; i < context->name_count; i++) {
        printf("\t%6lu: ", i);
        char buffer[2048] = {0};
        size_t index = 0;
        represent((struct resolved){i, 0, 0, 0}, buffer, sizeof buffer, &index, context);
        printf("%s", buffer);
        memset(buffer, 0, sizeof buffer);
        index = 0;
        represent(context->names[i].definition, buffer, sizeof buffer, &index, context);
        if (context->names[i].definition.id) printf("      %s", buffer);
        puts("\n");
    }
    puts("}");
}


  

int main(const int argc, const char** argv) {
    size_t max_depth = 20;
    for (int i = 1; i < argc; i++) {
        if (!strcmp("version", argv[i])) exit(!puts("n: 0.0.3"));
        
        char* text = open_file(argv[i]);
        size_t at = 0;
        uint32_t line = 1, column = 1;
        struct expression expression = parse(argv[i], text, &at, &line, &column);
        debug_expression(expression); puts("");
        struct name zero = {0};
        zero.signature = calloc(zero.length = 1, sizeof(struct resolved));
        zero.signature[0].value = 'o';
        struct name one = {0};
        one.signature = calloc(one.length = 1, sizeof(struct resolved));
        one.signature[0].value = 'i';
        struct frame init = {0};
        init.indicies = calloc(init.count = 2, sizeof(size_t));
        init.indicies[0] = 0;
        init.indicies[1] = 1;
        struct context context = {0};
        context.names = calloc(context.name_count = 2, sizeof(struct name));
        context.names[0] = zero;
        context.names[1] = one;
        context.frames = calloc(context.frame_count = 1, sizeof(struct frame));
        context.frames[0] = init;
        struct resolved type = {1, 0, 0, 0};
        struct resolved resolved = resolve(expression, type, &context, 0, max_depth);
        if (context.at < expression.count) {
            printf("error: could not resolve entire expression.\n");
        }
        
        debug_context(&context);
        char buffer[2048] = {0}; size_t index = 0;
        represent(resolved, buffer, sizeof buffer, &index, &context);
        
        printf("printing solution: \n\n\t%s\n\n", buffer);
        
        if (context.at < expression.count || !resolved.id) {
            printf("\nRESOLUTION ERROR\n");
        }
        
        free(text);
    }
}
