"
	this is a program to test the ct/rt language, and generating arm64 machine code. 
	everything is working really well so far! we got the first working rt program yayyyy
	on 202403251.044244.  
	dwrr

"

"eof" "0" "1" "=" "debug arguments" "setarchitecture"  "setoutputformat" 
"preserveexecutable" "preserveobject"
"set object name" "set executable name"
"setcompiletime" "setruntime" "add" "sub" "addi" "ctdebug" 
"set debug" "ecall"

0 = "no runtime"
1 = "riscv 32"
01 = "riscv 64"
11 = "arm 32"
001 = "arm 64"

0 = "print binary"
1 = "elf object"
01 = "elf executable"
11 = "macho object"
001 = "macho executable"

0101 = "a0"
1101 = "a1"
0011 = "a2"
1011 = "a3"
0111 = "a4"
1111 = "a5"
00001 = "a6"
10001 = "a7" "system call number"

1 = "system exit"     a0 "system exit code"
01 = "system fork"
11 = "system read"    a0 "system read fd"     a1 "system read buffer"      a2 "system read length"
001 = "system write"
101 = "system open"
011 = "system close"

0 = "standard in"
1 = "standard out"


1 = "ra"
01 = "sp"



1 = set debug

0 = "zr"
001 = "4"
101 = "5"
011 = "sum"

set compiletime
4 zr sum addi 
5 sum sum addi
ct debug

arm 64 			set architecture
macho executable 	set output format
"object.o"		set object name
"program.out"		set executable name
0 = preserve object
0 = preserve executable


set runtime 



"         
	todo:  we should also call   the write()  system call next!  
"



standard in= 	zr 	system read fd 	addi
sp= 		zr 	system read buffer 	addi
1= 		zr 	system read length 	addi
system read 	zr 	system call number 	addi 
ecall

11111= 		zr 	system exit code 	addi
system exit 	zr 	system call number 	addi 
ecall


eof




















"enable debug output"
"print arguments"
"print registers"
"print instructions"
"print dictionary"
"set target arch"
"set output format"
"set object name"
"set executable name"
"preserve existing object"
"preserve existing executable"
"delete last argument"
"push new argument"





"enable debug output"
"print arguments"
"print registers"
"print instructions"
"print dictionary"



print dictionary 

end of file




	ins_eof, 
	ins_0, 
	ins_1, 
	ins_l, 

	ins_d, 
	ins_da, 
	ins_dr, 
	ins_di, 
	ins_dd,

	ins_ar, 
	ins_of, 
	ins_on, 
	ins_en, 
	ins_po, 
	ins_pe,

	ins_del, 
	ins_arg, 

	ctabort, 
	ctprint, 
	ctmode, 
	ctat, 
	ctget, 
	ctput,


	ctclear, 
	ctls, 
	ctli, 
	ctstop,

	ctpc, 
	ctb, 
	ctf, 
	ctblt,
	ctbge, 
	ctbeq, 
	ctbne,
 
	ctincr, 
	ctzero,

	ctadd, 
	ctsub, 
	ctmul, 
	ctdiv, 
	ctrem, 

	ctnor, 
	ctxor, 
	ctand, 
	ctor,
	ctsl, 
	ctsr,
 
	ctlb, 
	ctlh, 
	ctlw,
	ctld, 

	ctsb, 
	ctsh, 
	ctsw, 
	ctsd,

	db, 
	dh, 
	dw, 

	ecall, 
	ebreak, 
	fence, 
	fencei, 
	
	add, 
	sub, 
	sll, 
	slt, 
	sltu, 
	xor_, 
	srl, 
	sra, 
	or_, 
	and_, 
	addw, 
	subw, 
	sllw, 
	srlw, 
	sraw,
	lb, 
	lh, 
	lw, 
	ld, 
	lbu, 
	lhu, 
	lwu, 
	addi, 
	slti, 
	sltiu, 
	xori, 
	ori, 
	andi, 
	slli, 
	srli, 
	srai, 
	addiw, 
	slliw, 
	srliw, 
	sraiw,
	jalr, 
	csrrw, 
	csrrs, 
	csrrc, 
	csrrwi, 
	csrrsi, 
	csrrci, 
	sb, 
	sh, 
	sw, 
	sd, 
	lui, 
	auipc, 
	beq, 
	bne, 
	blt, 
	bge, 
	bltu, 
	bgeu, 
	jal, 
	mul, 
	mulh, 
	mulhsu, 
	mulhu,
	div_, 
	divu, 
	rem, 
	remu, 
	mulw, 
	divw, 
	divuw, 
	remw, 
	remuw, 

