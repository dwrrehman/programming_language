#include <stdio.h>

struct da_int {
    short i;
};

struct da_int add(struct da_int x, struct da_int y) {
    struct da_int rv = {x.i + y.i};
    return rv;
}

int main(int argc, char *argv[]) {
    struct da_int x = {5}, y = {3};
    printf("%d\n", add(x, y).i);
}
