# makefile for my compiler.

#disabled_warnings = -Wno-poison-system-directories -Wno-nullability-completeness -Wno-nullability-extension -Wno-padded -Wno-sign-conversion -Wno-implicit-int-conversion -Wno-reserved-id-macro -Wno-undef

warning_flags = -w # -Wall # -Wextra -Wpedantic -Wno-nullability-completeness -Wno-nullability-extension -Wno-error # -Weverything $(disabled_warnings) 

debug_flags = -fsanitize=address,undefined

include_flags = `llvm-config --cflags`
linker_flags = `llvm-config --ldflags`
libraries = `llvm-config --libs core irreader linker executionengine interpreter --system-libs` -lm -lffi

n: source/n/n.c 
	clang -g -O1 $(warning_flags) $(debug_flags) $(include_flags) source/n/n.c -o n $(linker_flags) $(libraries)

release: source/n/n.c 
	clang -Ofast $(warning_flags) $(include_flags) source/n/n.c -o n $(linker_flags) $(libraries)

clean:
	rm -rf n
