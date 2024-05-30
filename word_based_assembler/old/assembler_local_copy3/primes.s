def ari . ari dup 
def . _comment_ .

arz def .reg0. ari def .reg1. 
ari def .reg2. ari def .reg3.
ari def .reg4. ari def .reg5.
ari def .reg6. ari def .reg7.
ari def .reg8. ari def .reg9.

reg2 def .print. reg1 def .exit.

reg1 def .index.

reg0 reg0 reg1 cte add
reg9 reg0 reg2 cte imm add

reg7 cte atr

reg2 reg0 reg3 cte imm add
print cte ecl

reg8 cte atr
reg3 cte bge
reg1 dup dup cte imm add
print cte ecl
reg7 reg2 reg1 cte blt
exit cte ecl



------------------------------------------

do ./run
primes.s

copy do ./build



copy do /bin/zsh
-c
./build && ./run primes.s



















reg4 reg0 reg5 def .count. ct addi
reg0 reg0 reg4 def .index. ct add
reg5 reg5 reg5 ct sll

reg7 def .label. ct atr

reg2 reg0 over ct addi
reg1 index dup ct addi 
over ct rem
index reg2 ct mul

reg6 swap reg0 ct beq
index print ct ecall
reg6 ct atr

label count index ct blt



all garbage from here:




debug dictionary ct ecall

4 ct get char ct ecall















1 def .ct exit.
2 def .debug print.
3 def .ct get char.
4 def .ct put char.
5 def .debug dictionary.
8 def .debug arguments.
6 def .debug instructions.
7 def .debug registers.


25 0 21 ct addi

5 21 21 ct addi

20 3 ct addi

3 21 21 ct mul

debug print ct ecall

ct exit ct ecall 



21 carg def .'A'.

debug print ct ecall



10 0 1 ct addi 
ct putchar ct ecall

'A' 0 1 ct addi
ct putchar ct ecall

'A' 0 1 ct addi
ct putchar ct ecall

10 0 1 ct addi
ct putchar ct ecall


ct exit ct ecall

















def _comment_

2405282.003534

	problem!!

	we actually need to rename the instructions to not be called    add   and  addi   etc  ...those mean we can't define a variable called i... so yeah. i think we need to use binary or something. not sure. pretty crazy. gosh. GRR

_comment_





