/// this is my csr testing code, to try and figure out bfs.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

typedef size_t nat;
typedef const char* string;

struct resolved {
    nat index;          // ranges in max name count          √
    nat begin;          // ranges in max file size           ?
    nat done;           // ranges in max sig length          ?
    nat queue_next;           // ranges in memory size       √
    nat parent;         // ranges in memory size             √
    nat count;          // ranges in 64;        delete me?
    nat args[64];       // make this just a nat.
};

struct name {
    nat* signature;
    nat length;
};

struct context {
    nat best;
    nat name_count;
    struct name* names;
};

typedef struct resolved* res_array;

static struct context context = {0};

#define best(x) context.best = x > context.best ? x : context.best

static void print_resolved_data(struct resolved given) {
    printf("   :   { i:%-5lu b:%-5lu d:%-5lu p:%-5lu qn:%-5lu cnt:%-5lu } : [ ",
           given.index, given.begin, given.done, given.parent, given.queue_next, given.count);
    for (nat i = 0; i < given.count; i++) printf("%lu ", given.args[i]);
    printf("]\n");
}

static void debug_resolved(res_array memory, nat r, nat depth) {
    const struct resolved given = memory[r];
    for (size_t i = depth; i--;) printf(".   ");
    int printed = 0;
    if (r) {
        if (given.index >= context.name_count) printf("ERROR IN SIG INDEX");
        else for (nat s = 0; s < context.names[given.index].length; s++) {
            nat c = context.names[given.index].signature[s];
            if (c >= 256) printf("_"); else printf("%c", (char) c);
            printed++;
        }
    } else {
            printf("?");
            printed++;
        }
    for (int j = 0; j < abs(30 - printed); j++) printf(" ");
    print_resolved_data(given);
    for (size_t i = 0; i < given.count; i++) {
        for (size_t i = depth + 1; i--;) printf(".   ");
        printf("#%lu: \n", i);
        debug_resolved(memory, given.args[i], depth + 2);
    }
}

static void debug_context() {
    printf("\n----- context names: ------ \n{\n");
    for (size_t i = 0; i < context.name_count; i++) {
        printf("\t%6lu  :    ", i);
        struct name name = context.names[i];
        for (nat s = 0; s < name.length; s++)
            printf("%c",
                   name.signature[s] < 256
                    ? (char) name.signature[s]
                    : '_');
        puts("\n");
    }
    puts("}\n");
}

static void debug_memory(res_array m, nat head, nat tail, nat next, nat count) {
    const int max_name_length = 32;
    printf("-------- memory -----------------------------------------------------------------------\n");
    for (nat i = 0; i < count; i++) {
        const struct resolved given = m[i];
        printf("%5lu     %c%c%c   :   ", i, i == head ? 'H' : ' ', i == tail ? 'T' : ' ', i == next ? 'N' : ' ');
        int printed = 0;
        if (given.index < context.name_count) {
            for (nat s = 0; s < context.names[given.index].length; s++) {
                nat c = context.names[given.index].signature[s];
                if (c >= 256) printf("_"); else printf("%c", (char) c);
                printed++;
            }
        } else {
            printf("////");
            printed += 4;
        }
        for (int j = 0; j < abs(max_name_length - printed); j++) printf(" ");
        print_resolved_data(given);
    }
    printf("-------------------------------------------------------------------------------------------\n");
}

static void display_signature(nat* signature, nat at, nat length) {
    printf("\n         signature: ");
    for (nat i = 0; i < length + 1; i++)
        if (i < length) printf(i == at ? "[%c] " : "%c ",
                               (signature[i] < 256) ? (char) signature[i] : '_');
        else printf(i == at ? "[%c] " : "%c ", '\0');
    puts("\n");
}

static void display_string_at_char(string input, nat at) {
    printf("\n            string:  ");
    for (nat i = 0; i < strlen(input) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    puts("\n");
}



static void clone(struct resolved me,
                  nat n, nat max,
                  nat* head, nat* next, nat* tail,
                  res_array memory) {

    printf("cloning parent stack...\n");

    if (*next + 2 < max) {
        struct resolved child = {
            .index = 0,
            .begin = me.begin,
            .done = 0,
            .queue_next = 0,
            .parent = *next + 1,
            .count = 0,
            .args = {0},
        };
        memory[*tail].queue_next = *next; *tail = *next;
        me.args[me.count++] = *next;
        memory[(*next)++] = child;
        memory[(*next)++] = me;
    
    } else { printf("error, ran out of memory.\n"); }
    
    debug_memory(memory, *head, *tail, *next, max);
}

static bool matches(nat n, string given, res_array memory,
                      nat max, nat* head, nat* next, nat* tail) {
    for (; n; n = memory[n].parent) {
        
        printf("trying me/parent = %lu\n", n);
        
        struct resolved me = memory[n];
        struct name name = context.names[me.index];
        
        printf("recognizing...\n");
        for (; me.done < name.length; ) {
            
            printf("looking at: \n");
            display_signature(name.signature, me.done, name.length);
            display_string_at_char(given, me.begin);
                        
            nat c = name.signature[me.done++];
            
            if (c >= 256) {
                printf("[found argument, failing]\n");
                clone(me, n, max, head, next, tail, memory);
                return 0;
            } else if (c != (nat) given[me.begin]) {
                printf("[character mismatch, failing]\n");
                return 0;
            }
            me.begin++;
            best(me.begin);
        }
        printf("[signature succeeded]\n");
        
        if (!me.parent) {
            printf("at last parent, returning %s!\n\n", me.begin == strlen(given) ? "SUCCESS" : "FAILURE");
            return me.begin == strlen(given);
        } else {
            nat p = me.parent;
            memory[p].begin = me.begin;
            memory[p].args[memory[p].count - 1] = *next;
            
            if (*next + 1 < max) {
                    memory[(*next)++] = me;
            } else { printf("error, ran out of memory.\n"); }
            
            printf("continuing to next parent...\n");
        }
    }
    printf("[failing signature][out]\n");
    return 0;
}

static nat csr(string given, res_array memory, nat max) {
    nat next = 2, head = 1, tail = 1;
    for (; head; head = memory[head].queue_next) {
        printf("------------ moving on to head = %lu --------------\n", head);
        debug_memory(memory, head, tail, next, max);
        printf("trying indexes for head = %lu...\n", head);
        for (; memory[head].index < context.name_count; memory[head].index++) {
            printf("\n\ntrying index:  %lu \n", memory[head].index);
            if (matches(head, given, memory, max, &head, &next, &tail)) {
                printf("found a match!\n");
                return head;
            } else {
                printf("didnt match, or found argument, trying next index...\n");
            }
        }
        printf("no index solved it. moving onto next head...\n");
    }
    printf("[NO MATCH FOUND]\n");
    if (next + 1 == max) printf("----> ERROR, ran out of memory!\n");
    return 0;
}














static bool compacted_matches(nat n, string given, res_array memory,
                              nat max, nat* head, nat* next, nat* tail) {
    for (; n; n = memory[n].parent) {
        struct resolved me = memory[n];
        struct name name = context.names[me.index];
        for (; me.done < name.length; ) {
            nat c = name.signature[me.done++];
            if (c >= 256) {
                if (*next + 2 < max) {
                    struct resolved child = {0};
                    child.begin = me.begin;
                    child.parent = *next + 1;
                    me.args[me.count++] = *next;
                    memory[*tail].queue_next = *next; *tail = *next;
                    memory[(*next)++] = child;
                    memory[(*next)++] = me;
                }
                return 0;
            } else if (c != (nat) given[me.begin]) {
                return 0;
            }
            me.begin++; best(me.begin);
        }
        nat p = me.parent;
        if (!p) return me.begin == strlen(given);
        memory[p].begin = me.begin;
        memory[p].args[memory[p].count - 1] = *next;
        if (*next + 1 < max) memory[(*next)++] = me;
    }
    return 0;
}

static nat compacted_csr(string given, res_array memory, nat max) {
    nat next = 2, head = 1, tail = 1;
    for (; head; head = memory[head].queue_next) {
        for (; memory[head].index < context.name_count; memory[head].index++) {
            if (compacted_matches(head, given, memory, max, &head, &next, &tail)) {
                printf("used   %lu / %lu   memory cells.\n", next, max);
                return head;
            }
        }
    }
    if (next + 1 == max) printf("----> ERROR, ran out of memory!\n");
    return 0;
}


static void print_result(string input, res_array memory, nat s) {
    
    while (s) {
        nat p = memory[s].parent;
        if (p) s = p; else break;
    }
    
    printf("\n\n\n\nsolution = %lu\n", s);
    debug_resolved(memory, s, 0);
    printf("\n\n\n\n");
    if (!s) {
        printf("error: @ %lu: unexpected %c%s\n",
               context.best,
               input[context.best],
               input[context.best] ? "" : "end of expression");
        display_string_at_char(input, context.best);
    } else printf("\n\t [parse successful]\n\n");
}

static void test_csr_with_string(string input, nat size) {
    res_array memory = calloc(size, sizeof(struct resolved));
    nat solution = compacted_csr(input, memory, size);
    print_result(input, memory, solution);
//    debug_memory(memory, 0, 0, 0, size);
    free(memory);
}

static void push_signature(string string) {
    const nat n = context.name_count, length = strlen(string);
    context.names = realloc(context.names, sizeof(struct name) * (context.name_count + 1));
    context.names[n].signature = calloc(length, sizeof(nat));
    for (nat i = 0; i < length; i++)
        context.names[n].signature[i] = string[i] != '_' ? string[i] : 256;
    context.names[n].length = length;
    context.name_count++;
}

static void construct_context(string* signatures) {
    for (nat i = 0; signatures[i]; i++)
        push_signature(signatures[i]);
}

static void destroy_context() {
    for (nat i = 0; i < context.name_count; i++)
        free(context.names[i].signature);
    free(context.names);
    context = (struct context){0};
}

void start() {
    string names[] = {"-", "j__", "h", "", 0};
    construct_context(names);
    debug_context();
    test_csr_with_string("jhjhjhjhjhjhjhjhjhjhjhjhjhjhjhjhjhjhjhjhjhjh", 10000000);
    destroy_context();
}
