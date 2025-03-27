. testing out how macros work in the language!
1202503193.221354 by dwrr .

df arch zero arch
df format zero format
df overwrite zero overwrite

df lr 

df  arg0 df  arg1 df  arg2 df  arg3
df  arg4 df  arg5 df  arg6 df  arg7
df  arg8 df  arg9 df arg10 df arg11
df arg12 df arg13 df arg14 df arg15

df one bn one 1 ro one
df five bn five 101 ro five

df skip do skip


df1 double cat double
	si arg0 one
	incr lr do lr


df2 printlrs cat printlrs

	df printcount 
	set printcount arg0 

	df number 
	set number arg1

	df i zero i 
	df loop cat loop
		ctdebug i incr i
		lt i printcount loop
	udf i udf loop 

	ctdebug number
	ctpause

	udf printcount udf number
	incr lr do lr

cat skip

df x set x five double x set x arg0
printlrs x one
df other bn other 1010_1000_0000_1
printlrs other five



eoi


