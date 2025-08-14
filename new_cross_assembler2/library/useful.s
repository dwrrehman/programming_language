(some useful routines for the standard library)

set '0' 000011
set newline 0101

st ctstackpointer ctcallstack

zero c0 zero c1
zero c2 zero c3
zero c3 zero c5
zero c6 zero c7

eq 0 0 skip_all_routines


at function_begin
	ld ra 0
	set arg c0

	ld sp ctstackpointer
	st sp arg
	incr sp
	st ctstackpointer sp 

	del sp
	del arg

	eq 0 0 ra del ra
	lt 0 0 function_begin


at function_end
	ld ra 0

	ld sp ctstackpointer sub sp 1
	st ctstackpointer sp
	del sp

	eq 0 0 ra del ra
	lt 0 0 function_end


at riscv_uf2
	ld ra 0
	set c0 ra function_begin

	set target riscv_arch
	st output_format uf2_executable
	st overwrite_output true

	function_end
	eq 0 0 ra del ra lt 0 0 riscv_uf2


at riscv_hex
	ld ra 0
	set c0 ra function_begin

	set target riscv_arch
	st output_format hex_array
	st overwrite_output true

	function_end
	eq 0 0 ra del ra lt 0 0 riscv_hex


at arm64
	ld ra 0
	set c0 ra function_begin


	set target arm64_arch
	st output_format macho_executable
	st overwrite_output true

	st executable_stack_size min_stack_size_macos

	function_end
	eq 0 0 ra del ra lt 0 0 arm64



at ctabort
	ld ra 0
	set c0 ra function_begin
	eq 0 0 -1 lt 0 0 ctabort


at ctputchar
	ld ra 0
	set c c0
	set c0 ra function_begin
	ld p assembler_pass
	eq p 0 skip
		st assembler_putc c
	at skip del skip
	del p del c
	function_end
	eq 0 0 ra del ra
	lt 0 0 ctputchar

at ctprintbinary
	ld ra 0
	set n c0
	set c0 ra function_begin

	at loop
		set bit n
		and bit 1
		add bit '0'
		set c0 bit ctputchar 
		sd n 1
		lt 0 n loop  

	del loop del n del bit

	function_end
	eq 0 0 ra del ra
	lt 0 0 ctprintbinary



at ctnl 
	ld ra 0
	set c0 ra function_begin
	set c0 newline ctputchar
	function_end
	eq 0 0 ra del ra lt 0 0 ctnl


at la
	ld ra 0
	set destination c0
	set source_label c1
	set c0 ra function_begin

	eq target riscv_arch valid
		eq 0 0 -1 str "error: la only supports riscv currently" 
	at valid del valid
	
	at pc
	set offset source_label
	sub offset pc
	del pc

	ru r_auipc destination 0

(todo: do the bitwise operations to extract 
the top 20 bits of the immediate too!!)

	ri r_imm r_add destination destination offset 
	del offset

	del destination
	del source_label

	function_end
	eq 0 0 ra del ra
	lt 0 0 la


at li
	ld ra 0
	set destination c0
	set immediate c1	
	set c0 ra function_begin

	eq target riscv_arch valid
		eq 0 0 -1 str "error: la only supports riscv currently" 
	at valid del valid

	lt immediate 0000_0000_0000_0000_0000_0000_0000_0000_1 skip_abort
		eq 0 0 -1 str "error: argument to li did not fit in a 32-bit number."
	at skip_abort del skip_abort

	set bit immediate and bit 0000_0000_0001 sd bit 1101
	set a immediate sd a 0011 add a bit and a 1111_1111_1111_1111_1111 
	set b immediate and b 1111_1111_1111

	set source 0
	eq 0 a s
		ru r_lui destination a
		set source destination
	at s del s

	eq 0 b s
		ri r_imm r_add destination source b
	at s del s

	del source del a del b del bit
	del destination del immediate

	function_end

	eq 0 0 ra del ra
	lt 0 0 li

at exit
	ld ra 0
	set code c0
	set c0 ra function_begin

	eq target riscv_arch riscv_exit
	eq target arm64_arch arm64_exit
	eq 0 0 -1 str "error: exit system only supports riscv and arm64" 

	at arm64_exit 
	
		mov a6_number a6_exit 0 mov_type_zero 1
		mov a6_arg0 code 0 mov_type_zero 1
		svc

	eq 0 0 return
	at riscv_exit

		ri r_imm r_add r_number 0 r_exit
		ri r_imm r_add r_arg0 0 code
		ri r_ecall 0 0 0 0

	at return
	del riscv_exit del arm64_exit

	function_end
	del code
	eq 0 0 ra del ra 
	lt 0 0 exit



at writestring
	ld ra 0
	set string c0
	set length c1
	set c0 ra function_begin

	eq target riscv_arch riscv_writestring
	eq target arm64_arch arm64_writestring

	eq 0 0 -1 str "error: writestring only supports riscv and arm64"

	at arm64_writestring

		mov a6_number a6_write 0 mov_type_zero 1
		mov a6_arg0 stdout 0 mov_type_zero 1
		adr a6_arg1 string 0
		mov a6_arg2 string.length 0 mov_type_zero 1
		svc
	
	eq 0 0 return
	at riscv_writestring

		ri r_imm r_add r_number 0 r_write
		ri r_imm r_add r_arg0 0 stdout
		set c0 r_arg1 set c1 string la
		ri r_imm r_add r_arg2 0 length
		ri r_ecall 0 0 0 0
	
	at return

	del riscv_writestring 
	del arm64_writestring 
	del string del length


	function_end
	eq 0 0 ra del ra  lt 0 0 writestring



at readchar
	ld ra 0
	set out c0
	set c0 ra function_begin

	eq target riscv_arch valid
		eq 0 0 -1 str "error: readchar only supports riscv currrently"
	at valid del valid

	set copy r_arg3
	ri r_imm r_add r_number 0 r_read
	ri r_imm r_add r_arg0 0 stdin
	set c0 r_arg1 set c1 buffer la
	ri r_imm r_add copy r_arg1 0 
	ri r_imm r_add r_arg2 0 1
	ri r_ecall 0 0 0 0
	ri r_load r_lbu out copy 0

	function_end
	eq 0 0 ra del ra del out del copy
	lt 0 0 readchar


at writechar
	ld ra 0
	set in c0
	set c0 ra function_begin

	ri r_imm r_add r_number 0 r_write
	ri r_imm r_add r_arg0 0 stdout
	set c0 r_arg1 set c1 buffer la
	rs r_store r_sb r_arg1 in 0
	ri r_imm r_add r_arg2 0 1
	ri r_ecall 0 0 0 0

	function_end
	eq 0 0 ra del ra del in
	lt 0 0 writechar




at writebuffer 
	ld ra 0
	set pointer_register c0
	set length_register c1
	set c0 ra function_begin

	eq target riscv_arch valid
		eq 0 0 -1 str "error: writebuffer only supports riscv currrently"
	at valid del valid

	ri r_imm r_add r_number 0 r_write
	ri r_imm r_add r_arg0 0 stdout
	ri r_imm r_add r_arg1 pointer_register 0
	ri r_imm r_add r_arg2 length_register 0
	ri r_ecall 0 0 0 0

	function_end
	eq 0 0 ra del ra 
	del pointer_register del length_register
	lt 0 0 writebuffer

at getstringlength
	ld ra 0
	set arg c0
	set c0 ra function_begin

	set begin arg
	at here
	set arg here
	sub arg begin
	set c0 arg
	del here del begin
	

	function_end
	eq 0 0 ra del ra del arg
	lt 0 0 getstringlength




at printbinary   (note: "buffer" must be present in the executable.)
	ld ra 0
	set input c0
	set c0 ra function_begin

	eq target riscv_arch valid
		eq 0 0 -1 str "error: writebuffer only supports riscv currrently"
	at valid del valid


	set bit 	r_number
	set data 	r_arg0
	set begin 	r_arg1
	set p 		r_arg2
	
	ri r_imm r_add data input 0
	set c0 begin set c1 buffer la
	ri r_imm r_add p begin 0
	at loop
		ri r_imm r_and bit data 1
		ri r_imm r_add bit bit '0'
		rs r_store r_sb p bit 0
		ri r_imm r_add p p 1
		ri r_imm r_srl data data 1
		rb r_branch r_bne data 0 loop

	ri r_imm r_add bit 0 newline
	rs r_store r_sb p bit 0
	ri r_imm r_add p p 1

	ri r_imm r_add r_number 0 r_write
	ri r_imm r_add r_arg0 0 stdout
	rr r_reg r_add r_arg2 p begin r_signed
	ri r_ecall 0 0 0 0

	del loop 
	del p 
	del bit 
	del data 
	del begin
	del input

	function_end
	eq 0 0 ra del ra
	lt 0 0 printbinary



at readstring
	ld ra 0
	set c0 ra function_begin

	eq target riscv_arch valid
		eq 0 0 -1 str "error: writebuffer only supports riscv currrently"
	at valid del valid

	set string c0
	set length c1
	ri r_imm r_add r_number 0 r_read
	ri r_imm r_add r_arg0 0 stdin
	set c0 r_arg1 set c1 string la
	ri r_imm r_add r_arg2 0 length
	ri r_ecall 0 0 0 0

	function_end
	eq 0 0 ra del ra 
	del string del length
	lt 0 0 readstring


at stringsequal 
	ld ra 0
	set c0 ra function_begin

	eq 0 0 allones
	str 'unimplemented'

	function_end
	eq 0 0 ra del ra


at decrement
	ld ra 0
	set d c0
	set c0 ra function_begin
	ri r_imm r_add d d 1111_1111_1111
	del d
	function_end
	eq 0 0 ra del ra
	lt 0 0 decrement
	

at increment
	ld ra 0
	set d c0
	set c0 ra function_begin
	
	ri r_imm r_add d d 1
	del d

	function_end
	eq 0 0 ra del ra
	lt 0 0 increment
	


at skip_all_routines del skip_all_routines


eoi


a set of utility routines which are useful
for risc-v programs, and compiletime programs.
originally written on 1202507292.174513 by dwrr 

rewritten and made more portable on 1202508037.203119 


























(at not 
	ld ra 0
	set n c0
	set c0 ra function_begin

	set r -1 sub r n
	set c0 r del r

	function_end
	del n
	eq 0 0 ra del ra 
	lt 0 0 not)

(at rem
	ld ra 0
	set n c0 
	set m c1
	set c0 ra function_begin
	
	set d n div d m
	set r d mul r m set k n sub k r
	set c0 k
	del k del r del n del m del d

	function_end
	eq 0 0 ra del ra
	lt 0 0 rem)




(at si
	ld ra 0
	set value c0
	set shift_amount c1

	lt shift_amount 0000_001 valid
		eq 0 0 -1 str "error: invalid shift amount to si" 
	at valid del valid

	set i 0
	at loop
		eq i shift_amount done 
		mul value 01
		incr i 
		eq 0 0 loop 
		del loop del i del shift_amount
	at done del done

	set c0 value del value 
	eq 0 0 ra del ra
	lt 0 0 si

at sd
	ld ra 0
	set value c0	
	set shift_amount c1

	lt shift_amount 0000_001 valid
		eq 0 0 -1 str "error: invalid shift amount to si" 
	at valid del valid

	set i 0
	at loop
		eq i shift_amount done 
		div value 01
		incr i 
		eq 0 0 loop 
		del loop del i del shift_amount
	at done del done

	set c0 value del value 
	eq 0 0 ra del ra
	lt 0 0 sd

at and     (a = a & b  in 64 bits)
	ld ra 0 set a c0 set b c1
	set c0 1 set c1 111_111 si set msb c0
	zero i zero c
	at loop
		mul c 01
		lt a msb s 
		lt b msb s 
			incr c 
		at s del s
		mul a 01 mul b 01 
		incr i lt i 0000_001 loop del loop del i
	del a del b del msb
	set c0 c del c
	eq 0 0 ra del ra 
	lt 0 0 and



at or    (a = a | b    in 64 bits)
	ld ra 0 set a c0 set b c1
	set c0 1 set c1 111_111 si set msb c0
	zero i zero c
	at loop
		add c c
		lt a msb try_b
			incr c 
			eq 0 0 advance
		at try_b 
		lt b msb advance
			incr c
		at advance 
		add a a 
		add b b
		incr i lt i 0000_001 loop 

	del a del b del msb 
	del advance del try_b 
	del loop del i
	set c0 c del c
	eq 0 0 ra del ra 
	lt 0 0 or
)











