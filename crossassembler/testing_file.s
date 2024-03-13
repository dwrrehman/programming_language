0 library/targets define include
0 library/ascii define include

arm64 settarget
machoexecutable setoutputformat
object.o define setobjectname 
program.out define setexecutablename
false preserveexistingexecutable
false preserveexistingobject

01 11 001 addi
01 11 001 add

0 0 0 add
0 0 0 addi


ecall

true enabledebugoutput
eof 























the following code prints out a message at compile time:




 library/targets define include
0 library/ascii define include
noruntime settarget
printbinary setoutputformat

1 c define 
01 lower define 
000001 lower ctli
lower space define 

'H' c ctli lower c ctadd ctput
'I' c ctli lower c ctadd ctput
space ctput

'H' c ctli lower c ctadd ctput
'O' c ctli lower c ctadd ctput
'W' c ctli lower c ctadd ctput
space ctput

'A' c ctli lower c ctadd ctput
'R' c ctli lower c ctadd ctput
'E' c ctli lower c ctadd ctput
space ctput

'Y' c ctli lower c ctadd ctput
'O' c ctli lower c ctadd ctput
'U' c ctli lower c ctadd ctput
'\n' c ctli ctput


'I' c ctli lower c ctadd ctput
space ctput

'A' c ctli lower c ctadd ctput
'M' c ctli lower c ctadd ctput
space ctput

'P' c ctli lower c ctadd ctput
'R' c ctli lower c ctadd ctput
'E' c ctli lower c ctadd ctput
'T' c ctli lower c ctadd ctput
'T' c ctli lower c ctadd ctput
'Y' c ctli lower c ctadd ctput
space ctput

'G' c ctli lower c ctadd ctput
'O' c ctli lower c ctadd ctput
'O' c ctli lower c ctadd ctput
'D' c ctli lower c ctadd ctput
'\n' c ctli ctput









