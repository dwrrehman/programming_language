(
	a set of compiletime constants used with 
	the languages target machine instructions.

	written on 1202505224.161251 by dwrr
)

(arm64 machine instruction constants)

constant width64 	set width64 1
constant width32 	set width32 0

constant type_zero set type_zero 01
constant type_neg  set type_neg  0
constant type_keep set type_keep 11

constant shift_none set shift_none 0
constant shift_16   set shift_16   1
constant shift_32   set shift_32   01
constant shift_48   set shift_48   11

. register indexes .
constant linkregister set linkregister 01111
constant stackpointer set stackpointer 11111
constant zeroregister set zeroregister 11111

. branch conditions .
constant is_equal set is_equal 0
constant is_not_equal set is_not_equal 1
constant is_unsigned_higher_or_same set is_unsigned_higher_or_same 01
constant is_unsigned_greater_or_equal set is_unsigned_greater_or_equal 01
constant is_unsigned_lower set is_unsigned_lower 11
constant is_unsigned_less set is_unsigned_less 11
constant is_minus set is_minus 001
constant is_negative set is_negative 001
constant is_nonnegative set is_nonnegative 101
constant has_overflow_set set has_overflow_set 011
constant has_overflow_clear set has_overflow_clear 111
constant is_unsigned_greater set is_unsigned_greater 0001
constant is_unsigned_lower_or_same set is_unsigned_lower_or_same 1001
constant is_signed_greater_or_equal set is_signed_greater_or_equal 0101
constant is_signed_less set is_signed_less 1101
constant is_signed_greater set is_signed_greater 0011
constant is_signed_less_or_equal set is_signed_less_or_equal 1011
constant cond_always set cond_always 0111
constant cond_never set cond_never 1111

constant with_register set with_register 0
constant with_immediate set with_immediate 1

constant is_zero set is_zero 0           . valid with cond arguments .
constant is_nonzero set is_nonzero 1     . valid with cond arguments .

constant type_byteoffset set type_byteoffset 0
constant type_pageoffset set type_pageoffset 1

constant isreturn set isreturn 1
constant isnotreturn set isnotreturn 0
constant nolink set nolink 0
constant jumplink set jumplink 1

constant shift_increase set shift_increase 0
constant shift_decrease set shift_decrease 1
constant shift_signed_decrease set shift_signed_decrease 01
constant rotate_decrease set rotate_decrease 11

constant positive set positive 0
constant subtract set subtract 1

constant noflags  set noflags  0
constant setflags set setflags 1

constant shift_12 set shift_12 1

constant cmp_negative set cmp_negative 0
constant cmp_positive set cmp_positive 1

constant extend_uxtb set extend_uxtb 0
constant extend_uxth set extend_uxth 1
constant extend_uxtw set extend_uxtw 01
constant extend_uxtx set extend_uxtx 11
constant addx_sp_lsl set addx_sp_lsl 11
constant extend_sxtb set extend_sxtb 001
constant extend_sxth set extend_sxth 101
constant extend_sxtw set extend_sxtw 011
constant extend_sxtx set extend_sxtx 111

constant csel_incr set csel_incr 1
constant csel_not set csel_not 1
constant csel_set set csel_set 0

constant divr_unsigned set divr_unsigned 0
constant divr_signed set divr_signed 1

constant maddl_unsigned set maddl_unsigned 1
constant maddl_signed set maddl_signed 0
constant type_maddl set type_maddl 1
constant type_madd set type_madd 0

constant bfm_signed_zero set bfm_signed_zero 0
constant bfm_keep set bfm_keep 1
constant bfm_unsigned_zero set bfm_unsigned_zero 01

constant regular_second set regular_second 0
constant invert_second set invert_second 1

constant bitwise_and set bitwise_and 0
constant bitwise_or set bitwise_or 1
constant bitwise_eor set bitwise_eor 01
constant bitwise_and_setflags set bitwise_and_setflags 11

constant store set store 0
constant load_width32 set load_width32 001
constant load_width64 set load_width64 101
constant load_signed_width32 set load_signed_width32 011
constant load_signed_width64 set load_signed_width64 111

constant 1_byte  set 1_byte  0
constant 2_bytes set 2_bytes 1
constant 4_bytes set 4_bytes 01
constant 8_bytes set 8_bytes 11

constant zero_extend_second32 set zero_extend_second32 0
constant use_second64 set use_second64 1
constant sign_extend_second32 set sign_extend_second32 01
constant sign_extend_second64 set sign_extend_second64 01      (todo: is this one right?...)
constant scale_then_zero_extend_second32 set scale_then_zero_extend_second32 001
constant scale_second64 set scale_second64 101
constant scale_then_sign_extend_second32 set scale_then_sign_extend_second32 011
constant scale_then_sign_extend_second64 set scale_then_sign_extend_second64 111

constant store_pair set store_pair 0
constant load_pair set load_pair 1
constant pair_width32 set pair_width32 0
constant pair_width64 set pair_width64 01
constant nontemporal_pair set nontemporal_pair 0
constant post_advance_pair set post_advance_pair 1
constant offset_pair set offset_pair 01
constant pre_advance_pair set pre_advance_pair 11

constant pre_advance  set pre_advance  1
constant post_advance set post_advance 0





















(msp430 machine instruction constants)


constant pc set pc 0
constant sp set sp 1
constant sr set sr 01
constant cg set cg 11
constant r4 set r4 001
constant r5 set r5 101
constant r6 set r6 011
constant r7 set r7 111
constant r8 set r8 0001
constant r9 set r9 1001
constant r10 set r10 0101
constant r11 set r11 1101
constant r12 set r12 0011
constant r13 set r13 1011
constant r14 set r14 0111
constant r15 set r15 1111

constant msp_mov set msp_mov 001
constant msp_add set msp_add 101
constant msp_addc set msp_addc 011
constant msp_sub set msp_sub 111
constant msp_subc set msp_subc 0001
constant msp_cmp set msp_cmp 1001
constant msp_dadd set msp_dadd 0101
constant msp_bit set msp_bit 1101
constant msp_bic set msp_bic 0011
constant msp_bis set msp_bis 1011
constant msp_xor set msp_xor 0111
constant msp_and set msp_and 1111

constant condjnz set condjnz 0
constant condjz  set condjz  1
constant condjnc set condjnc 01
constant condjc  set condjc  11
constant condjn  set condjn  001
constant condjge set condjge 101
constant condjl  set condjl  011
constant condjmp set condjmp 111

constant size_byte set size_byte 1
constant size_word set size_word 0

constant bit0 set bit0 10000000
constant bit1 set bit1 01000000
constant bit2 set bit2 00100000
constant bit3 set bit3 00010000
constant bit4 set bit4 00001000
constant bit5 set bit5 00000100
constant bit6 set bit6 00000010
constant bit7 set bit7 00000001

constant reg_mode   set reg_mode 0
constant index_mode set index_mode 1
constant deref_mode set deref_mode 01
constant incr_mode  set incr_mode 11

constant imm_mode set imm_mode incr_mode
constant imm_reg set imm_reg pc

constant literal_mode set literal_mode index_mode
constant constant_1 set constant_1 cg

constant fixed_reg set fixed_reg sr
constant fixed_mode set fixed_mode index_mode

constant nat8 set nat8 1
constant nat16 set nat16 01



































(todo: move these to somewhere else!)


( system call ABI on macos)

df syscallnumber bn syscallnumber 00001
df syscallarg0 	 bn syscallarg0 0
df syscallarg1   bn syscallarg1 1
df syscallarg2   bn syscallarg2 01
df syscallarg3   bn syscallarg3 11
df syscallarg4   bn syscallarg4 001




( system call numbers on macos )

df system_exit 	bn system_exit  1
df system_fork 	bn system_fork  01
df system_read 	bn system_read  11
df system_write bn system_write 001
df system_open 	bn system_open  101
df system_close bn system_close 101





















