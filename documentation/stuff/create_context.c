{
		const char* spellings[] = {
			"\0\1",   // should be at index 127
  			"\4name\1", // stay
			"\5data0\1", // stay
			"\5decl\3\2", // after y256
			"\5def\3\2\2", // after 256
			"\6join\2\2\2", // after 256
			"\5data8\1", // stay
			"\6data16\1", // stay
			"\3nop\2",   // after 256.
			"\41(\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1)\3", // should at index. 255
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0", 
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0",
		NULL};

		// printf("%lu\n", sizeof(spellings) / 8);
		// abort();

		for (int i = 0; spellings[i]; i++) { 
			memcpy(context + S * index_count, spellings[i], (size_t) (spellings[i][0] + 2));
			macros[index_count++] = 0;
		}

		int k = (int) index_count;
		int limit = 256;

		for (int i = 33; i < limit; i++) {
			if (i == '(' or i == ')') {
				char string[11] = "\10n3zqx2l \1";
				string[8] = (char) i;
				memcpy(context + S * index_count, string, 11);
				macros[index_count++] = 0;
			} else {
				char string[4] = "\1 \1";
				string[1] = (char) i;
				memcpy(context + S * index_count, string, 4);
				macros[index_count++] = 0;
			}
		}

		{
			int count = 0;
			indicies[count++] = (i16) 0; 
			for (int i = k; i < k + (limit - 33); i++) {
				if (i == '(' or i == ')') continue;
				indicies[count++] = (i16) i;
			}
			indicies[count++] = '(';
			indicies[count++] = ')';
			for (int i = 1; i < k; i++) indicies[count++] = (i16) i; 
		}
		
	}
