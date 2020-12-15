# makefile for my compiler.

the_file = ./*.c
output_name = n

disabled_warnings = -Wno-documentation-unknown-command -Wno-poison-system-directories
warning_flags = -Wall -Wextra -Wpedantic -Weverything $(disabled_warnings) 

debug_flags = -fsanitize=address,undefined
linker_flags = -Xlinker -syslibroot -Xlinker `xcrun --sdk macosx --show-sdk-path`
libraries = `llvm-config --cflags --ldflags --libs --system-libs` -lc++ -lffi

$(output_name): $(the_file) 
	clang -g -O1 $(warning_flags) $(debug_flags) $(the_file) -o $(output_name) $(linker_flags) $(libraries)

release: $(the_file)
	clang -Ofast $(warning_flags) $(the_file) -o $(output_name) $(linker_flags) $(libraries)

clean:
	rm -rf $(output_name)
	rm -rf "$(output_name).dSYM"

