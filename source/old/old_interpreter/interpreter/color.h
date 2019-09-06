//
//  color.h
//  sandbox6
//
//  Created by Daniel Rehman on 1902074.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef color_h
#define color_h


#define use_colored_output 1


#if use_colored_output
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define BRIGHT_GREEN "\x1B[92m"
#define BRIGHT_RED "\x1B[091m"
#define BRIGHT_BLUE "\x1B[94m"
#define BRIGHT_YELLOW "\x1B[93m"
#define BRIGHT_MAGENTA "\x1B[95m"
#define BRIGHT_CYAN "\x1B[96m"
#define GRAY "\x1B[90m"
#define LIGHTGRAY "\x1B[37m"
#define RESET "\x1B[0m"
#define BOLDBLACK   "\033[1m\033[30m"
#define BOLDRED     "\033[1m\033[31m"
#define BOLDGREEN   "\033[1m\033[32m"
#define BOLDYELLOW  "\033[1m\033[33m"
#define BOLDBLUE    "\033[1m\033[34m"
#define BOLDMAGENTA "\033[1m\033[35m"
#define BOLDCYAN    "\033[1m\033[36m"
#define BOLDWHITE   "\033[1m\033[37m"
#define BOLDLIGHTGRAY "\033[1m\x1B[37m"

#else
#define RED ""
#define GREEN ""
#define YELLOW ""
#define BLUE ""
#define MAGENTA ""
#define CYAN ""
#define WHITE ""
#define BRIGHT_GREEN ""
#define BRIGHT_RED ""
#define BRIGHT_BLUE ""
#define BRIGHT_YELLOW ""
#define BRIGHT_MAGENTA ""
#define BRIGHT_CYAN ""
#define GRAY ""
#define LIGHTGRAY ""
#define RESET ""
#define BOLDBLACK   ""
#define BOLDRED     ""
#define BOLDGREEN   ""
#define BOLDYELLOW  ""
#define BOLDBLUE    ""
#define BOLDMAGENTA ""
#define BOLDCYAN    ""
#define BOLDWHITE   ""
#define BOLDLIGHTGRAY ""
#endif

#endif /* color_h */
