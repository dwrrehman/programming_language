# makefile for my compiler.

disabled_warnings = -Wno-documentation-unknown-command

warning_flags = -Wall -Wextra -Wpedantic -Weverything $(disabled_warnings) 

debug_flags = -fsanitize=address,undefined

libraries = `llvm-config --cflags --ldflags --libs core irreader linker executionengine interpreter --system-libs` -lc++ -lffi

compile: source/main.c 
	clang -g -O1 $(warning_flags) $(debug_flags) source/main.c -o compile $(libraries)

release: source/main.c 
	clang -Ofast $(warning_flags) source/main.c -o compile $(libraries)

clean:
	rm -rf compile
	rm -rf compile.dSYM
