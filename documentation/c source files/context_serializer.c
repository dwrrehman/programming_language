#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


static inline void print(int* vector, int length) {
	printf("(%d){ ", length);
	for (int i = 0; i < length; i++) 
		printf("%d ", vector[i]);
	printf("}\n");
}

static inline void use_context(int alphabet, int index_count, int context_count, int* context, int* indicies) {

	printf("context indexes: (index_count = %d, context_count = %d, alphabet = %d)\n", 
		index_count, context_count, alphabet);

	for (int i = 0; i < index_count; i++) {
		int index = indicies[i];
		int* n = context + index;
		printf("i=%d index=%d  |  (length=%d) [ ", i, index, n[0]);
		for (int j = 0; j <= n[0]; j++) {
			int c = n[j + 1];
			if (c < 33) printf(" char{%d} ", c);
			else if (c < 128) printf("%c ", c);
			else if (c < 256) printf(" unicode{%d} ", c);
			else printf(" (%d) ", c);
		}
		printf(" ] \n");
		
	}
	printf("-----------------------------\n\n");
}

static inline void define(int name_length, int* name, int* context_count, int* index_count, int* context, int* indicies) {
	int place = (*index_count);
	while (place and name_length < context[indicies[place - 1]]) place--;
	if (*index_count) memmove(indicies + place + 1, indicies + place, sizeof(int) * (size_t) (*index_count - place));
	indicies[place] = (*context_count);
	(*index_count)++;
	context[(*context_count)++] = name_length;
	for (int i = 0; i <= name_length; i++) {
		int ind = name[i];
		context[(*context_count)++] = ind < 248 ? ind : ind;
	}
}

static inline void serialize(const char* filename, int alphabet, 
				int* indicies, int index_count, 
				int* context, int context_count) {
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0) {
		printf("error: %s: ", "filename");
		perror("open");
		exit(1);
	}

	write(fd, &index_count, sizeof(int));
	write(fd, &context_count, sizeof(int));
	write(fd, &alphabet, sizeof(int));

	write(fd, indicies, (size_t) index_count * sizeof(int));
	write(fd, context, (size_t) context_count * sizeof(int));
	
	close(fd);
}


static inline void* open_file(const char* filename, int* length) {

	struct stat file_data = {0};
	const int file = open(filename, O_RDONLY);

	if (file < 0 or stat(filename, &file_data) < 0) {
		fprintf(stderr, "error: %s: ", filename);
		perror("open");
		exit(3);
	}

	*length = (int) file_data.st_size;
	if (not *length) return NULL;
	void* input = mmap(0, (size_t) *length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) {
		fprintf(stderr, "error: %s: ", filename);
		perror("mmap");
		exit(4);
	}
	close(file);
	return input;
}

int main() {


if ((1))

	{
		printf("creating the init context...\n");

		int* indicies = malloc(sizeof(int) * 65536);
		int* context = malloc(sizeof(int) * 65536);
		int index_count = 0, context_count = 0;
		int name[2048] = {0};

		const int alphabet = 650;

		const int type_type = 650;
		const int none_type = 656;
		const int unit_type = 662;
		const int name_type = 668;
		const int any_type = 674;
		
		for (int i = 33; i < 248; i++) {
			if (i == 127) continue;
			if (i == '(' or i == ')') {
				name[0] = i;
				name[1] = 'z';
				name[2] = '3';
				name[3] = 'n';
				name[4] = 'q';
				name[5] = name_type;
				define(5, name, &context_count, &index_count, context, indicies);
			} else {
				name[0] = i;
				name[1] = name_type;
				define(1, name, &context_count, &index_count, context, indicies);
			}
		}
		define(4, (int[]){'t','y','p','e', type_type}, &context_count, &index_count, context, indicies);
		define(4, (int[]){'n','o','n','e', type_type}, &context_count, &index_count, context, indicies);
		define(4, (int[]){'u','n','i','t', type_type}, &context_count, &index_count, context, indicies);
		define(4, (int[]){'n','a','m','e', type_type}, &context_count, &index_count, context, indicies);
		define(3, (int[]){'a','n','y', type_type}, &context_count, &index_count, context, indicies);
		
		define(5, (int[]){'d','e','f', name_type, any_type, unit_type}, &context_count, &index_count, context, indicies);
		define(6, (int[]){'u','n','d','e','f', any_type, unit_type}, &context_count, &index_count, context, indicies);
		define(6, (int[]){'j','o','i','n', unit_type, unit_type, unit_type}, &context_count, &index_count, context, indicies);

		for (int i = 0; i < 32; i++) {
			int name_length = 0;
			name[name_length++] = '(';
			for (int n = 0; n < i ; n++) name[name_length++] = name_type;
			name[name_length++] = type_type;
			name[name_length++] = ')';
			name[name_length] = name_type;
			define(name_length, name, &context_count, &index_count, context, indicies);
		}

		printf("context: ");
		print(context, context_count);
		printf("indexes: ");
		print(indicies, index_count);
		use_context(alphabet, index_count, context_count, context, indicies);
		
		printf("serializing %d indicies and %d size context to file...\n", index_count, context_count);
		serialize("init.i", alphabet, indicies, index_count, context, context_count);
		
		exit(0);
	}

else
 
	{
		printf("loading the context: init.i...\n");

		int length = 0;
		int* base = open_file("init.i", &length);

		int index_count = base[0];
		int context_count = base[1];
		int alphabet = base[2];

		int* indicies = malloc(sizeof(int) * 65536);
		memcpy(indicies, base + 3, sizeof(int) * (size_t) index_count);

		int* context = malloc(sizeof(int) * 65536);
		memcpy(context, base + index_count + 3, sizeof(int) * (size_t) context_count);

		printf("loaded: \n");
		use_context(alphabet, index_count, context_count, context, indicies);

		exit(0);
	}
}












// static inline void print_context(int* base, int context_count) {

// 	int index_count = *base;
// 	int alphabet = *(base + 1);

// 	int* indicies = base + 2;
// 	int* context = base + 2 + index_count;
// 	int end = context_count - (index_count + 2);

// 	printf("DEBUG: context_count = %d\n", context_count);
// 	printf("DEBUG: context = %p\n", (void*) context);
// 	printf("context indexes: (%d, alphabet=%d) \n", index_count, alphabet);
	
// 	printf("indicies = (%d){ ", index_count);
// 	for (int i = 0; i < index_count; i++) 
// 		printf("%d ", indicies[i]);
// 	printf("}\n");

// 	printf("\ncontext: \n");
// 	for (int i = 0; i < end; ) {
// 		int* n = context + i;
// 		printf("%d | (def=%d)(length=%d) [ ", i, n[0], n[1]);
// 		for (int j = 0; j <= n[1]; j++) {
// 			int c = n[j + 2];
// 			if (c < 33) printf(" char{%d} ", c);
// 			else if (c >= 992) printf(" (%d) ", c);
// 			else putchar(c);
// 		}
// 		printf(" ] \n");
// 		i += n[1] + 3;
		
// 	}
// 	printf("-----------------------------\n\n");
// }

// static inline void use_context(int* base) {

// 	int index_count = *base;
// 	int alphabet = *(base + 1);
// 	int* indicies = base + 2;
// 	int* context = base + 2 + index_count;

// 	printf("context indexes: (%d, alphabet=%d) \n", index_count, alphabet);

// 	for (int i = 0; i < index_count; i++) {
// 		int index = indicies[i];
// 		int* n = context + index;
// 		printf("i=%d index=%d  |  (def=%d)(length=%d) [ ", i, index, n[0], n[1]);
// 		for (int j = 0; j <= n[1]; j++) {
// 			int c = n[j + 2];
// 			if (c < 33) printf(" char{%d} ", c);
// 			else if (c < 128) printf("%c ", c);
// 			else if (c < 256) printf(" unicode{%d} ", c);
// 			else printf(" (%d) ", c);
// 		}
// 		printf(" ] \n");
		
// 	}
// 	printf("-----------------------------\n\n");
// }

// int main() {
// 	int context_count = 0;
// 	int* context = malloc(sizeof(int) * 65536);

// 	// context int count = 864         if a signature has index >= 864, its a param.

// 	//  ie, the alphabet value, that we need to put at the beginning 
// 	//  of the context, right after the index count.
// 	//  is going to be 864.

// 	int index_count = 214; // 248 - 33 - 1    

// 	int alphabet_start = 864; // 860 + 4

// 	// remove inprntable chars: space, ctrl chars, del char, 
// 	// and also remove the unicode chars that arent used in utf8: 0xF8 through 0xFF.

// 	context[context_count++] = index_count;
// 	context[context_count++] = alphabet_start;

// 	for (int i = 0; i < index_count + 1; i++) {
// 		if (i + 33 <= '(') context[context_count++] = i * 4;
// 		else if (i + 33 == ')') { context[context_count++] = (i + 1) * 4; }
// 		else if (i + 33 > 127) {
// 			int k = (i + 1) * 4;
// 			context[context_count++] = k;
// 		} else if (i + 33 > ')') {
// 			int k = (i + 2) * 4;
// 			if (i + 33 != 127) context[context_count++] = k;
// 		}
// 	}

// 	for (int i = 0; i < index_count + 1; i++) {
// 		if (i + 33 == '(' or i + 33 == ')') { 
// 			context[context_count++] = 0;
// 			context[context_count++] = 5;
// 			context[context_count++] = i + 33;
// 			context[context_count++] = 'z';
// 			context[context_count++] = '5';
// 			context[context_count++] = '#';
// 			context[context_count++] = 'Q';
// 			context[context_count++] = alphabet_start + 6;
// 		} else if (i + 33 != 127) {
// 			context[context_count++] = 0;
// 			context[context_count++] = 1;
// 			context[context_count++] = i + 33;
// 			context[context_count++] = alphabet_start + 6;
// 		}
// 	}

// 	context = realloc(context, sizeof(int) * (size_t) context_count);

// 	printf("RAW context: \n"); 
// 	print(context, context_count);
// 	printf("\n\n");
	
// 	printf("pretty print: \n\n");
// 	print_context(context, context_count);
	
// 	printf("used context via indicies:\n");
// 	use_context(context);
	
// }









// // static inline void serialize(i8* context, size_t context_length) {
// // 	int fd = open("context.ctx", O_WRONLY | O_CREAT | O_TRUNC);
// // 	if (fd < 0) {
// // 		printf("error: %s: ", "filename");
// // 		perror("open");
// // 		exit(1);
// // 	}
// // 	write(fd, context, context_length);
// // 	close(fd);
// // }



// // static inline void* open_file(const char* filename, int* length) {

// // 	struct stat file_data = {0};
// // 	const int file = open(filename, O_RDONLY);

// // 	if (file < 0 or stat(filename, &file_data) < 0) {
// // 		fprintf(stderr, "error: %s: ", filename);
// // 		perror("open");
// // 		exit(3);
// // 	}

// // 	*length = (int) file_data.st_size;
// // 	if (not *length) return NULL;
// // 	void* input = mmap(0, (size_t) *length, PROT_READ, MAP_SHARED, file, 0);
// // 	if (input == MAP_FAILED) {
// // 		fprintf(stderr, "error: %s: ", filename);
// // 		perror("mmap");
// // 		exit(4);
// // 	}
// // 	close(file);
// // 	return input;
// // }





// 	// nat x = 0x0FFF0776;
// 	// i8* data = (i8*) (intptr_t) &x;
// 	// for (int i = 0; i < 4; i++) {
// 	// 	printf("0x%hhx ", data[i]);
// 	// }
// 	// printf("\n");




// // static inline void print_program(int* program, int* context, int p, int depth) {
// // 	for (int i = 0; i < depth; i++) printf(".   ");
// // 	int index = program[p], count = program[p + 1];
// // 	printf("p=%d:  [i=%d] : (c=%d) : ", p, index, count);
// // 	fflush(stdout);
// // 	print(context + index + 2, context[index + 1]);
// // 	puts("");
// // 	for (int i = 0; i < count; i++) {
// // 		print_program(program, context, program[p + i + 2], depth + 1);
// // 	}
// // }



// /*
// nat index_count = * (nat*) (intptr_t) base;
// 	nat* indicies = (nat*) (intptr_t) (base + sizeof(nat));
// 	i8* context = base + (index_count + 1) * sizeof(nat);
// 	nat start = (index_count + 1) * sizeof(nat);
// 	nat end = length - start;

// 	// printf("length = %d\n", length);
// 	// printf("start = %d\n", start);
// 	// printf("end = %d\n", end);
// 	// printf("base = %p\n", (void*) base);
// 	// printf("indicies = %p\n", (void*) indicies);
// 	// printf("context = %p\n", (void*) context);

// 	printf("indicies = (%d){ ", index_count);
// 	for (nat i = 0; i < index_count; i++) 
// 		printf("%d ", indicies[i]);
// 	printf("}\n");
// 	printf("\ncontext: \n");
// 	for (nat i = 0; i < end; ) {
// 		i8* n = context + i;
// 		printf("%d | (length=%d) [ ", i, *n);
// 		for (nat j = 0; j < *n + 2; j++) {
// 			i8 c = n[j];
// 			if (c < 33) printf(" (%d) ", c);
// 			else putchar(c);
// 		}
// 		printf(" ] \n");
// 		i += *n + 2;
// 		// if (!(i % 8)) printf("\n");
// 		// printf("0x%x(%d)[%c] ", context[i], context[i], context[i]);
// 	}
// 	// printf("-----------------------------\n\n");

// */


// // if (!(i % 8)) printf("\n");
// 		// printf("0x%x(%d)[%c] ", context[i], context[i], context[i]);


// // printf("serializing...\n");
// 	// serialize(context, sizeof context);
// 	// printf("done.\n");






/*



static inline void define(int name_length, int* name, int* context_count, int* index_count, int* context, int* indicies) {

	int place = (*index_count);

	// printf("place = %d\n", place);

	while (place and name_length < context[indicies[place - 1] + 1]) place--;

	// printf("chosen place = %d\n", place);

	// printf("memmove(%p, %p, %d)\n ", (void*) (indicies + place + 1), (void*) (indicies + place), *index_count - place);

	if (*index_count) memmove(indicies + place + 1, indicies + place, sizeof(int) * (size_t) (*index_count - place));

	// printf("placing...\n");

	indicies[place] = (*context_count);
	(*index_count)++;

	// printf("reading name length: %d\n", name_length);

	context[(*context_count)++] = 0xFFFF;
	context[(*context_count)++] = name_length;

	for (int i = 0; i <= name_length; i++) {
		int ind = name[i];
		// printf("reading name literal: %d\n", ind);
		context[(*context_count)++] = ind < 248 ? ind : ind; // context[ind + 2]
	}
}


*/


