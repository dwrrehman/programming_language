(a file to test out the ref and deref instructions in the language!
i'm part way through implmenting macros, in a very extensible and 
flexible way, that is also quite simple to implement! yay!
written on 1202506194.011847 by dwrr)

file library/foundation.s
ct
st compiler_target rv32_arch nat
st compiler_format hex_array_output nat
st compiler_should_overwrite true nat

set constant 0000_001

rt 
register x 101
register y 011

ref hello x
ref bubbles y
ref var constant

deref hello  deref bubbles  deref var   r5_i r5_addi_op1 hello r5_addi_op2 bubbles var
deref hello deref bubbles  set hello bubbles 
deref hello deref var  add hello var

halt













































(set bubbles 1
deref hello
ref hello bubbles)

