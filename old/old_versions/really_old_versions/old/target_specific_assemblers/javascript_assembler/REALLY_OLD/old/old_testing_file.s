

0 ctzero 
1 ctzero ctincr 
4 ctzero ctincr ctincr ctincr ctincr

10  ctzero 

	ctincr ctincr ctincr 
	ctincr ctincr ctincr 
	ctincr ctincr ctincr 
	ctincr





4 1 5 ctshl
4 5 5 ctshl

4 5 5 ctsub
4 5 5 ctsub
4 5 5 ctsub



0 0 0 3 cselx
0 0 0 3 cselw

0 0 0 3 csincx
0 0 0 3 csincw

0 0 0 3 csinvx
0 0 0 3 csinvw

0 0 0 3 csnegx
0 0 0 3 csnegw


0 0 0 0 adr

0 0 0 0 adrp






10 cted 
10 cted
25 ctdc
10 cted
10 cted
10 cted

25 ctprint

t cfinv t

20 2 bc
5 0 17 movzx
0 0 31 17 0 addx
21 b

25 emitd

20 ctat
4 0 0 movzx
21 ctat
1 0 16 movzw
svc


