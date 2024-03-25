"eof" "0" "1" "=" "debug arguments" "setarchitecture"  "setoutputformat" 
"set object name" "set executable name"
"setcompiletime" "setruntime" "add" "sub" "addi" "ctdebug" 
"set debug" "ecall"

1 = set debug

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
"my_cool_ob   yect.o"	set object name
"program.out"		set executable name


set runtime 

4 zr sum addi
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

