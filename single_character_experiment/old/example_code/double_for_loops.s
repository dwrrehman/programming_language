1	zero 		define

101	i		define
011	j		define

111	a		define
0001	b		define
1001 	c		define
0101	d		define

000001 	loop 		define
100001	loop2		define

000011	ascii('0')	define
1000001 ascii('A')	define
0100001 ascii('B')	define
1100001 ascii('C')	define
1001	ascii('\t')	define
0101	ascii('\n')	define
0001111 tab		define
1001111 newline 	define

zero ctzero
ascii('0') d ctli zero d ctnor d ctincr

ascii('\t') tab ctli
ascii('\n') newline ctli

ascii('A') a ctli ctput
ascii('B') a ctli ctput
ascii('C') a ctli ctput
ascii('\n') a ctli ctput

a ctget d a ctadd
zero ctget ctzero

i ctzero 
loop ctpc
	i ctprint 

	j ctzero
	loop2 ctpc

		tab ctput j ctprint

		j ctincr a j ctblt loop2 ctb

	i ctincr a i ctblt loop ctb



ascii('C') a ctli ctprint 

eof



b ctget d b ctadd zero ctget ctzero

a b c ctadd ctprint




eof







011 0 0101 addi
1011101 0 10001 addi
ecall






	"ctdel", "ctls", "ctarg", "ctli", "ctstop",
	"ctat", "ctpc", "ctb", "ctf", "ctblt",
	"ctbge", "ctbeq", "ctbne", "ctincr", "ctzero",
	"ctadd", "ctmul", "ctdiv", "ctnor",
	"ctsl", "ctsr", "ctlb", "ctlh", "ctlw",
	"ctld", "ctsb", "ctsh", "ctsw", "ctsd",
	"ctprint", "ctabort",





