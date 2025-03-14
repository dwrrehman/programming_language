. a program to blink an LED on the msp430fr2433 msp430 microcontroller, 
written on 1202503134.230344 by dwrr. . 

lf foundation.s 

set _targetarchitecture msp430_arch 
set _outputformat ti_txt_executable
set _shouldoverwrite true

. --------- stdlib stuff----------------- . 

df 00h set 00h 0
df 01h set 01h 1
df 02h set 02h 2
df 03h set 03h 3
df 04h set 04h 4
df 05h set 05h 5
df 06h set 06h 6
df 07h set 07h 7
df 08h set 08h 8
df 09h set 09h 9
df 0Ah set 0Ah 10
df 0Bh set 0Bh 11
df 0Ch set 0Ch 12
df 0Dh set 0Dh 13
df 0Eh set 0Eh 14
df 0Fh set 0Fh 15
df 10h set 10h 16

df 1000h set 1000h 01h si 1000h 12
df 0FFFh set 0FFFh 1000h decr 0FFFh

df 2000h set 2000h 02h si 2000h 12
df 1FFFh set 1FFFh 2000h decr 1FFFh

df 3000h set 3000h 03h si 3000h 12
df 2FFFh set 2FFFh 3000h decr 2FFFh

df 4000h set 4000h 04h si 4000h 12
df 3FFFh set 3FFFh 4000h decr 3FFFh


df C400h set C400h 0Ch si C400h 4 or C400h 04h si C400h 8
df FFFFh set FFFFh 1 si FFFFh 16 decr FFFFh
df 1800h set 1800h 01h si 1800h 4 or 1800h 08h si 1800h 8

df 19FFh  set 19FFh 01h 
si 19FFh 4 or 19FFh 09h 
si 19FFh 4 or 19FFh 0Fh 
si 19FFh 4 or 19FFh 0Fh 

df x  set x 00h 
si x 4 or x 01h 
si x 4 or x 00h 
si x 4 or x 00h 
df 0100h set 0100h x udf x

df x  set x 00h 
si x 4 or x 01h 
si x 4 or x 02h 
si x 4 or x 00h 
df 0120h set 0120h x udf x

df x  set x 00h 
si x 4 or x 01h 
si x 4 or x 04h 
si x 4 or x 00h 
df 0140h set 0140h x udf x

df x  set x 00h 
si x 4 or x 01h 
si x 4 or x 08h 
si x 4 or x 00h 
df 0180h set 0180h x udf x

df x  set x 00h 
si x 4 or x 01h 
si x 4 or x 0Ah 
si x 4 or x 00h 
df 01A0h set 01A0h x udf x

df x  set x 00h 
si x 4 or x 01h 
si x 4 or x 0Ch 
si x 4 or x 0Ch 
df 01CCh set 01CCh x udf x

df x  set x 00h 
si x 4 or x 02h 
si x 4 or x 00h 
si x 4 or x 00h 
df 0200h set 0200h x udf x

df x  set x 00h 
si x 4 or x 02h 
si x 4 or x 02h 
si x 4 or x 00h 
df 0220h set 0220h x udf x


df x  set x 00h 
si x 4 or x 03h 
si x 4 or x 00h 
si x 4 or x 00h 
df 0300h set 0300h x udf x

df x  set x 00h 
si x 4 or x 03h 
si x 4 or x 08h 
si x 4 or x 00h 
df 0380h set 0380h x udf x

df x  set x 00h 
si x 4 or x 06h 
si x 4 or x 06h 
si x 4 or x 00h 
df 0660h set 0660h x udf x

df x  set x 00h 
si x 4 or x 04h 
si x 4 or x 0Ch 
si x 4 or x 00h 
df 04C0h set 04C0h x udf x

df x  set x 00h 
si x 4 or x 07h 
si x 4 or x 00h 
si x 4 or x 00h 
df 0700h set 0700h x udf x

df x  set x 0Fh 
si x 4 or x 0Fh 
si x 4 or x 0Fh 
si x 4 or x 0Eh 
df FFFEh set FFFEh x udf x

. msp430fr2433 datasheet information and constants .
. (these should all be in a file called "msp430fr2433_data.s") .

df ram_start set ram_start 2000h
df ram_end set ram_end   2FFFh
df fram_start set fram_start C400h
df fram_end set fram_end   FFFFh
df ifram_start set ifram_start 1800h
df ifram_end set ifram_end   19FFh

df reset_vector set reset_vector FFFEh

df sfr_base set sfr_base 0100h
df pmm_base set pmm_base 0120h
df sys_base set sys_base 0140h
df cs_base set cs_base 0180h
df fram_base set fram_base 01A0h

df wdt_base set wdt_base 01CCh

df port_1/2_base set port_1/2_base 0200h
df port_3_base set port_3_base 0220h

df rtc_base set rtc_base 0300h
df timer0_base set timer0_base 0380h
df backupmem_base set backupmem_base 0660h

df mpy32_base set mpy32_base 04C0h
df adc_base set adc_base 0700h



. hardware registers in peripherals . 

df pmm_pmmctl0 set pmm_pmmctl0 00h add pmm_pmmctl0 pmm_base
df pmm_pmmctl1 set pmm_pmmctl1 02h add pmm_pmmctl1 pmm_base
df pmm_pmmctl2 set pmm_pmmctl2 04h add pmm_pmmctl2 pmm_base
df pmm_pmmifg  set pmm_pmmifg  0Ah add pmm_pmmifg pmm_base
df pmm_pm5ctl0 set pmm_pm5ctl0 10h add pmm_pm5ctl0 pmm_base

df cs_csctl0 set cs_csctl0 00h add cs_csctl0 cs_base
df cs_csctl1 set cs_csctl1 02h add cs_csctl1 cs_base
df cs_csctl2 set cs_csctl2 04h add cs_csctl2 cs_base
df cs_csctl3 set cs_csctl3 06h add cs_csctl3 cs_base
df cs_csctl4 set cs_csctl4 08h add cs_csctl4 cs_base
df cs_csctl5 set cs_csctl5 0Ah add cs_csctl5 cs_base
df cs_csctl6 set cs_csctl6 0Ch add cs_csctl6 cs_base
df cs_csctl7 set cs_csctl7 0Eh add cs_csctl7 cs_base
df cs_csctl8 set cs_csctl8 10h add cs_csctl8 cs_base

df fram_frctl0 set fram_frctl0 00h add fram_frctl0 fram_base
df fram_gcctl0 set fram_gcctl0 04h add fram_gcctl0 fram_base
df fram_gcctl1 set fram_gcctl1 06h add fram_gcctl1 fram_base

df wdt_wdtctl set wdt_wdtctl 00h add wdt_wdtctl wdt_base

df port1_p1in  set port1_p1in  00h add port1_p1in port_1/2_base
df port1_p1out set port1_p1out 02h add port1_p1out port_1/2_base
df port1_p1dir set port1_p1dir 04h add port1_p1dir port_1/2_base
df port1_p1ren set port1_p1ren 06h add port1_p1ren port_1/2_base

df port2_p2in  set port2_p2in  01h add port2_p2in port_1/2_base
df port2_p2out set port2_p2out 03h add port2_p2out port_1/2_base
df port2_p2dir set port2_p2dir 05h add port2_p2dir port_1/2_base
df port2_p2ren set port2_p2ren 07h add port2_p2ren port_1/2_base

df port3_p3in  set port3_p3in  00h add port2_p2ren port_3_base
df port3_p3out set port3_p3out 02h add port3_p3out port_3_base
df port3_p3dir set port3_p3dir 04h add port3_p3dir port_3_base
df port3_p3ren set port3_p3ren 06h add port3_p3ren port_3_base

df rtc_rtcctl set rtc_rtcctl  00h add rtc_rtcctl rtc_base
df rtc_rtciv  set rtc_rtciv   04h add rtc_rtciv rtc_base
df rtc_rtcmod set rtc_rtcmod  08h add rtc_rtcmod rtc_base
df rtc_rtccnt set rtc_rtccnt  0Ch add rtc_rtccnt rtc_base

df pc set pc 0
df sp set sp 1
df sr set sr 2
df cg set cg 3

.
editor macro sequence
fo copying the fields to make 
them defined as well, 
after doing the set, and df:

	ooaocapowu
.

df msp_mov set msp_mov 4
df msp_add set msp_add 5
df msp_addc set msp_addc 6
df msp_sub set msp_sub 7
df msp_subc set msp_subc 8
df msp_cmp set msp_cmp 9
df msp_dadd set msp_dadd 10
df msp_bit set msp_bit 11
df msp_bic set msp_bic 12
df msp_bis set msp_bis 13
df msp_xor set msp_xor 14
df msp_and set msp_and 15

df condjnz set condjnz 0
df condjz set condjz 1
df condjnc set condjnc 2
df condjc set condjc 3
df condjn set condjn 4
df condjge set condjge 5
df condjl set condjl 6
df condjmp set condjmp 7

df size_byte set size_byte 1
df size_word set size_word 0

df bit0 set bit0 1
df bit1 set bit1 2
df bit2 set bit2 4
df bit3 set bit3 8
df bit4 set bit4 16
df bit5 set bit5 32
df bit6 set bit6 64
df bit7 set bit7 128



df reg_mode   set reg_mode 0
df index_mode set index_mode 1
df deref_mode set deref_mode 2
df incr_mode  set incr_mode 3




df imm_mode set imm_mode incr_mode
df imm_reg set imm_reg pc

df literal_mode set literal_mode index_mode
df constant_1 set constant_1 cg

df fixed_reg set fixed_reg sr
df fixed_mode set fixed_mode index_mode

df nat8 set nat8 1
df nat16 set nat16 2


df stack_start set stack_start ram_start  add stack_start 256


df x  set x 05h 
si x 4 or x 0Ah 
si x 4 or x 08h 
si x 4 or x 00h 
df 5A80h set 5A80h x udf x


df x  set x 00h 
si x 4 or x 03h 
si x 4 or x 0Fh 
si x 4 or x 0Fh 
df 03FFh set 03FFh x udf x


df 0400h set 0400h 03FFh incr 0400h

df x  set x 0Ah 
si x 4 or x 05h 
si x 4 or x 00h 
si x 4 or x 00h 
df A500h set A500h x udf x


df ret
df skip_macros 
do skip_macros

df msp_nop cat msp_nop
	gen4 msp_mov  reg_mode 15 0  reg_mode 15 0 size_word
	incr ret do ret

cat skip_macros


. ------------------- start of the actual code ----------------- . 

section fram_start
	gen4 msp_mov  reg_mode sp 0   imm_mode imm_reg  stack_start  size_word
	gen4 msp_mov  fixed_mode fixed_reg wdt_wdtctl   imm_mode imm_reg  5A80h  size_word
	gen4 msp_bis  fixed_mode fixed_reg port2_p2dir  imm_mode imm_reg  bit6     size_byte
	gen4 msp_bis  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte
	gen4 msp_bic  fixed_mode fixed_reg pmm_pm5ctl0  literal_mode constant_1 0  size_word
	gen4 msp_mov  fixed_mode fixed_reg pmm_pmmctl0  imm_mode imm_reg  A500h  size_word

df main at main	

	gen4 msp_bis  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte

	df delay_time set delay_time FFFFh
	df iterator set iterator 4
	gen4 msp_mov  reg_mode iterator 0  imm_mode imm_reg delay_time size_word
	df loop at loop gen4 msp_sub  reg_mode iterator 0  literal_mode constant_1 0 size_word	
	br4 condjnz loop 
	udf loop udf iterator udf delay_time


	gen4 msp_bic  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte


	df delay_time2 set delay_time2 7
	df iterator2 set iterator2 5
	gen4 msp_mov  reg_mode iterator2 0  imm_mode imm_reg delay_time2 size_word
	df loop2 at loop2 

	df delay_time set delay_time FFFFh
	df iterator set iterator 4
	gen4 msp_mov  reg_mode iterator 0  imm_mode imm_reg delay_time size_word
	df loop at loop gen4 msp_sub  reg_mode iterator 0  literal_mode constant_1 0 size_word	
	br4 condjnz loop 
	udf loop udf iterator udf delay_time

	gen4 msp_sub  reg_mode iterator2 0  literal_mode constant_1 0 size_word	
	br4 condjnz loop2 
	udf loop2 udf iterator2 udf delay_time2

	br4 condjmp main
	cat ret do msp_nop




section reset_vector
	emit nat16 fram_start

eoi

-------------------------------------------------------------







































.	gen4 msp_bis  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte

	df i set i 0
	df nops cat nops
	cat ret do msp_nop
	incr i lt i 256 nops 
	udf nops udf i 

	gen4 msp_bic  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte

	df i set i 0
	df nops cat nops
	cat ret do msp_nop
	incr i lt i 256 nops 
	udf nops udf i 

	gen4 msp_bis  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte


	df delay_time set delay_time 0Fh
	df iterator set iterator 4
	gen4 msp_mov  reg_mode iterator 0  imm_mode imm_reg delay_time size_word
df loop at loop
	gen4 msp_sub  reg_mode iterator 0  literal_mode constant_1 0 size_word	
	br4 condjnz loop 
	udf loop udf iterator udf delay_time


	gen4 msp_bic  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte
.





.	df delay_time set delay_time 0Fh

	gen4 msp_bic  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte

	df iterator set iterator 4
	gen4 msp_mov  reg_mode iterator 0  imm_mode imm_reg delay_time size_word

df loop at loop
	gen4 msp_sub  reg_mode iterator 0  literal_mode constant_1 0 size_word	
	br4 condjnz loop 
udf loop





	df delay_time set delay_time 0FFFh

	gen4 msp_bis  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  bit6     size_byte

	df iterator set iterator 4
	gen4 msp_mov  reg_mode iterator 0  imm_mode imm_reg delay_time size_word

df loop at loop
	gen4 msp_sub  reg_mode iterator 0  literal_mode constant_1 0 size_word	
	br4 condjnz loop
udf loop


.

















. 

testing out all addressing mode combinations:

gen4 msp_mov   reg_mode 0 0   reg_mode 0 0   size_word
gen4 msp_mov   reg_mode 0 0   index_mode 0 0   size_word
gen4 msp_mov   reg_mode 0 0   deref_mode 0 0   size_word
gen4 msp_mov   reg_mode 0 0   incr_mode 0 0   size_word

gen4 msp_mov   index_mode 0 0   reg_mode 0 0   size_word
gen4 msp_mov   index_mode 0 0   index_mode 0 0   size_word
gen4 msp_mov   index_mode 0 0   deref_mode 0 0   size_word
gen4 msp_mov   index_mode 0 0   incr_mode 0 0   size_word

.



. 

define all possible gen4 op codes here! 

df msp_mov set msp_mov 4
 ....etc...
 

df is_not_equal4 set is_not_equal4 0
df is_equal4 set is_equal4 1
df is_equal4 set is_equal4 2
df is_not_equal4 set is_not_equal4 3
df is_not_equal4 set is_not_equal4 4  

 ...other branch conditions too... 


	br4 cond label

	0 : JNE / JNZ
	1 : JEQ / JZ
	2 : JNC
	3 : JC
	4 : JN
	5 : JGE
	6 : JL
	7 : JMP


 
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




	0 : pc
	1 : sp
	2 : sr
	3 : cg2
	4 : R4
	5 : R5
	6 : R6
	7 : R7
	8 : R8
	9 : R9
	10 : R10
	11 : R11
	12 : R12
	13 : R13
	14 : R14
	15 : R15


. 








.	df loop at loop
	df max set max 16
	df i set i 0 df l cat l
		df op set op 7
		gen4 op   0 i 0   0 i 0   1		
		incr i lt i max l
	br4 7 loop
.




. 

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


	
df my_address set my_address 1 si my_address 14
section my_address

	df loop at loop
	df max set max 16
	df i set i 0 df l cat l
		df op set op 7
		gen4 op   0 i 0   0 i 0   1		
		incr i lt i max l		
	br4 7 loop


.






