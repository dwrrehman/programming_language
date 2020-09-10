/// Daniel Rehman, CE202008285.121444
/// this is my second try at making a more memory efficient bfs csr algorithm.
/// i reworked the data structures a ton, and am making things simpler, hopefully.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef size_t nat;
typedef const char* string;

static string context = NULL, input = NULL;
static nat* memory = NULL, best = 0, size = 10;

static void debug_memory(nat head, nat tail) {
    printf("-------- memory ----------\n");
    for (nat i = 0; i < size; i++) {
        printf("%5lu   %c%c  {  i:%-5lu p:%-5lu b:%-5lu  }\n",
               i * 3,
               i * 3 == head ? 'H' : ' ',
               i * 3 == tail ? 'T' : ' ',
               memory[i * 3 + 0], memory[i * 3 + 1], memory[i * 3 + 2]);
    }
    printf("-------------------------\n");
}

static void display_signature(nat at) {
    printf("\n         signature: ");
    for (nat i = 0; i < strlen(context) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", i == strlen(context) ? '?' : context[i]);
    puts("\n");
}

static void display_string_at_char(nat at) {
    printf("\n            string:  ");
    for (nat i = 0; i < strlen(input) + 1; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    puts("\n");
}

static void debug_context() {
    printf("------ context ------\n");
    for (nat i = 0; i < strlen(context); i++) {
        printf("%c ", context[i]);
        if (context[i] == '.') printf("\n%lu: ", i);
    }
    printf("---------------------\n");
}

static void print_solution(nat s) {
    printf("\n\t%lu\n\n", s);
    if (!s) {
        printf("error: @%lu: unexpected %c\n", best, input[best]);
        display_string_at_char(best);
    } else printf("\n\t [parse successful]\n\n");
}

static nat solve() {
    for (nat head = 0, tail = 3; head != tail; head += 3) {
        for (nat index = memory[head]; index < strlen(context); index++) {
            for (nat at = head; at; at = memory[at + 1]) {
                nat parent = memory[at + 1], begin = memory[at + 2], bad = 0;
                for (; context[index] != '.'; index++) {
                    if (bad) continue;
                    else if (context[index] == '_') {
                        memory[tail++] = index;
                        memory[tail++] = parent;
                        memory[tail++] = begin;
                        bad = 1;
                    } else if (context[index] == input[begin]) begin++;
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

void start() {
    context = "-.hello.bubbles_there.";
    input = "bubbleshellothere";
    memory = malloc(30 * sizeof(nat));
    debug_context();
    print_solution(solve());
    debug_memory(0, 0);
    free(memory);
}

