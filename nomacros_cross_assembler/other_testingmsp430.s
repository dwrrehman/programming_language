lf foundation.s 

set _targetarchitecture msp430_arch 
set _outputformat ti_txt_executable

df nonzero set nonzero 0

df my_address set my_address 1 si my_address 14

section my_address
df loop at loop
br4 nonzero loop


