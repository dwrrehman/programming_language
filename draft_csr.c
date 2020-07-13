//
//  batch_trees.c
//  sandbox
//
//  Created by Daniel Rehman on 2007127.
//  Copyright © 2020 Daniel Rehman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

typedef size_t nat;

static const nat memory_size = 15; //65536;

struct resolved {
    nat index;// changes in max name count
    nat begin;//ranges in max file size
    nat done; // ranges in max sig length
    nat parent; // changes in memory size
    nat queue_next;// changes in memory size
    
    nat count; // can be 62 max.
    nat args[63];
};

/**
 struct resolved {
     nat index;
     nat begin;
     nat done;
     nat parent;
     nat head;
     nat tail;
     nat next;
     nat queue_next;
 };
 */

struct name {
    nat* signature;
    nat length;
};

struct context {
    nat best;
    nat name_count;
    struct name* names;
};

static struct context context = {0};

#define best() context.best = m[n].begin > context.best ? m[n].begin : context.best

//
//static void prep(size_t d) {
//    for (size_t i = d; i--;) printf(".   ");
//}
//

static inline void represent
(size_t given, char* buffer, size_t limit,
 size_t* at) {
    if (given < 256) {
        buffer[(*at)++] = given;
        return;
    } else given -= 256;
    if (given >= context.name_count) return;
    struct name s = context.names[given];
    for (size_t i = 0; i < s.length; i++) {
        const size_t c = s.signature[i];
        if (c < 256) {
            buffer[(*at)++] = c;
        } else {
            buffer[(*at)++] = ' ';
            buffer[(*at)++] = '(';
            represent(c, buffer, limit, at);
            buffer[(*at)++] = ')';
            buffer[(*at)++] = ' ';
        }
    }
}




//static void debug_resolved_parent(struct resolved* memory, nat r, nat depth) {
//    const struct resolved given = memory[r];
//    prep(depth); printf("@%lu: { index: %lu, begin: %lu, done: %lu, parent: %lu, next: %lu, previous: %lu, count: %lu} \n", r, given.index, given.begin, given.done, given.parent, given.next, given.previous, given.count);
//    for (size_t i = 0; i < given.count; i++) {
//        prep(depth + 1); printf("#%lu: %lu\n", i, given.args[i]);
//        debug_resolved_parent(memory, given.args[i], depth + 1);
//    }
//
//    if (given.parent) {
//        prep(depth); printf("parent: \n");
//        debug_resolved_parent(memory, given.parent, depth);
//    }
//}
//
//static void debug_resolved_next(struct resolved* memory, nat r, nat depth) {
//    const struct resolved given = memory[r];
//    prep(depth); printf("@%lu: { index: %lu, begin: %lu, done: %lu, parent: %lu, next: %lu, previous: %lu, count: %lu} \n", r, given.index, given.begin, given.done, given.parent, given.next, given.previous, given.count);
//    for (size_t i = 0; i < given.count; i++) {
//        prep(depth + 1); printf("#%lu: %lu\n", i, given.args[i]);
//        debug_resolved_next(memory, given.args[i], depth + 1);
//    }
//
//    if (given.next) {
//        prep(depth); printf("next: \n");
//        debug_resolved_next(memory, given.next, depth);
//    }
//}
//
//static void debug_resolved_previous(struct resolved* memory, nat r, nat depth) {
//    const struct resolved given = memory[r];
//    prep(depth); printf("@%lu: { index: %lu, begin: %lu, done: %lu, parent: %lu, next: %lu, previous: %lu, count: %lu} \n", r, given.index, given.begin, given.done, given.parent, given.next, given.previous, given.count);
//    for (size_t i = 0; i < given.count; i++) {
//        prep(depth + 1); printf("#%lu: %lu\n", i, given.args[i]);
//        debug_resolved_previous(memory, given.args[i], depth + 1);
//    }
//
//    if (given.previous) {
//        prep(depth); printf("previous: \n");
//        debug_resolved_previous(memory, given.previous, depth);
//    }
//}

static void debug_context() {
    printf("\n----- names: ------ \n{\n");
    for (size_t i = 0; i < context.name_count; i++) {
        char buffer[4096] = {0}; size_t index = 0;
        represent(i + 256, buffer, sizeof buffer, &index);
        printf("\t%6lu: %s\n\n", i, buffer);
    } puts("}\n");
}

static void construct_context() {
    context.best = 0;
    context.name_count = 4;
    context.names = calloc(context.name_count, sizeof(struct name));
    
    // a      param type.
    context.names[0].length = 1;
    context.names[0].signature = calloc(1, sizeof(size_t));
    context.names[0].signature[0] = 'a';
    
    // b
    context.names[1].length = 1;
    context.names[1].signature = calloc(1, sizeof(size_t));
    context.names[1].signature[0] = 'b';
    
    // (x) (y)
    context.names[2].length = 3;
    context.names[2].signature = calloc(3, sizeof(size_t));
    context.names[2].signature[0] = 'i';
    context.names[2].signature[1] = 256;
    context.names[2].signature[2] = 256;
    
//    // (x) empty
//    context.names[3].length = 6;
//    context.names[3].signature = calloc(6, sizeof(size_t));
//    context.names[3].signature[0] = 256;
//    context.names[3].signature[1] = 'e';
//    context.names[3].signature[2] = 'm';
//    context.names[3].signature[3] = 'p';
//    context.names[3].signature[4] = 't';
//    context.names[3].signature[5] = 'y';
    
    // (x)
    context.names[3].length = 1;
    context.names[3].signature = calloc(1, sizeof(size_t));
    context.names[3].signature[0] = 256;
}

void debug_memory(struct resolved* m) {
    printf("----------------- memory ------------------\n");
    for (nat i = 0; i < memory_size; i++) {
        if (m[i].index) {
            struct resolved given = m[i];
            printf("%5lu  :  { i:%-5lu b:%-5lu d:%-5lu p:%-5lu cnt:%-5lu qn:%-5lu} : [ ", i, given.index, given.begin, given.done, given.parent, given.count, given.queue_next);
            for (nat i = 0; i < given.count; i++) {
                printf("%lu ", given.args[i]);
            } printf("]\n");
        }
    }
    printf("--------------------------------------\n");
}

static nat csr(const uint8_t* given, nat end, struct resolved* m) {
    
    nat p = 2, head = 1, tail = 1;
    m[1] = (struct resolved){256 + 3, 0, 0, 0, 0, 0, {0}};
    struct name name;
    
    while (head) {
        printf("head = %lu, tail = %lu\n", head, tail);
        debug_memory(m);
    
        nat N = head, n = N;
        
        recognize:
        name = context.names[m[n].index - 256];
        while (m[n].done < name.length) {
            nat c = name.signature[m[n].done];
            if (c < 256) {
                if (c == given[m[n].begin]) {
                    m[n].begin++; m[n].done++; best();
                } else goto next;
            } else {
                for (nat i = 0; i < context.name_count; i++) {
                    if (p < memory_size) {
                        m[p] = (struct resolved){i + 256, m[n].begin, 0, n, 0, 0, {0}};
                        m[tail].queue_next = p; tail = p++;
                    }
                }
                goto next;
            }
        }
        if (m[n].parent) {
            nat parent = m[n].parent;
            m[parent].args[m[parent].count++] = n;
            n = parent;
            goto recognize;
        }
        
        if (m[n].begin == end) return n;
        
        next: head = m[N].queue_next;
        
    }
    return 0;
}

static void print_error(const char *input, struct resolved *memory, nat s) {
    struct resolved solution = memory[s];
    printf("solution = %lu\n", s);
    if (!solution.index)
        printf("error: @ %lu: unexpected %c%s\n",
               context.best,
               input[context.best],
               input[context.best] ? "" : "end of expression");
    else
        printf("\n\t [parse successful]\n\n");
}

static void test_csr_with_string() {
    struct resolved* m = malloc(memory_size * sizeof(struct resolved));
    memset(m, 0, memory_size * sizeof(struct resolved)); //temp
    const char* input = "a";
    nat s = csr((const uint8_t*) input, strlen(input), m);
    print_error(input, m, s);
    debug_memory(m);
    free(m);
}

void start() {
    construct_context();
    debug_context();
    test_csr_with_string();
}








































// ------------------------------------------------------------------------




/*

nat csr(const char* given, nat end, nat max_depth, struct resolved* memory, nat ptr) {
    
    
    memory[ptr] = (struct resolved){256 + 1, 0, 0, 0, 0, 0, 0};
    nat queue = ptr++;
                
    while (queue) {
        
        struct resolved node = queue_pop(&queue, &queue_count);
        struct name name = context.names[node.index - 256];
        
        while (node.done < name.length) {
            nat c = name.signature[node.done];
            if (c < 256) {
                if (c == given[node.begin]) {
                    node.begin++; node.done++;
                } else goto next;
            } else {
                for (nat i = 0; i < context.name_count; i++) {

                    // push i to node's parameter list.

                    // push that tree to the queue.

                    // pop i from node's parameter list.         (undo)

                }
                goto next;
            }
        }
        
        if (node.parent) {
            node.parent->begin = node.begin;
            node.parent->done++;
             ????
            
        } else if (node.begin == end) return node;
        next: continue;
    }
    
    return 0;
}
*/





//
//struct resolved queue_pop(struct resolved** v, size_t* count) {
//    if (!*count) abort();
//    struct resolved f = **v;
//    --*count;
//    memmove(*v, *v + 1, sizeof(struct resolved) * *count);
//    *v = realloc(*v, sizeof(struct resolved) * *count);
//    return f;
//}
//
//void queue_push(struct resolved f, struct resolved** v, size_t* count) {
//    *v = realloc(*v, sizeof(struct resolved) * (*count + 1));
//    (*v)[(*count)++] = f;
//}



//void debug_queue(struct resolved* queue, nat queue_count) {
//    printf("----------------------------- debugging queue (queue count = %lu) ------------------------\n\n", queue_count);
//    for (nat i = 0; i < queue_count; i++) {
//        printf("\t----------------- queue node #%lu --------------- \n", i);
//        struct resolved n = queue[i];
//        debug_resolved(n, 2);
//        printf("\t-------------------------------------------------\n\n");
//    }
//    printf("----------------------------------------------------------------------------------------\n");
//}
