. 
	a program to blink an LED on the 
	msp430fr2433 msp430 microcontroller, 
	written on 1202503134.230344 by dwrr. 
	rewritten to use macros and binary 
	constants on 1202503204.214715 
. 

lf foundation.s 
lf msp430fr2433_hardware.s

set _targetarchitecture 	msp430_arch 
set _outputformat 		ti_txt_executable
set _shouldoverwrite 		true

. port 2 pins : controlling a parellel-output 
  serial-input shift register. 
. 
df srclk	set srclk	bit0
df rclk		set rclk	bit1
df srclr	set srclr	bit2
df oe		set oe		bit3
df ser		set ser		bit4

df stack_start set stack_start ram_start add stack_start 000000001

section fram_start
	gen4 msp_mov  reg_mode sp 0   imm_mode imm_reg  stack_start  size_word

	df x bn x 0000_0001_0101_1010 
	gen4 msp_mov  fixed_mode fixed_reg wdt_wdtctl   imm_mode imm_reg x size_word
	udf x

	gen4 msp_bis  fixed_mode fixed_reg port2_p2dir  imm_mode imm_reg  1111_111     size_byte
	gen4 msp_bis  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  1111_111     size_byte
	gen4 msp_bic  fixed_mode fixed_reg pmm_pm5ctl0  literal_mode constant_1 0  size_word

	df x bn x 0000_0000_1010_0101
	gen4 msp_mov  fixed_mode fixed_reg pmm_pmmctl0  imm_mode imm_reg  x  size_word
	udf x

	reset_p2_pin oe
	set_p2_pin   srclr
	reset_p2_pin ser
	reset_p2_pin rclk
	reset_p2_pin srclk

	reset_p2_pin srclr  delay
	set_p2_pin srclr

	df value set value 011
	df addend set addend 111
	gen4 msp_xor reg_mode value 0   reg_mode value 0  size_word
	gen4 msp_mov reg_mode addend 0  imm_mode imm_reg 1011  size_word

df main at main	
	
	reset_p2_pin ser
	gen4 msp_add reg_mode value 0 reg_mode addend 0 size_word

	gen4 msp_bit reg_mode value 0 imm_mode imm_reg bit2 size_word
	df l br4 condjz l
		set_p2_pin ser
	at l udf l

	set_p2_pin srclk
	set_p2_pin rclk    delay
	reset_p2_pin srclk
	reset_p2_pin rclk  delay

	br4 condjmp main
	msp_nop

section reset_vector
	emit nat16 fram_start
eoi

-------------------------------------------------------------



















































































































. 

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

.































const int ser = 0;
const int oe = 7;
const int rclk = 6;
const int srclk = 5;
const int srclr = 28;

int data = 0;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(ser, OUTPUT);
  pinMode(srclk, OUTPUT);
  pinMode(srclr, OUTPUT);
  pinMode(rclk, OUTPUT);
  pinMode(oe, OUTPUT);

  digitalWrite(oe, 0); // enable output
  digitalWrite(srclr, 1); // don't clear
  digitalWrite(ser, 0); 
  digitalWrite(rclk, 0); 
  digitalWrite(srclk, 0); 

  digitalWrite(srclr, 0); 
  delay(100);
  digitalWrite(srclr, 1); 
  data = 0;
}

void loop() {
  digitalWrite(ser, data);  data = rand() % 2;
  digitalWrite(srclk, 1);
  digitalWrite(rclk, 1);
  delay(100);
  digitalWrite(srclk, 0);
  digitalWrite(rclk, 0);
  delay(100);
}








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






