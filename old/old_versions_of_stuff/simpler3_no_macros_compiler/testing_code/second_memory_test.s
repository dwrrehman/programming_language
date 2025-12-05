(
second test of the memory system in the c backend. 
this tests out the label and la system, using emit instructions. ie, constant .text data.

testing out the memory system in the c backend!
using the load and store instructions. 
written on 1202506253.025920 by dwrr)

file library/foundation.s
ct
st compiler_target c_arch nat
st compiler_format c_source_output nat
st compiler_should_overwrite true nat

set c_system_debug 	0
set c_system_exit 	1
set c_system_read 	01
set c_system_write 	11
set c_system_open 	001
set c_system_close 	101
set c_system_mmap 	011
set c_system_munmap 	111

set -1 0 sub -1 1
set prot_read 1
set prot_write 01
set map_private 01
set map_anonymous 0000_0000_0000_1
set map_failed -1 

rt

register c_sc_call_number 0
register c_sc_arg0 1
register c_sc_arg1 01
register c_sc_arg2 11
register c_sc_arg3 001
register c_sc_arg4 101
register c_sc_arg5 011

ct do skip

operation print 1 at print ct
	ld ra compiler_ctsc_number nat
	ld data compiler_ctsc_arg0 nat rt 
	set c_sc_call_number c_system_debug
	dr data set c_sc_arg0 data 
	system
	ct do ra del ra del data

operation printi 1 at printi ct
	ld ra compiler_ctsc_number nat
	ld data compiler_ctsc_arg0 nat rt 
	set c_sc_call_number c_system_debug
	set c_sc_arg0 data 
	system
	ct do ra del ra del data

operation exit 1 at exit ct
	ld ra compiler_ctsc_number nat
	ld data compiler_ctsc_arg0 nat rt 
	set c_sc_call_number c_system_exit
	set c_sc_arg0 data 
	system halt
	ct do ra del ra del data


operation page 1 at page ct
	ld ra compiler_ctsc_number nat
	ld p compiler_ctsc_arg0 nat

	set page_size 0000_0000_0000_1 
	set permissions prot_read or permissions prot_write
	set type map_private or type map_anonymous
	rt
	set c_sc_call_number c_system_mmap

	set c_sc_arg0 0
	set c_sc_arg1 page_size
	set c_sc_arg2 permissions
	set c_sc_arg3 type
	set c_sc_arg4 -1
	set c_sc_arg5 0
	system

	dr p set p c_sc_arg0
	ct do ra 

	del ra del p
	del type del page_size 
	del permissions


operation write 11 at write ct
	ld ra compiler_ctsc_number nat
	ld fd compiler_ctsc_arg0 nat 
	ld address compiler_ctsc_arg1 nat
	ld length compiler_ctsc_arg2 nat rt 

	set c_sc_call_number c_system_write
	set c_sc_arg0 fd dr address
	set c_sc_arg1 address
	set c_sc_arg2 length
	system

	ct do ra 
	del ra 
	del fd 
	del address
	del length


at skip del skip
rt

la e hello print e
ld n e byte print n

ct st compiler_ctsc_number compiler_getlength nat 
st compiler_ctsc_arg0 0 nat
system
ld length compiler_ctsc_arg0 nat
rt

write 1 e length
exit 0

at hello
string "hello world!
"



























