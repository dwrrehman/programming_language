#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main() {


	


}




/*




// 36 bytes
    
    .arch armv6
    .global _start
    .text

_start:
    .code 32
    ldr    r0, =#0x6e69622f // /bin
    ldr    r1, =#0x68732f2f // //sh

    // switch to thumb mode
    add    r2, pc, #1
    bx     r2
    
    .code 16
    // execve("/bin/sh", NULL, NULL);
    eor    r2, r2, r2     // r2 = NULL    
    push   {r0, r1, r2}   // save string + null bytes
    mov    r0, sp         // r0 = "/bin//sh", 0
    eor    r1, r1, r1     // r1 = NULL
    mov    r7, #11        // r7 = execve
    svc    1








i think we are just going to generate a regular executable, maybe,  

and then give the first 8 bytes or so,              be             add r2, pc, #1  \n     bx  r2             

							in order to switch to thumb mode! yay.

	

		we can do everything in thumb mode, actually. so that is going to be awesome lol. yay. 










	heres the list of all the armv7 thumb instructions that we are going to try to use:

armv7 thumb instruction set:






sxth	1 0 1 1  0 0 1 0  0 0   Rm3   Rd3
sxtb	1 0 1 1  0 0 1 0  0 1   Rm3   Rd3
uxth	1 0 1 1  0 0 1 0  1 0   Rm3   Rd3
uxtb	1 0 1 1  0 0 1 0  1 1   Rm3   Rd3

rev	1 0 1 1  1 0 1 0  0 0   Rm3   Rd3
rev16	1 0 1 1  1 0 1 0  0 1   Rm3   Rd3
revsh	1 0 1 1  1 0 1 0  1 1   Rm3   Rd3

and	0 1 0 0 0 0   0 0 0 0   Rm3   Rdn3
eor	0 1 0 0 0 0   0 0 0 1   Rm3   Rdn3
lslr	0 1 0 0 0 0   0 0 1 0   Rm3   Rdn3
lsrr	0 1 0 0 0 0   0 0 1 1   Rm3   Rdn3
asr	0 1 0 0 0 0   0 1 0 0   Rm3   Rdn3
adc	0 1 0 0 0 0   0 1 0 1   Rm3   Rdn3
sbc	0 1 0 0 0 0   0 1 1 0   Rm3   Rdn3
ror	0 1 0 0 0 0   0 1 1 1   Rm3   Rdn3

tst	0 1 0 0 0 0   1 0 0 0   Rm3   Rdn3
rsb	0 1 0 0 0 0   1 0 0 1   Rm3   Rd3
cmp	0 1 0 0 0 0   1 0 1 0   Rm3   Rn3
cmn	0 1 0 0 0 0   1 0 1 1   Rm3   Rn3
orr	0 1 0 0 0 0   1 1 0 0   Rm3   Rdn3
mul	0 1 0 0 0 0   1 1 0 1   Rm3   Rdn3
bic	0 1 0 0 0 0   1 1 1 0   Rm3   Rdn3
mvn	0 1 0 0 0 0   1 1 1 1   Rm3   Rdn3





mov 	0 0 0  0 0 0 0  0 0 0   Rm3   Rd3
add	0 0 0  1 1 0 0   Rm3    Rn3   Rd3
sub	0 0 0  1 1 0 1   Rm3    Rn3   Rd3
addi3	0 0 0  1 1 1 0   imm3   Rn3   Rd3
subi3	0 0 0  1 1 1 1   imm3   Rn3   Rd3
str	0 1 0 1  0 0 0   Rm3    Rn3   Rt3
strh	0 1 0 1  0 0 1   Rm3    Rn3   Rt3
strb 	0 1 0 1  0 1 0   Rm3    Rn3   Rt3
ldrsb 	0 1 0 1  0 1 1   Rm3    Rn3   Rt3
ldr	0 1 0 1  1 0 0   Rm3    Rn3   Rt3
ldrh	0 1 0 1  1 0 1   Rm3    Rn3   Rt3
ldrb 	0 1 0 1  1 1 0   Rm3    Rn3   Rt3
ldrsh	0 1 0 1  1 1 1   Rm3    Rn3   Rt3



lsli5	0 0 0 0 0    imm5    Rm3   Rd3
lsri5	0 0 0 0 1    imm5    Rm3   Rd3
asri5 	0 0 0 1 0    imm5    Rm3   Rd3
stri5 	0 1 1 0 0    imm5    Rn3   Rt3
stri5b	0 1 1 1 0    imm5    Rn3   Rt3
stri5h	1 0 0 0 0    imm5    Rn3   Rt3
ldri5 	0 1 1 0 1    imm5    Rn3   Rt3
ldri5b	0 1 1 1 1    imm5    Rn3   Rt3
ldri5h	1 0 0 0 1    imm5    Rn3   Rt3



mov	0 0 1 0 0   Rd3   imm8
addi8	0 0 1 1 0   Rdn3  imm8
subi8	0 0 1 1 1   Rdn3  imm8
cmpi8	0 0 1 0 1   Rn3   imm8
ldrl	0 1 0 0 1   Rt3   imm8
adr	1 0 1 0 0   Rd3   imm8
ldspi	1 0 0 1 1   Rt3   imm8
stspi	1 0 0 1 0   Rt3   imm8


add	0 1 0 0 0 1  0 0  Rdn4(3)   Rm4   Rdn4(2:0)
cmp	0 1 0 0 0 1  0 1  Rdn4(3)   Rm4   Rdn4(2:0)
mov	0 1 0 0 0 1  1 0  Rdn4(3)   Rm4   Rdn4(2:0)
?????	0 1 0 0 0 1  1 1  Rdn4(3)   Rm4   Rdn4(2:0)


bc	1 1 0 1   cond4    imm8

b	1 1 1 0 0   imm11

blx	0 1 0 0 0 1  1 1 1  Rm4 0 0 0 
bx	0 1 0 0 0 1  1 1 0  Rm4 0 0 0 


cbz	1 0 1 1   1  0   imm6(5)   1    imm6(4:0)   Rn3
cbnz	1 0 1 1   1  1   imm6(5)   1    imm6(4:0)   Rn3

push 	1 0 1 1  0  1 0   push_LR   push_register_list
pop	1 0 1 1  1  1 0   pop_PC    pop_register_list

subsp	1 0 1 1  0 0 0 0 1   imm7

svc	1 1 0 1  1 1 1 1  imm8

nop	1 0 1 1  1 1 1 1  0 0 0 0  0 0 0 0 
yield	1 0 1 1  1 1 1 1  0 0 0 1  0 0 0 0 
wfe	1 0 1 1  1 1 1 1  0 0 1 0  0 0 0 0 
wfi	1 0 1 1  1 1 1 1  0 0 1 1  0 0 0 0 


it 	1 0 1 1  1 1 1 1  firstcond4 mask4



















probably not neccessary:


it 	1 0 1 1  1 1 1 1  firstcond4 mask4














note:
	you can unpredicate instructions, if they are inside of an IT block, i think. 



*/