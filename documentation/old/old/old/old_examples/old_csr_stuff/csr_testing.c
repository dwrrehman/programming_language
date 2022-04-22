#include <stdio.h>
#include <stdlib.h>
#define failure (r) {0}
const int max_depth = 10;

struct name {
    int* signature;
    int length;
};

struct context {
    int name_count;
    struct name* names;
};

struct context C;

typedef struct r { int index, total, count; struct r* args;} r;
void prep(int d) { for (int i = d; i--;) printf(".   "); }

void print(int* signature, int length) {
    for (int i = 0; i < length; i++) printf("%c ", signature[i] < 256 ? (char) signature[i] : '_');
}

r recognize_signature(uint8_t* given, int begin, int end, int depth, int remaining, r solution);

r resolve_string
(uint8_t* given,
 int begin, int end,
 int depth,
 int remaining, r parent
 ) {
    if (!depth) {
        return failure;
    }
    
    if (begin >= end) {
        return failure;
    }
    
    for (int i = 0; i < C.name_count; i++) {
        
        r solution = {i + 256, 0, 0, 0};
        struct name name = C.names[i];
        
        for (int s = 0; s < name.length; s++) {
            if (name.signature[s] < 256) {
                solution.total++;
                
            } else {
                r argument = resolve_string(given, begin, end, depth - 1, remaining, parent);
                if (!argument.index) return failure;
                solution.total += argument.total;
            }
        }
        
        r newparent = recognize_signature(given, begin, end, depth, remaining, parent);
        
        solution = recognize_signature(given, begin + newparent.total, end, depth, name.length, solution);
        if (!solution.index) continue;
    
        if (begin + solution.total == end) {
            return solution;
        }
    }
    return failure;
}

r recognize_signature
(uint8_t* given,
 int begin, int end,
 int depth, int remaining,
 r solution
 ) {
    
    if (!remaining) {
        return begin == end ? solution : failure;
    }
    
    if (begin >= end) {
        return failure;
    }
    
    const int c = C.names[solution.index - 256].signature[remaining - 1];
    
    if (c < 256) {
        
        solution = recognize_signature(given, begin, end - 1, depth, remaining - 1, solution);
        
        if (!solution.index) {
            return failure;
        }
        
        if (c == given[begin + solution.total]) {
            solution.total++;
        }
        
    } else {
        
        r arg = resolve_string(given, begin, end, depth - 1, remaining, solution);
        if (!arg.index) return failure;
        
        solution.args = realloc(solution.args, sizeof(r) * (solution.count + 1));
        solution.args[solution.count++] = arg;
        solution.total += arg.total;
    }
    
    return begin + solution.total == end ? solution : failure;
}


int main() {
    C.name_count = 3;
    C.names = calloc(C.name_count, sizeof(struct name));
    
    // h
    C.names[0].length = 1;
    C.names[0].signature = calloc(1, sizeof(int));
    C.names[0].signature[0] = 'h';
    
    // (x) e
    C.names[1].length = 2;
    C.names[1].signature = calloc(2, sizeof(int));
    C.names[1].signature[0] = 256;
    C.names[1].signature[1] = 256;
    
//    // (x)
//    C.names[2].length = 1;
//    C.names[2].signature = calloc(1, sizeof(int));
//    C.names[2].signature[0] = 256;
    
    const int given_count = 2;
    uint8_t* given = calloc(given_count + 1, sizeof(uint8_t));
    given[0] = 'h';
    given[1] = 'h';
    given[2] = '\0';
    
    r solution = {258, 0, 0, 0};
    const int begin = 0, end = given_count;
    solution = resolve_string(given, begin, end, max_depth, 1, solution);
    
    if (!solution.index) printf("error!\n");
    printf("solution = %d,%d\n", solution.index, solution.total);
}
