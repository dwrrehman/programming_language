/// Daniel Rehman,
/// CE202008285.121444
/// modified on 2010117.014841

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef ssize_t nat;
typedef const char* string;

struct name {
    string signature;
    nat length;
};

static string input = "bubblesbubblestherebubblesbubblestherebubblesbyebye";

static const struct name context[] = {
    
    {"as", 2},                      // 0
    {"mommy", 5},                   // 1
    {"mom", 3},                     // 2
    {"hello", 5},                   // 3
    {"join__", 6},                  // 4
    {"bubbles_there_bye", 17},      // 5
    {"bubbles", 7},                 // 6
    
}; static const nat context_length = 7;

struct unit {
    nat index;
    nat begin;
    nat done;
    nat count;
    struct unit* args;
};

static nat best = 0;


static void debug_list(struct unit* m, nat size, nat tail) {
    puts("\nMEMORY: {\n");
    for (nat i = 0; i < size; i++) {
        printf("   %5lu    %c   i:%-5lu    d:%-5lu    b:%-5lu    c:%-5lu    a:%p \n",
               i, i == tail ? 'T' : ' ',
               (nat) m[i].index, (nat) m[i].done, (nat) m[i].begin, (nat) m[i].count, m[i].args);
    }
    puts("}\n");
}

static void debug_string(string message, string input, nat at) {
    printf("\n       %15s:  ", message);
    for (nat i = 0; i < strlen(input) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    printf(" (%lu) \n", at);
}

static void debug_tree(struct unit tree, nat d) {
    
    for (nat i = 0; i < d; i++) {
        printf(".   ");
    }
    printf("[index = %lu]\n", tree.index);
    
    for (nat i = 0; i < tree.count; i++) {
        debug_tree(tree.args[i], d + 1);
    }
}

nat parse(struct unit* list, nat size) {
    nat s = 0;
    list[s++] = (struct unit) {0};
    
    while (1) {
        while (list[s - 1].index < context_length) {
            nat done = 0, begin = list[s - 1].begin;
            while (1) {
                debug_list(list, size, s);
                struct name name = context[list[s - 1].index];
                while (done != name.length) {
                    debug_string("context", name.signature, done); debug_string("input", input, begin); puts("\n");
                    const nat c = name.signature[done++];
                    if (c == '_') {
                        
                        list[s - 1].args = realloc(list[s - 1].args, sizeof(struct unit) * (list[s - 1].count + 1));
                        list[s - 1].args[list[s - 1].count++] = (struct unit) {0};
                        
                        list[s++] = (struct unit) {0, begin, done, 0, NULL};
                        goto next;
                        
                    } else if (c == input[begin]) { begin++; if (begin > best) best = begin; }
                    else goto skip;
                }
                
                if (s <= 1) {
                    if (begin == strlen(input)) return 1;
                    else goto skip;
                    
                } else {
                    list[s - 2].args[list[s - 2].count - 1] = list[s - 1];
                    done = list[s - 1].done;
                    s--;
                }
            }
            skip:
            list[s - 1].index++;
            list[s - 1].count = 0;
            next: continue;
        }
        s--;
        if (!s) break;
        else {
            list[s - 1].index++;
            list[s - 1].count = 0;
        }
    }
    return 0;
}

int main() {
    const nat s = 12;
    struct unit* m = malloc(s * sizeof(struct unit));
    nat r = parse(m, s);
    debug_list(m,s,s);
    debug_tree(m[0], 0);
    if (!r) debug_string("error", input, best);
    printf("\n\n\t%s\n\n", r ? "success" : "failure");
    free(m);
}



















//            if (c == '_') {
//                list[at].args = realloc(list[at].args, sizeof(struct unit) * (list[at].count + 1));
//                list[at].args[list[at].count++] = (struct unit) {
//                    .index = 0,
//                    .done = 0,
//                    .begin = 0,
//                    .count = 0,
//                    .args = NULL
//                };
//
//                list[stack_count++] = (struct unit) {
//                    .index = 0,
//                    .done = done,
//                    .begin = begin,
//                    .count = 0,
//                    .args = NULL,
//                };
//                goto skip;
//            } else


//typedef size_t u64;
//typedef uint32_t u32;
//typedef uint16_t u16;
//typedef uint8_t u8;
//static const size_t undef = 13744632839234567870UL;
