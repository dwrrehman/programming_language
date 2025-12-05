
section 010101010101010

subc word sp cg deref
mov sr index 0000_0100_1   pc incr 0000_0001_0101_1010
bis byte sr index 0100_01   cg incr
xor byte sr index 1000_01   cg incr
mov sp index 0   pc incr 0000_1000_1110_01





section 00010111010011

add r4 r5
add r4 r5
add r4 r5
add r4 r5
add r4 r5
branch always 1111_111_111
branch carry 1
branch zero 1


section 001011010

literalbyte 111101

literalbyte 111101

literalword 0001100101011

eof












add r4 r6
sub sp pc autoincr 11111
add r4 sr
mov r4 r6
mov sr sr
sub r4 r6




0000e000:   8321           	SUBC.W	SP <-- *(CG2)

0000e002:   40b2 5a80 0120 	MOV.W	(SR + 0x0120) <-- #0x5a80

0000e008:   d3f2      0022 	BIS.B	(SR + 0x0022) <-- *(CG2), CG2++

0000e00c:   e3f2      0021 	XOR.B	(SR + 0x0021) <-- *(CG2), CG2++

0000e010:   40b1 2710 0000 	MOV.W	(SP + 0x0000) <-- #0x2710
