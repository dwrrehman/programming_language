lf library/found

def 100 set 100 10 mul 100 10
def 1000 set 1000 100 mul 1000 10

def limit set limit 1000 mul limit 100
def primecounter zero primecounter
def i zero i incr i incr i

def loop at loop	
	def halfi set halfi i sd halfi 1
	def j set j 2
	def inner at inner
		def prime lt halfi j prime
		def r set r i rem r j def composite eq r 0 composite udf r
		incr j do inner
	udf j udf halfi udf inner
	at prime udf prime
	set arg i at lr do debug
	incr primecounter
	at composite udf composite
	incr i lt i limit loop
udf i udf limit udf loop

set arg 0 at lr do debug
set arg 0 at lr do debug
set arg primecounter at lr do debug




eoi

























	set arg i at lr do debug











def limit set limit 10
def i zero i 

def loop at loop
	set arg i at lr do debug

	def limit set limit 10
	def i zero i 

	def loop at loop
		set arg i at lr do debug
		incr i 
		lt i limit loop

	udf loop
	udf i 
	udf limit 

	incr i 
	lt i limit loop

udf loop
udf i 
udf limit 



eoi


def limit set limit 10
def i zero i 

def loop at loop
	set arg i at lr do debug
	incr i 
	lt i limit loop

udf loop
udf i 
udf limit 



