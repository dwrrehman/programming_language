

















// print_index(context, index, count);
		// printf("debug: index=%d current=%d top=%d begin=%d count=%d\n", index, current, top, begin, count);
		// print_vector(output, top + 16, context, count);









// _5:	if (context[index] == 32) goto _6;
// 	index++;
// 	goto _5; 
// _6:	index++;




// _3: 	if (context[index] == 32) goto _4;
//  	index++;
// 	goto _3;
// _4:	index++;












	// printf("%c\n", input[begin]);
	// if (input[begin] != '(')  {
	// 	printf("error: expected ( before name.\n");
	// 	top -= 4; index = output[top + 1];
	// 	goto try;
	// }
	// do begin++; while (begin < length and input[begin] < 33);

	// if (input[begin] != '\\') {
	// } else {
		// do begin++; while (begin < length and input[begin] < 33);
		// context[count++] = input[begin];
		// do begin++; while (begin < length and input[begin] < 33);
	// }






// 	const char* expected = output[top + 2] == -4 ? "init " : context + output[output[top + 2] + 1] + 1;
// 	const char* undefined = "undefined ", * copy_expected = expected;

// 	while (*undefined != ' ') {
// 		if (*undefined != *copy_expected) goto non;
// 		copy_expected++; undefined++;
// 	}

// 	while (begin < length and input[begin] != '.') {
// 		if (input[begin] != '\\') {
// 			if (input[begin] == ':') context[count++] = '\t';
// 			else if (input[begin] == '_') context[count++] = ' ';
// 			else context[count++] = input[begin];
// 			do begin++; while (begin < length and input[begin] < 33);
// 		} else {
// 			do begin++; while (begin < length and input[begin] < 33);
// 			context[count++] = input[begin];
// 			do begin++; while (begin < length and input[begin] < 33);
// 		}
// 	}
// 	context[count++] = '\n';
// 	if (count > best_count) best_count = count;

// 	do begin++; while (begin < length and input[begin] < 33);

// 	index = context_limit;

// 	// if (begin > best) { best = begin; candidate = index; }
	
// 	// printf("i found it!!! an undef param.\n");
// 	// printf("NOW NEW: CONTEXT: ::::%.*s::::\n", count, context);
// 	// printf("NEW SIG: begin = %d, index = %d, top = %d\n", begin, index, top);
// 	// print_vector(output, top + 4);

// 	goto done;

// non: 






	// printf("\n\n------------------- PARENT ------------------------\n\n");
	// printf("CURRENT CONTEXT: ::::%.*s::::\n", count, context);
	// printf("STATUS: begin = %d, index = %d, top = %d\n", begin, index, top);
	// print_vector(output, top + 4);
	// if (index < count) print_index(context, index);
	// else printf("\nERROR: could not print signature, because index == count!!!\n");
	// printf("continue? "); getchar();



// const int argc, const char** argv



