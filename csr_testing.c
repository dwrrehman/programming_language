/// Daniel Rehman, CE202008285.121444
/// this is my second try at making a more memory efficient bfs csr algorithm.
/// i reworked the data structure a ton, and am making things simpler, hopefully.

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef size_t nat;
typedef const char* string;

static nat* memory = 0, * context = 0, best = 0, context_length = 0, width = 3, size = 10;

static void debug_memory(nat head, nat tail) {
    printf("-------- memory ----------\n");
    for (nat i = 0; i < size; i++) {
        nat* m = memory + i * width;
        printf("%5lu   %c%c  {  i:%-5lu p:%-5lu b:%-5lu  }\n",
               i * width,
               i * width == head ? 'H' : ' ',
               i * width == tail ? 'T' : ' ',
               m[0], m[1], m[2]);
    }
    printf("-------------------------\n");
}

static void display_signature(nat at) {
    printf("\n         signature: ");
    for (nat i = 0; i < context_length + 1; i++)
        if (i < context_length) printf(i == at ? "[%c] " : "%c ",
                               (context[i] < 256)
                                       ? (char) context[i]
                                       : '_');
        else printf(i == at ? "[%c] " : "%c ", '\0');
    puts("\n");
}

static void display_string_at_char(string input, nat at) {
    printf("\n            string:  ");
    for (nat i = 0; i < strlen(input) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    puts("\n");
}

static void debug_context() {
    printf("------ context ------\n");
    for (nat i = 0; i < context_length; i++) {
        const nat c = context[i];
        if (c && c < 256) printf("%c ", (char) c);
        else printf("(%lu) ", c);
        if (!c) printf("\n%lu: ", i);
    }
    printf("---------------------\n");
}

static void print_result(string input, nat* memory, nat s, nat best) {
    printf("\n\n\n\nsolution = %lu\n", s);
    printf("\n\n\n\n");
    if (!s) {
        printf("error: @ %lu: unexpected %c%s\n",
               best,
               input[best],
               input[best] ? "" : "end of expression");
        display_string_at_char(input, best);
    } else printf("\n\t [parse successful]\n\n");
}

static void create_context(string* names) {
    for (nat i = 0; names[i]; i++) {
        for (nat c = 0; names[i][c]; c++) {
            const nat k = names[i][c];
            context = realloc(context, sizeof(nat) * (context_length + 1));
            context[context_length++] = k == '_' ? 256 : k;
        }
        context = realloc(context, sizeof(nat) * (context_length + 1));
        context[context_length++] = 0;
    }
    
    memory = malloc(size * width * sizeof(nat));
    for (nat i = 0; i < size * width; i++) {
        memory[i] = 999;
    }
    memset(memory, 0, width * sizeof(nat));
}

static void destroy_context() {
    free(memory);
    free(context);
}

static nat solve(string input) {
    for (nat head = 0, tail = 3; head != tail; head += 3) {
        for (nat index = memory[head]; index < context_length; index++) {
            for (nat at = head; at; at = memory[at + 1]) {
                nat parent = memory[at + 1], begin = memory[at + 2], bad = 0;
                for (; context[index]; index++) {
                    if (bad) continue;
                    else if (context[index] >= 256) {
                        memory[tail++] = index;
                        memory[tail++] = parent;
                        memory[tail++] = begin;
                        bad = 1;
                    } else if (context[index] == (nat) input[begin]) begin++;
                    else bad = 1;
                }
                if (bad) break;
                else if (parent) memory[parent + 2] = begin;
                else if (begin == strlen(input)) return at;
                else break;
            }
        }
    }
    return 0;
}



      

//static
void start() {
    create_context((string[]) {"-", "hello", "bubbles_there", 0});
    debug_context();
    printf("%lu\n", solve("bubbleshellothere"));
    debug_memory(0, 0);
    destroy_context();
}










//        memory[tail++] = 0;
//        memory[tail++] = head;
//        memory[tail++] = memory[head + 2];

//                memory[at + 0] = index;
//                memory[at + 1] = parent;
//                memory[at + 2] = begin;
       














/*
 
 
 
 
 
 
 
 
 
 
 

 static nat debug_solve(string given) {
     for (nat head = 3, tail = 6; head != tail; head += 3) {
         puts("queue state:");
         debug_memory(head, tail);
         for (nat index = memory[head]; index < context_length; index++) {
             puts("index state:");
             debug_memory(head, tail);
             for (nat at = head; at; at = memory[at + 1]) {
                 puts("parent state:");
                 debug_memory(head, tail);
                 nat parent = memory[at + 1], begin = memory[at + 2];
                 bool good = true;
                 for (; context[index]; ) {
                     
                     if (good) {
                         display_signature(index);
                         display_string_at_char(given, begin);
                     }
                     
                     nat c = context[index++];
                     
                     if (!good) { puts("signature was deemed invalid."); continue; }
                     else if (c >= 256) {
                         puts("found argument! moving on.");
                         memory[at + 0] = index;
                         memory[at + 1] = parent;
                         memory[at + 2] = begin;
                         
                         memory[tail + 0] = 0;
                         memory[tail + 1] = at;
                         memory[tail + 2] = begin;
                         tail += 3;
                         
                         good = false;
                         debug_memory(head, tail);
                     }
                     else if (c == (nat) given[begin]) { puts("char match..."); begin++; }
                     else { puts("character mismatch in signature"); good = false; }
                 }
                 debug_memory(head, tail);
                 if (!good) { puts("failing signature, moving to next index."); break; }
                 else if (parent) { puts("moving to next parent"); memory[parent + 2] = begin; }
                 else if (begin == strlen(given)) { puts("-> found solution"); return at; }
                 else { puts("skip: no parents, length didnt match"); break; }
             }
         }
     }
     return 0;
 }
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 ------------------------------ working (slow) csr ------------------------------------------
 

 static inline size_t parse(uint8_t* given, size_t length, size_t size,
                            struct resolved* memory, struct context* context) {
     for (size_t next = 2, head = 1, tail = 1; head; head = memory[head].queue_next) {
         for (; memory[head].index < context->name_count; memory[head].index++) {
             for (size_t at = head; at; at = memory[at].parent) {
                 struct resolved me = memory[at];
                 struct name name = context->names[me.index];
                 while (me.done < name.length) {
                     size_t c = name.signature[me.done++];
                     if (c >= 256 && next + 2 < size) {
                         struct resolved child = {.begin = me.begin, .parent = next + 1};
                         me.args[me.count++] = next;  // this is definitely wrong.
                         memory[tail].queue_next = next; tail = next;
                         memory[next++] = child;
                         memory[next++] = me;     // this seems wrong.
                     }
                     if (c >= 256 || c != (size_t) given[me.begin]) goto skip;
                     me.begin++;
                     if (me.begin > context->best) context->best = me.begin;
                 }
                 size_t p = me.parent;
                 if (!p) { if (me.begin == length) return at; else break; }
                 memory[p].begin = me.begin;
                 memory[p].args[memory[p].count - 1] = next;
                 if (next + 1 < size) memory[next++] = me;
             } skip: continue;
         }
         memory[head].index = 0;
     }
     return 0;
 }


 */


//static inline size_t parse(string given, nat length, nat size,
//                           struct unit* memory, struct context* context) {
//    size_t tail = 2;
//     for (nat head = 1; head < size; head++) {
//         for (; memory[head].index < context->name_count; memory[head].index++) {
//             for (size_t at = head; at; at = memory[at].parent) {
//                 struct resolved me = memory[at];
//                 struct name name = context->names[me.index];
//                 for (; me.done < name.length; ) {
//                     size_t c = name.signature[me.done++];
//                     if (c >= 256 && next + 2 < size) {
//                         struct resolved child = {.begin = me.begin, .parent = next + 1};
//                         me.args[me.count++] = next;
//                         memory[tail].queue_next = next; tail = next;
//                         memory[next++] = child; memory[next++] = me;
//                     }
//                     if (c >= 256 || c != (size_t) given[me.begin]) goto skip;
//                     me.begin++; context->best = me.begin > context->best ? me.begin : context->best;
//                 }
//                 size_t p = me.parent;
//                 if (!p) {
//                     if (me.begin == length) return at; else break;
//                 }
//                 memory[p].begin = me.begin;
//                 memory[p].args[memory[p].count - 1] = next;
//                 if (next + 1 < size) memory[next++] = me;
//             } skip: continue;
//         }
//     }
//     return 0;
// }

//
//static nat csr(string given) {
//    nat tail = 2;
//    for (nat head = 1; head != tail; head++) {
//        for (nat at = head; at; at = memory[at].parent) {
//            struct unit me = memory[at];
//            while (me.index < context_length) {
//                nat c = context[me.index++];
//                if (c < 256) {
//                    if (c != (nat) given[me.string]) goto skip;
//                    else me.string++;
//                } else {
//                    struct unit child = { .string = me.string, .parent = at };
//                    memory[tail++] = child;
//                    goto skip;
//                }
//            }
//            if (me.parent) memory[me.parent].string = me.string;
//            else if (me.string == strlen(given)) return at;
//            skip: continue;
//        }
//    }
//    return 0;
//}




//        int printed = 0;
//        if (given.string < context.length) {
//            for (nat s = 0; s < context.names[given.index].length; s++) {
//                nat c = context.names[given.index].signature[s];
//                if (c >= 256) printf("_"); else printf("%c", (char) c);
//                printed++;
//            }
//        } else {
//            printf("////");
//            printed += 4;
//        }
//        for (int j = 0; j < abs(max_name_length - printed); j++) printf(" ");







/*
 
 
 
 
 

 static nat s(string given) {
     nat tail = 2;
     printf("in csr.\n");
     for (nat head = 1; head != tail; head++) {
         printf("head = %lu\n", head);
         for (nat at = head; at; at = memory[at].where) {
             printf("at = %lu\n", at);
             nat begin = memory[at].begin;
             nat where = memory[at].where;
             for (nat index = 0; index < context_length; index++) {
                 printf("me.index = %lu\n", index);
                 while (context[index]) {
                     
                     display_signature(context, context_length, index);
                     display_string_at_char(given, begin);
                     
                     nat c = context[index++];
                     
                     if (c < 256) {
                         if (c != (nat) given[begin]) {
                             printf("mismatch, going to next index.\n");
                             goto next_index;
                         } else {
                             begin++;
                         }
                     } else {
                         printf("found argument! going to next node.\n");
                         struct unit child = {
                             .index = index,
                             .begin = begin,
                             .where = at,
                         };
                         memory[tail++] = child;
                         goto next_node;
                     }
                 }
                 if (where) {
                     memory[where].begin = begin;
                     printf("going to next parent!\n");
                     goto next_parent;
                 } else if (begin == strlen(given)) {
                     printf("returning solution = %lu\n", at);
                     return at;
                 }
                 next_index:
                 while (context[index]) index++;
                 continue;
             } next_parent: continue;
         } next_node: continue;
     } return 0;
 }
 
 
 
 
 
 
 
 
 
 
 
 
 
 context:
 ---------------
 
 
 
 
 
 
 
 
                        h e l l o  \0  b u b b l e s  \0
 
                                    ^
 
 
 
 
 
 
 
 
 
 
static nat s(string given) {
    for (nat head = 3, tail = 6; head != tail; head++) {
        for (nat index = memory[head]; index < length; index++) {
            for (nat at = head; at; at = memory[at + 1]) {
                nat where = memory[at + 1], begin = memory[at + 2];
                bool good = true;
                for (; context[index]; ) {
                    nat c = context[index++];
                    if (!good) {}
                    else if (c >= 256) {}
                    else if (c == (nat) given[begin]) {}
                    else {}
                }
            }
        }
    }
    return 0;
}
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 static nat debug_solve(string given) {
     for (nat head = 3, tail = 6; head != tail; head += 3) {
         puts("queue state:");
         debug_memory(head, tail);
         for (nat index = memory[head]; index < context_length; index++) {
             puts("index state:");
             debug_memory(head, tail);
             for (nat at = head; at; at = memory[at + 1]) {
                 puts("parent state:");
                 debug_memory(head, tail);
                 nat parent = memory[at + 1], begin = memory[at + 2];
                 bool good = true;
                 for (; context[index]; ) {
                     
                     if (good) {
                         display_signature(index);
                         display_string_at_char(given, begin);
                     }
                     
                     nat c = context[index++];
                     
                     if (!good) { puts("signature was deemed invalid."); continue; }
                     else if (c >= 256) {
                         puts("found argument! moving on.");
                         memory[at + 0] = index;
                         memory[at + 1] = parent;
                         memory[at + 2] = begin;
                         
                         memory[tail + 0] = 0;
                         memory[tail + 1] = at;
                         memory[tail + 2] = begin;
                         tail += 3;
                         
                         good = false;
                         debug_memory(head, tail);
                     }
                     else if (c == (nat) given[begin]) { puts("char match..."); begin++; }
                     else { puts("character mismatch in signature"); good = false; }
                 }
                 debug_memory(head, tail);
                 if (!good) { puts("failing signature, moving to next index."); break; }
                 else if (parent) { puts("moving to next parent"); memory[parent + 2] = begin; }
                 else if (begin == strlen(given)) { puts("-> found solution"); return at; }
                 else { puts("skip: no parents, length didnt match"); break; }
             } // parent loop
 //            memory[head + 1] = 0;
         } // index loop
 //        memory[head] = 0;
     } // queue loop
     return 0;
 }


 static nat s(string given) {
     for (nat head = 3, tail = 6; head != tail; head += 3) {
         for (nat index = memory[head]; index < context_length; index++) {
             for (nat at = head; at; at = memory[at + 1]) {
                 nat parent = memory[at + 1], begin = memory[at + 2];
                 bool good = true;
                 for (; context[index]; ) {
                     nat c = context[index++];
                     if (!good) continue;
                     else if (c >= 256) {
                         memory[at + 0] = index;
                         memory[at + 1] = parent;
                         memory[at + 2] = begin;
                         memory[tail + 0] = 0;
                         memory[tail + 1] = at;
                         memory[tail + 2] = begin;
                         tail += 3;
                         good = false;
                     }
                     else if (c == (nat) given[begin]) begin++;
                     else good = false;
                 }
                 if (!good) break;
                 else if (parent) memory[parent + 2] = begin;
                 else if (begin == strlen(given)) return at;
                 else break;
             }
         }
     }
     return 0;
 }

 
 
 
 
 
 
 
 
 
 
 
*/
