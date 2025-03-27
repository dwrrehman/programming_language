.
	msp430fr2433 TI microcontroller 
	datasheet information and constants 
	rewritten on 1202503204.223508 by dwrr
. 

df ram_start 	bn ram_start 	0000_0000_0000_0100
df ram_end 	bn ram_end 	1111_1111_1111_0100
df fram_start 	bn fram_start	0000_0000_0010_0011
df fram_end	bn fram_end	1111_1111_1111_1111 
df ifram_start	bn ifram_start	0000_0000_0001_1000
df ifram_end	bn ifram_end 	1111_1111_1001_1000 

df reset_vector bn reset_vector	0111_1111_1111_1111

df sfr_base 	bn sfr_base 	0000_0000_1000_0000
df pmm_base 	bn pmm_base	0000_0100_1000_0000
df sys_base 	bn sys_base	0000_0010_1000_0000
df cs_base 	bn cs_base	0000_0001_1000_0000
df fram_base 	bn fram_base	0000_0101_1000_0000
df wdt_base	bn wdt_base	0011_0011_1000_0000
df port12_base 	bn port12_base 	0000_0000_0100_0000
df port3_base 	bn port3_base 	0000_0100_0100_0000

df rtc_base 	bn rtc_base 	0000_0000_1100_0000
df timer0_base 	bn timer0_base 	0000_0001_1100_0000
df backup_base 	bn backup_base 	0000_0110_0110_0000
df mpy32_base 	bn mpy32_base 	0000_0011_0010_0000
df adc_base 	bn adc_base 	0000_0000_1110_0000

. hardware registers in peripherals . 

df pmm_pmmctl0 	bn pmm_pmmctl0 0000_0000 	add pmm_pmmctl0 pmm_base
df pmm_pmmctl1 	bn pmm_pmmctl1 0100_0000	add pmm_pmmctl1 pmm_base
df pmm_pmmctl2	bn pmm_pmmctl2 0010_0000	add pmm_pmmctl2 pmm_base
df pmm_pmmifg	bn pmm_pmmifg  0101_0000	add pmm_pmmifg pmm_base
df pmm_pm5ctl0	bn pmm_pm5ctl0 0000_1000	add pmm_pm5ctl0 pmm_base

df cs_csctl0	bn cs_csctl0 0000_0000	add cs_csctl0 cs_base
df cs_csctl1	bn cs_csctl1 0100_0000	add cs_csctl1 cs_base
df cs_csctl2	bn cs_csctl2 0010_0000	add cs_csctl2 cs_base
df cs_csctl3	bn cs_csctl3 0110_0000	add cs_csctl3 cs_base
df cs_csctl4	bn cs_csctl4 0001_0000	add cs_csctl4 cs_base
df cs_csctl5	bn cs_csctl5 0101_0000	add cs_csctl5 cs_base
df cs_csctl6	bn cs_csctl6 0011_0000	add cs_csctl6 cs_base
df cs_csctl7	bn cs_csctl7 0111_0000	add cs_csctl7 cs_base
df cs_csctl8	bn cs_csctl8 0000_1000	add cs_csctl8 cs_base

df fram_frctl0	bn fram_frctl0 0000_0000	add fram_frctl0 fram_base
df fram_gcctl0	bn fram_gcctl0 0010_0000	add fram_gcctl0 fram_base
df fram_gcctl1	bn fram_gcctl1 0110_0000	add fram_gcctl1 fram_base

df wdt_wdtctl set wdt_wdtctl 0 add wdt_wdtctl wdt_base

df port1_p1in  set port1_p1in  0 add port1_p1in port12_base
df port1_p1out set port1_p1out 01 add port1_p1out port12_base
df port1_p1dir set port1_p1dir 001 add port1_p1dir port12_base
df port1_p1ren set port1_p1ren 011 add port1_p1ren port12_base

df port2_p2in  set port2_p2in  1 add port2_p2in port12_base
df port2_p2out set port2_p2out 11 add port2_p2out port12_base
df port2_p2dir set port2_p2dir 101 add port2_p2dir port12_base
df port2_p2ren set port2_p2ren 111 add port2_p2ren port12_base

df port3_p3in  set port3_p3in  0 add port2_p2ren port3_base
df port3_p3out set port3_p3out 01 add port3_p3out port3_base
df port3_p3dir set port3_p3dir 001 add port3_p3dir port3_base
df port3_p3ren set port3_p3ren 011 add port3_p3ren port3_base

df rtc_rtcctl set rtc_rtcctl  0 add rtc_rtcctl rtc_base
df rtc_rtciv  set rtc_rtciv   001 add rtc_rtciv rtc_base
df rtc_rtcmod set rtc_rtcmod  0001 add rtc_rtcmod rtc_base
df rtc_rtccnt set rtc_rtccnt  0011 add rtc_rtccnt rtc_base

. todo: add the rest of the relevant 
   microcontroller information here! 
. 




. useful macros for msp430: . 


df skip  do skip

df0 msp_nop cat msp_nop
	gen4 msp_mov  reg_mode r15 0 reg_mode r15 0 size_word
	incr lr do lr

df0 delay cat delay
	df delay_time2 	set delay_time2 1
	df delay_time 	bn delay_time 1111_1111_1111_1111 
	div delay_time 01

	df iterator2 df iterator set iterator2 r5
	gen4 msp_mov  reg_mode iterator2 0  imm_mode imm_reg delay_time2 size_word
	df loop2 at loop2 
		df iterator set iterator r4
		gen4 msp_mov  reg_mode iterator 0  imm_mode imm_reg delay_time size_word
		df loop at loop 
			gen4 msp_sub  reg_mode iterator 0  literal_mode constant_1 0 size_word	
			br4 condjnz loop 
		udf loop udf iterator udf delay_time
		gen4 msp_sub  reg_mode iterator2 0  literal_mode constant_1 0 size_word	
		br4 condjnz loop2 
	udf loop2 udf iterator2 udf delay_time2
	incr lr do lr

df1 set_p2_pin cat set_p2_pin
	df pin set pin arg0
	gen4 msp_bis  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  pin     size_byte
	udf pin
	incr lr do lr

df1 reset_p2_pin cat reset_p2_pin
	df pin set pin arg0
	gen4 msp_bic  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  pin     size_byte
	udf pin
	incr lr do lr

cat skip udf skip


eoi







