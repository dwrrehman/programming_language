/**
 we are almost done!
 
 
 we just need to duplicate all parent trees, before pushing
 things,
 
 
    so like, we duplicate the tree so that each node we append to the queue, has its own little version of the tree, that it can operate on..?
 
 
 i think so........
 
 that seemt o be the root of the priblem'
 
 
 now,
 
 
 @   after that, we have to figure out how to properly imple,ent paraemters!!!
 
 
 
    we need to push and pop them, in a stacky way, so that we can backtrack from an argument list, properly..?
 
 
 
 thats important.
 
 
 
 
 
 */


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
#include <assert.h>

typedef size_t nat;
typedef nat character;
typedef nat pointer;

static const nat memory_size = 100000000;

struct resolved {
    nat index;// changes in max name count
    nat at;//ranges in max file size
    nat done; // ranges in max sig length
    nat parent; // changes in memory size
    nat queue_next;// changes in memory size
    
    nat count; // can be 63 max.
    nat args[64];
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

#define best(x) context.best = x > context.best ? x : context.best

static void prep(size_t d) { for (size_t i = d; i--;) printf(".   "); }

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

static void debug_resolved(struct resolved* memory, nat r, nat depth) {
    const struct resolved given = memory[r];
    prep(depth);
    nat printed = 0;
    if (given.index) {
        for (nat s = 0; s < context.names[given.index - 256].length; s++) {
            nat c = context.names[given.index - 256].signature[s];
            if (c >= 256) printf("_"); else printf("%c", (char) c);
            printed++;
        }
    }
    for (nat j = 0; j < 20 - printed; j++) printf(" ");
    printf(" : { i:%-5lu b:%-5lu d:%-5lu p:%-5lu cnt:%-5lu qn:%-5lu} : [ ", given.index, given.at, given.done, given.parent, given.count, given.queue_next);
    for (nat i = 0; i < given.count; i++) {
        printf("%lu ", given.args[i]);
    } printf("]\n");
    
    for (size_t i = 0; i < given.count; i++) {
        prep(depth + 1); printf("#%lu: \n", i);
        debug_resolved(memory, given.args[i], depth + 2);
    }
}

static void debug_context() {
    printf("\n----- names: ------ \n{\n");
    for (size_t i = 0; i < context.name_count; i++) {
        char buffer[4096] = {0}; size_t index = 0;
        represent(i + 256, buffer, sizeof buffer, &index);
        printf("\t%6lu: %s\n\n", i, buffer);
    } puts("}\n");
}

static void push_signature(const char* string) {
    const nat n = context.name_count, length = strlen(string);
    context.names = realloc(context.names, sizeof(struct name) * (context.name_count + 1));
    context.names[n].signature = calloc(length, sizeof(nat));
    for (nat i = 0; i < length; i++)
        context.names[n].signature[i] = string[i] != '_' ? string[i] : 256;
    context.names[n].length = length;
    context.name_count++;
}

void debug_memory(struct resolved* m, nat head, nat tail) {
    printf("-------- memory -----------------------------------------------------------------------\n");
    for (nat i = 0; i < memory_size; i++) {
        if (m[i].index) {
            const struct resolved given = m[i];
            printf("%5lu      %c%c   :   ", i, i == head ? 'H' : ' ', i == tail ? 'T' : ' ');
            nat printed = 0;
            if (given.index) {
                for (nat s = 0; s < context.names[given.index - 256].length; s++) {
                    nat c = context.names[given.index - 256].signature[s];
                    if (c >= 256) printf("_"); else printf("%c", (char) c);
                    printed++;
                }
            }
            for (nat j = 0; j < 20 - printed; j++) {
                printf(" ");
            }
            printf("   :   { i:%-5lu b:%-5lu d:%-5lu p:%-5lu cnt:%-5lu qn:%-5lu} : [ ", given.index, given.at, given.done, given.parent, given.count, given.queue_next);
            for (nat i = 0; i < given.count; i++) {
                printf("%lu ", given.args[i]);
            } printf("]\n");
        }
    }
    printf("-------------------------------------------------------------------------------------------\n");
}


void display_signature(nat* signature, nat at, nat length) {
    printf("\n         signature: ");
    for (nat i = 0; i < length + 1; i++)
        if (i < length) printf(i == at ? " [%c] " : " %c ",
               (signature[i] < 256) ? (char) signature[i] : '_');
        else printf(i == at ? " [%c] " : " %c ", '\0');
    puts("\n\n\n");
}

static void display_string_at_char(const char* input, nat at) {
    printf("\n            string:  ");
    for (nat i = 0; i < strlen(input) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    puts("");
}

nat csr(const char* given, struct resolved* memory) {
    nat length = strlen(given);
    pointer new = 2, head = 1, tail = 1;
    
    while (head) {
//        debug_memory(memory, head, tail);
        
        pointer try = head, at = try;
        
        while (at) {
            
//            printf("while(at): now recognizing: %lu\n", at);
            
//            struct resolved me = memory[at];
            struct name name = context.names[memory[at].index - 256];
            
            while (memory[at].done < name.length) {
                
//                display_string_at_char(given, memory[at].at);
//                display_signature(name.signature, memory[at].done, name.length);
                
                nat c = name.signature[memory[at].done++];
                
                if (c < 256) {
                    if (c != (nat) given[memory[at].at]) {
//                        printf("character mismatch!    "
//                               " given[.begin]('%c') != sig[.done]('%c')   "
//                               "  moving on.\n", given[memory[at].at], (char) c);
                        
                        goto next;
                    } else {
                        memory[at].at++;
                        best(memory[at].at);
                    }
                } else {
//                    printf("found parameter: pushing signatures...\n");
                    
                    for (nat i = 0; i < context.name_count; i++) {
                        
                        if (new < memory_size) {
                            memory[tail].queue_next = new;
                            tail = new;
                            memory[new] = (struct resolved){i + 256, memory[at].at, 0, new + 1, 0, 0, {0}};
                            new++;
                        }
                        
                        nat n = at;
                        if (new < memory_size) {
                            memory[new] = memory[n];
                            memory[new].args[memory[new].count++] = new - 1;
                            memory[new].parent = new + 1;
                            new++;
                            n = memory[n].parent;
                        }
                        
                        /// theproblem is that we need to redo the arguments, for  each parent, now. not just the .parent's, but also every single last argument, i think. thats crucial. lets do it.
                        
                    
                        while (n && new < memory_size) {
                            memory[new] = memory[n];
                            memory[new].args[memory[new].count - 1] = new - 1;
                            memory[new].parent = new + 1;
                            new++;
                            n = memory[n].parent;
                        }
                        
                        memory[new - 1].parent = 0;
                    }
//                    printf("pushing complete. moving on.\n");
                    goto next;
                }
            }
            
//            printf("while(at): sig finished: \n");
//            display_string_at_char(given, memory[at].at);
//            display_signature(name.signature, memory[at].done, name.length);
            
            pointer parent = memory[at].parent;
            
            if (parent) {
//                printf("while(at): parent is nonnull! pushing argument...\n");
//                memory[parent].args[memory[parent].count++] = at;
                memory[parent].at = memory[at].at;
                
            } else {
                if (memory[at].at == length) {
//                    printf("recognitition success: returning %lu\n", at);
                    return at;
                } else {
//                    printf("recognitition failure: %lu != %lu  ...\n", memory[at].at, length);
                    goto next;
                }
            }
            at = parent;
//            printf("while(at): recognitition success. moving to parent: %lu\n", parent);
        }
//        printf("next: reached end of parent chain, and didnt return, moving on...\n");
    next:
        head = memory[try].queue_next;

//        printf("next:   trying now: [head = %lu]\n", head);
    }
    return 0;
}

//        struct resolved saved = memory[try]; //, saved_me = me;
//        memory[try] = saved;

static void print_error(const char *input, struct resolved *memory, nat s) {
    struct resolved solution = memory[s];
    printf("\n\n\n\nsolution = %lu\n", s);
    debug_resolved(memory, s, 0);
    printf("\n\n\n\n");
    if (!solution.index) {
        printf("error: @ %lu: unexpected %c%s\n",
               context.best,
               input[context.best],
               input[context.best] ? "" : "end of expression");
        display_string_at_char(input, context.best);
    } else printf("\n\t [parse successful]\n\n");
}

void test_csr_with_string(const char* input, nat i) {
    
    struct resolved* m = calloc(memory_size, sizeof(struct resolved));
    m[1].index = 256 + i;
    nat s = csr(input, m);
    print_error(input, m, s);
//    debug_memory(m, 0, 0);
    free(m);
}

static void construct_context(const char** signatures, nat length) {
    for (nat i = 0; i < length; i++) push_signature(signatures[i]);
}

static void destroy_context() {
    for (nat i = 0; i < context.name_count; i++) {
        free(context.names[i].signature);
    }
    free(context.names);
    context = (struct context){0};
}

void start() {
    const char* signatures[] = {
        "h",
        "ht",
        "__",
    };
    
    construct_context(signatures, sizeof(signatures) / sizeof(const char*));
    debug_context();
    test_csr_with_string("hhhhht", 2);
    destroy_context();
}
