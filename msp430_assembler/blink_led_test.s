comment 
	1202409054.190054 an assembly file for the msp430 
	meant to help me program the msp430fr5994 that i have.
	eventually, this will control and get input from a 
	large led matrix that im going to construct lol.
comment

comment  assembler will produce a file named "output_machine_code.txt" always.  comment
setoutputname

comment ----------- main FRAM memory program: (address 0x4000) ------------- comment
section 0000_0000_0000_0010

	mov sp	pc incr 0000_0000_1101_1100 
	comment initialize the stack pointer to be 256 bytes before the end of RAM. comment

	mov	sr index 0011_1010_1000_0000     comment sr index means absolute addressing comment
		comment Watchdog timer control register comment

		pc incr 0000000_1_01011010    comment pc incr means immediate comment
		comment stop watchdog timer, with wdt_password 5A. comment


	bis byte	sr index 1010_0000_0100_0000
			comment p2 direction register comment

			pc incr 0001_0000
			comment p2.6 pin, set output comment


	bis byte	sr index 1100_0000_0100_0000
			comment p2 output register comment
		
			pc incr 0001_0000
			comment p2.6 pin, outputting high initially comment


	bic	sr index 0000_1100_1000_0000
		comment pm-control register, gpio lock comment
			
		pc incr 1
		comment disable gpio lpm5 lock bit comment


comment main loop: comment

at pc	


	bis byte	sr index 1100_0000_0100_0000
			comment p2 output port comment

			pc incr 0001_0000
			comment toggle p2.6 pin state output comment






	mov	r5	pc incr 0000_1000_0000_0000
at sp
	sub	r5	cg index		comment cg index is the constant 1, this does a decrement on r5 comment

	branch nonzero  sp 			comment  loop the decr until zero, a delay loop. comment




	bic byte	sr index 1100_0000_0100_0000
			comment p2 output port comment

			pc incr 0001_0000
			comment toggle p2.6 pin state output comment




	mov	r4	pc incr 101
at sr

	mov	r5	pc incr 0000_0000_0001_0000
at sp
	sub	r5	cg index		comment cg index is the constant 1, this does a decrement on r5 comment
	branch nonzero  sp 			comment  loop the decr until zero, a delay loop. comment

	sub r4  cg index
	branch nonzero sr





	branch always   pc			comment  loop the blinking until power turns us off lol. comment 

	mov   r4   r4    			comment  nop after last branch in section, hardware erratta  comment
	


comment ----------- interrupt vectors (address 0xFFFE) ------------- comment
section 0111_1111_1111_1111

	literalword 0000_0000_0000_0010     comment  start of main FRAM section  comment
eof









































					comment 0010_1111_11    branch offset -12,  =   0011_0000_00    1100_1111_11  
						goes to xor instruction.  comment









1202409054.204636:

output from dissassembler of this code:


printing 2 sections: 
section #0: .address = 0x4000, .length = 46 :: 
	[31] [40] [00] [3b] [b2] [40] [80] [5a] [5c] [01] [f2] [c8] [40] [00] [05] [02] 
	[f2] [d8] [40] [00] [03] [02] [b2] [c0] [01] [00] [30] [01] [f2] [e8] [40] [00] 
	[03] [02] [35] [40] [00] [80] [15] [73] [ff] [23] [f6] [3f] [04] [44] [end of section]
section #1: .address = 0xfffe, .length = 2 :: 
	[00] [40] [end of section]
[done]
heres the text too...
@4000
31 40 00 3B B2 40 80 5A 5C 01 F2 C8 40 00 05 02 
F2 D8 40 00 03 02 B2 C0 01 00 30 01 F2 E8 40 00 
03 02 35 40 00 80 15 73 FF 23 F6 3F 04 44 
@fffe
00 40 
q

dissassembly of section #0: [address=0x00004000], [length=46]
00004000:   4031 3b00      	MOV.W	SP <-- #0x3b00

00004004:   40b2 5a80 015c 	MOV.W	(SR + 0x015c) <-- #0x5a80

0000400a:   c8f2      0040 	BIC.B	(SR + 0x0040) <-- *(R8), R8++

0000400e:   0205 				extended ins: MOVA, CMPA, ADDA, SUBA, RRCM, RRAM, RLAM, RRUM

00004010:   d8f2      0040 	BIS.B	(SR + 0x0040) <-- *(R8), R8++

00004014:   0203 				extended ins: MOVA, CMPA, ADDA, SUBA, RRCM, RRAM, RLAM, RRUM

00004016:   c0b2 0001 0130 	BIC.W	(SR + 0x0130) <-- #0x1

0000401c:   e8f2      0040 	XOR.B	(SR + 0x0040) <-- *(R8), R8++

00004020:   0203 				extended ins: MOVA, CMPA, ADDA, SUBA, RRCM, RRAM, RLAM, RRUM

00004022:   4035 8000      	MOV.W	R5 <-- #0x8000

00004026:   7315      	SUB.W	R5 <-- (CG2 + 0x0000)

00004028:   23ff           	JNE/JNZ	[s=1] [offset=1ff]

0000402a:   3ff6           	JMP	[s=1] [offset=1f6]

0000402c:   4404           	MOV.W	R4 <-- R4

dissassembly of section #1: [address=0x0000fffe], [length=2]
0000fffe:   4000           	MOV.W	PC <-- PC







for reference, the language's full isa:


static const char* spelling[isa_count] = {
        "eof",
        "section", "literalbyte", "literalword", 
        "mov", "add", "addc", "sub", "subc", "cmp", 
	"dadd", "bit", "bic", "bis", "xor", "and", "branch",
        "pc", "sp", "sr", "cg", "r4", "r5", "r6", "r7", 
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", 
        "word", "byte", "address", 
        "nonzero", "zero", "nocarry", "carry", "negative", 
	"greaterequal", "less", "always",
        "direct", "index", "deref", "incr",
};




	xor r4 r4
	add r4 index 111   pc incr 101
	branch nonzero 1111_111_111







