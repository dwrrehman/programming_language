#!/bin/sh
#
# Marco Buzzanca © 2016. MIT License.
# Originarilly written by Tom Hale © 2016. MIT Licence.
#
# Print out 256 colors, with each number printed in its corresponding color.
#
# Compared to the original script this one also prints the first 16 colors in
# bright/bold, to check the interaction between bold and the ANSI colors. This
# script is also POSIX compliant. Unfortunately this meant I had to use
# subshells for print_line and print_blocks because POSIX does not define
# local.

# Fail on errors or undeclared variables
set -eu

# Format strings
normal_fmt='\033[38;5;%dm%3d\033[0m'
bold_fmt='\033[1;38;5;%dm%3d\033[0m'

# Prints a line of colors from $2 to $3
# Disable the "don't use variables in printf" warning. This is intended.
# shellcheck disable=SC2059
print_line() (
    format=$1
    offset=$2
    stop=$3

    while [ "$offset" -lt "$stop" ]; do
        printf "$format " "$offset" "$offset"
        offset=$((offset + 1))
    done

    exit 0
)

# Print $4 $2x$3 blocks of colors starting from $1
print_blocks() (
    base_offset=$1
    rows=$2
    cols=$3
    blocks=$4

    block_size=$((rows * cols))
    stop=$((base_offset + rows * cols))

    while [ "$base_offset" -lt "$stop" ]; do
        line_offset=$((base_offset))
        line_stop=$((line_offset + block_size * blocks));

        while [ "$line_offset" -lt "$line_stop" ]; do
            print_line "$normal_fmt" "$line_offset" $((line_offset + cols))
            line_offset=$((line_offset + block_size))

            # Obey the 80 columns rule
            [ "$line_offset" -lt "$line_stop" ] && printf "    "
        done

        printf "\n"
        base_offset=$((base_offset + cols))
    done

    exit 0
)

# Normal colors
printf "Normal  "
print_line "$normal_fmt" 0 16
printf "\n\033[1mBold\033[0m    "
print_line "$bold_fmt" 0 16
printf "\n\n"

# Three 6x6 color cubes
print_blocks 16 6 6 3
printf "\n"
print_blocks 124 6 6 3
printf "\n"

# Not 50, but 24 Shades of Grey
print_blocks 232 2 6 2