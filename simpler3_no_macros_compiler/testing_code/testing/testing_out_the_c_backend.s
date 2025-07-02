(testing out the c backend of the compiler that i just finished implementing lol,  1202506194.204004 by dwrr)

file library/foundation.s ct

st compiler_target c_arch nat
st compiler_format c_source_output nat
st compiler_should_overwrite true nat

set my_constant 1001

rt 

register a 1 
set a 101
register b 01 
set b 011
add a b
add a my_constant

halt

