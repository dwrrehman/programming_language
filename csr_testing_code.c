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
