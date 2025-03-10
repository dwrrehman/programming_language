lf foundation.s
set _targetarchitecture no_arch

. testing out the emergent macro system using just the ct system!
  using cat, incr, do, and lr  to make macros at compiletime!!!
	1202503097.161616 dwrr
.
df lr
df skip do skip
df function cat function df arg0
	df n set n arg0
	df i zero i
	df loop cat loop
		ctprint i
		incr i
		lt i n loop
	udf n udf i udf loop
	ctpause
	incr lr do lr
cat skip

set arg0 7
cat lr do function

set arg0 4
cat lr do function

set arg0 3
cat lr do function




