"eof" 
"0" "1" "=" 

"set compiletime" 
"set runtime" 

"set architecture"  
"set output format" 
"preserve executable" 
"preserve object"
"set object name" 
"set executable name"

"disable debug" 
"enable debug"

"debug arguments" 
"debug instructions" 
"debug registers" 
"debug dictionary"

"ctdebug" 
"ctget" 
"ctput"
"ctdel"
"ctlast"
"ctarg"
"cttop"
"ctabort" 
"ctprint"

"ctat"
"add"  "sub"  "addi" 
"beq"  "bne" 
"bltu" "blt" 
"bgeu" "bge" 
"jalr" "jal"
"ecall"


0 = "no runtime"
1 = "riscv 32"
01 = "riscv 64"
11 = "arm 32"
001 = "arm 64"

0 = "print binary"
1 = "elf object"
01 = "elf executable"
11 = "macho object"
001 = "macho executable"

0 = "zr"
1 = "ra"
01 = "sp"

0101 = "a0"
1101 = "a1"
0011 = "a2"
1011 = "a3"
0111 = "a4"
1111 = "a5"
00001 = "a6"
10001 = "a7" "system call number"

1 = "system exit"     a0 "system exit code"
01 = "system fork"
11 = "system read"    a0 "system read fd"     a1 "system read buffer"      a2 "system read length"
001 = "system write"
101 = "system open"
011 = "system close"

0 = "standard in"
1 = "standard out"

0 = "false"
1 = "true"

disable debug
set runtime

