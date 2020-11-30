# makefile for my compiler.

# disabled_warnings = -Wno-poison-system-directories 
# -Wno-nullability-completeness -Wno-nullability-extension 
# -Wno-padded -Wno-sign-conversion -Wno-implicit-int-conversion 
# -Wno-reserved-id-macro -Wno-undef

# -Wall 
# -Wextra -Wpedantic -Wno-nullability-completeness -Wno-nullability-extension -Wno-error 
# -Weverything $(disabled_warnings) 

disabled_warnings = -Wno-documentation-unknown-command

warning_flags = -Wall -Wextra -Wpedantic -Weverything $(disabled_warnings) 

debug_flags = -fsanitize=address,undefined

libraries = `llvm-config --cflags --ldflags --libs core irreader linker executionengine interpreter --system-libs` -lc++ -lffi

n: source/n/n.c 
	clang -g -O1 $(warning_flags) $(debug_flags) source/n/n.c -o n $(libraries)

release: source/n/n.c 
	clang -Ofast $(warning_flags) source/n/n.c -o n $(libraries)

clean:
	rm -rf n
