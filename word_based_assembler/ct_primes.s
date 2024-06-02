
10001 def callnumber
0101 def arg0

01 def sp
1 def ra
0 def zero

11 def ctecall_abort
01 def ctecall_print


sp 0 arg0 ct add
ctecall_print 0 callnumber ct addi 
ct ecall def hello

1 sp sp ct addi

sp 0 arg0 ct add
ctecall_print 0 callnumber ct addi 
ct ecall  def hello

1 sp sp ct addi

sp 0 arg0 ct add
ctecall_print 0 callnumber ct addi 
ct ecall 

1 sp sp ct addi

sp 0 arg0 ct add
ctecall_print 0 callnumber ct addi 
ct ecall 

1 sp sp ct addi

ctecall_abort 0 callnumber ct addi
ct ecall

everything from here is comments..
