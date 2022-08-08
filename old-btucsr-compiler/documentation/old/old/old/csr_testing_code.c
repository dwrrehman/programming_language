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
static nat best = 0;

static string input = "joinbubblesjoinbubblesbubbles";

static const struct name context[] = {
    {"as", 2},
    {"mommy", 5},
    {"mom", 3},
    {"hello", 5},
    {"join__", 6},
    {"bubbles_there_bye", 17},
    {"bubbles", 7},
}; static const nat context_length = 7;


struct unit {
    nat index;
    nat begin;
    nat done;
    nat count;
    struct unit* args;
};

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
            
            nat count = 0;
            struct unit* args = NULL;
            
            while (1) {
                
                debug_list(list, size, s);
                
                struct name name = context[list[s - 1].index];
                
                while (done != name.length) {
                    
                    debug_string("context", name.signature, done); debug_string("input", input, begin); puts("\n");
                    
                    const nat c = name.signature[done++];
                    if (c == '_') {
                        
                        args = realloc(args, sizeof(struct unit) * (count + 1));
                        args[count++] = (struct unit) {0};
                        
                        list[s++] = (struct unit) {0, begin, done, count, args};
                        goto next;
                        
                    } else if (c == input[begin]) { begin++; if (begin > best) best = begin; }
                    else goto skip;
                }
                printf("hi");
                
                if (s <= 1) {
                    if (begin == strlen(input)) {
                        list[s - 1].done = done;
                        list[s - 1].count = count;
                        list[s - 1].args = args;
                        return 1;
                    } else goto skip;
                    
                } else {
                
                    done = list[s - 1].done;
                    count = list[s - 1].count;
                    args = list[s - 1].args;
                    
//                    args[count - 1] = list[s - 1];
                    
                    s--;
                }
            }
            skip: list[s - 1].index++;
            next: continue;
        }
        s--;
        
        if (!s) break;
        else {
            list[s - 1].index++;
//            list[s - 1].count--;
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












































old:






















/// Daniel Rehman,
/// CE202008285.121444
/// modified on 2010117.014841

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef size_t nat;
typedef const char* string;

static const nat undef = 13744632839234567870UL;


static string context[] = {
    "_",
    "hello_",
    "dog",
}; nat context_length = 3;
static string input = "hellohellodog";

struct unit {
    nat index;
    nat parent;
    nat done;
    nat begin;
};


static inline void debug_list(struct unit* m, nat size, nat head, nat tail) {
    puts("MEMORY: {\n");
    for (nat i = 0; i < size; i++) {
        printf("   %5lu    %c%c   i:%-5lu    p:%-5lu    d:%-5lu    b:%-5lu   \n",
               i, i == head ? 'H' : ' ', i == tail ? 'T' : ' ',
               m[i].index, m[i].parent, m[i].done, m[i].begin);
    }
    puts("}\n");
}

static inline void debug_string(string id, string input, nat at) {
    printf("\n       %15s:  ", id);
    for (nat i = 0; i < strlen(input) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    printf(" (%lu) \n", at);
}


nat syntax_algorithm(struct unit* list, nat size) {

    nat tail = 0;

    list[tail++] = (struct unit) {0 /*indexof("(_)")*/, size, 0, 0};

    for (nat head = 0; head != tail; head++) {

        {puts("queue"); debug_list(list, size, head, tail);}
                
        nat done = 0, begin = list[head].begin;
        
        for (nat at = head; at != size; at = list[at].parent) {
            
            {puts("parent"); debug_list(list, size, head, tail);}

            for (nat index = list[at].index; done < strlen(context[index]); done++) {
                
                {debug_string("signature", context[index], done); debug_string("input", input, begin); puts(""); }

                if (context[index][done] == '_') {

                    for (nat arg = 0; arg < context_length; arg++) {
                        if (tail == size) break;
                        list[tail++] = (struct unit) {arg, at, done + 1, begin};
                    }
                    goto skip;

                } else if (context[index][done] == input[begin]) begin++;
                else {
                    list[head].index = undef;
                    goto skip;
                }
            }
            done = list[at].done;
        }
        if (begin == strlen(input)) return head + 1;
        skip: continue;
    }
    return size;
}









nat sa(struct unit* list, nat size) {
    nat tail = 0;
    list[tail++] = (struct unit) {0, size, 0, 0};
    for (nat head = 0; head != tail; head++) {
        nat done = 0, begin = list[head].begin;
        for (nat at = head; at != size; at = list[at].parent) {
            for (nat index = list[at].index; done < strlen(context[index]); done++) {
                if (context[index][done] == '_') {
                    for (nat arg = 0; arg < context_length; arg++) {
                        if (tail == size) break;
                        list[tail++] = (struct unit) {arg, at, done + 1, begin};
                    }
                    goto skip;
                } else if (context[index][done] == input[begin]) begin++;
                else {
                    list[head] = (struct unit) {undef,undef,undef,undef};
                    goto skip;
                }
            }
            done = list[at].done;
        }
        if (begin == strlen(input)) return head + 1;
        skip: continue;
    }
    return 0;
}











int main() {
    
    const nat s = 65536;
    struct unit* m = malloc(s * sizeof(struct unit));
    for (nat i = 0; i < s; i++) m[i] = (struct unit) {undef, undef, undef, undef};
    
    nat h = sa(m, s);
    
    debug_list(m,h,0,0);
    printf("head = %lu\n", h);
    
    free(m);
}
