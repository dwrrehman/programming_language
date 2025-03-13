lf foundation.s 

set _targetarchitecture msp430_arch 
set _outputformat ti_txt_executable
set _shouldoverwrite true

. --------- stdlib stuff----------------- . 

. define all possible gen4 op codes here! . 

df msp_mov set msp_mov 4
. ....etc... .

 
. df is_not_equal4 set is_not_equal4 0
df is_equal4 set is_equal4 1
df is_equal4 set is_equal4 2
df is_not_equal4 set is_not_equal4 3
df is_not_equal4 set is_not_equal4 4 . 

. ...other branch conditions too... .


. 

	br4 cond label

	0 : JNE / JNZ
	1 : JEQ / JZ
	2 : JNC
	3 : JC
	4 : JN
	5 : JGE
	6 : JL
	7 : JMP
.


. 
	gen4  op(0)  dm(1) dr(2) di(3)  sm(4) sr(5) si(6)   size(7)

	4 : mov
	5 : add
	6 : addc
	7 : sub
	8 : subc
	9 : cmp
	10 : dadd
	11 : bit 
	12 : bic
	13 : bis
	14 : xor
	15 : and 

	1 : byte sized operation
	0 : word sized operation







. 



. --------- end of stdlib stuff----------------- . 

df my_address set my_address 1 si my_address 14
section my_address
	

	df loop at loop
	df max set max 4
	df i set i 0 df l cat l
		df j set j 0 df k cat k
			df op set op 7
			gen4 op   j 0 0   i 0 0   1		
			incr j lt j 2 k		
		incr i lt i max l		
	br4 7 loop
	
df my_address set my_address 1 si my_address 14
section my_address

	df loop at loop
	df max set max 4
	df i set i 0 df l cat l
		df j set j 0 df k cat k
			df op set op 7
			gen4 op   j 0 0   i 0 0   1		
			incr j lt j 2 k		
		incr i lt i max l		
	br4 7 loop









