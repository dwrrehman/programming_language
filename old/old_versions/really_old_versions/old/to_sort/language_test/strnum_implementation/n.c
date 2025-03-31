// the "convert string to number" function that will be used in my interpreter, for my language.
// written on 2209014.193511 by dwrr.
// notes
// 	- numbers are written out in reverse digit order, because its more correct to do it that way. 
// 	- the function can handle up to bases of 96, then runs out of ascii characters to express the number. 
// 	- deals with 64 bit numbers only. 
//	- usage: provide a string, and a pointer to a 64-bit variable which holds its length. 
//	- the return result is always the number that is most represents the string, 
// 		until you find a non-represented character in the radix.
//	- the first character in the string determines the radix of the number representation. 
//           eg,   "a" = decimal, "2" = binary, "g" = hexidecimal, "1" = unary, "3" = ternary, etc.
//
//                    for example,	string_to_number("a001", &len /* len holds 4 */)    	returns the 64-bit value 100.
//
//                    also,		string_to_number("20111", &len /* len holds 5 */)    	returns the 64-bit value 14.
//
//                    also,		string_to_number("3", &len /* len holds 1 */)    	returns the 64-bit value 0.
//
//                    also,		string_to_number("", &len /* len holds 0 */)    	returns the 64-bit value 0.
//
//                    also,		string_to_number("a96lol1", &len /* len holds 7 */)    	returns the 64-bit value 69, 
//												  and len is set to 3 by the call.
//


#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint64_t nat;
typedef uint8_t byte;

static const nat digit_count = 96;
static const char digits[96] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.;:-_=+/?!@#$%^&*()<>[]{}|\\~`\'\"";

static nat string_to_number(const char* string, nat* length) {
	byte radix = 0, value = 0;
	nat result = 0, index = 0, place = 1;
begin:	if (index >= *length) goto done;
	value = 0;
top:	if (value >= digit_count) goto found;
	if (digits[value] == string[index]) goto found;
	value++;
	goto top;
found:	if (index) goto check;
	radix = value;
	goto next;
check:	if (value >= radix) goto done;
	result += place * (nat) value;
	place *= radix;
next:	index++;
	goto begin;
done:	*length = index;
	return result;
}

int main(const int argc, const char** argv) {
	if (argc < 2) exit(puts("error"));
	nat length = strlen(argv[1]), save = length;
	printf("%s ==> %llu (len=%llu)\n", argv[1], string_to_number(argv[1], &length), length);
	if (save != length) puts("error: string not a number in the base.");
}
