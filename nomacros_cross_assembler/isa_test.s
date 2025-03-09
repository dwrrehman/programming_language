. testing out the machine code generation for all the instructions we have implemented so far, for arm64. .
. written on 1202503097.054017 dwrr . 

lf foundation.s

set _targetarchitecture arm64_arch

nop svc

mov 4 42 shift_none type_zero width64 
mov 4 42 shift_16 type_zero width64 
mov 4 42 shift_32 type_zero width64 
mov 4 42 shift_48 type_zero width64 

mov 1 42 shift_none type_zero width64 
mov 2 42 shift_none type_keep width64 
mov 3 42 shift_none type_neg width64 

mov 1 42 shift_none type_zero width32
mov 2 42 shift_none type_keep width32
mov 3 42 shift_none type_neg width32

nop

df nolink set nolink 0
df jumplink set jumplink 1

df label 
jmp jumplink label
jmp nolink label

nop

bc cond_always label
bc is_equal label
bc is_unsigned_less label
bc is_unsigned_greater_or_equal label

nop

df isreturn set isreturn 1
df isnotreturn set isnotreturn 0

. br 4 jumplink isreturn    jumplink is ignored if isreturn. .
br 4 nolink isreturn
br 4 jumplink isnotreturn
br 4 nolink isnotreturn

nop

df should_shift12 set should_shift12 1

df positive set positive 0
df negative set negative 1
df setflags set setflags 1
df noflags set noflags 0

addi 4 5 42 should_shift12 setflags negative width64
addi 4 5 42 should_shift12 noflags negative width64
addi 4 5 42 should_shift12 setflags positive width64
addi 4 5 42 should_shift12 noflags positive width64
nop
addi 4 5 42 should_shift12 setflags negative width32
addi 4 5 42 should_shift12 noflags negative width32
addi 4 5 42 should_shift12 setflags positive width32
addi 4 5 42 should_shift12 noflags positive width32
nop

zero should_shift12

addi 4 5 42 should_shift12 setflags negative width64
addi 4 5 42 should_shift12 noflags negative width64
addi 4 5 42 should_shift12 setflags positive width64
addi 4 5 42 should_shift12 noflags positive width64
nop
addi 4 5 42 should_shift12 setflags negative width32
addi 4 5 42 should_shift12 noflags negative width32
addi 4 5 42 should_shift12 setflags positive width32
addi 4 5 42 should_shift12 noflags positive width32
nop


at label

nop
nop




