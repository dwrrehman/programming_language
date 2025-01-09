#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdint.h>
typedef uint64_t nat;

enum {
	zero, incr, decr, not_, and_, or_, eor,
	set, add, sub, mul, div, lt, eq,   

};