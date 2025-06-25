(testing out the memory system in the c backend!
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


			      (^------------ heres the bug:  

					we need to be doing CTE stage 2   on the machine instructions, (roughly) for tracking what system calls we would execute. 

					this is critical, as we need to know if the system calls output something. 

					that is the only way for the compiler to NOT do const prop wrongly then. 

				)



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


at skip del skip

set p 0 
page p
print p
exit 0011








(
 void *addr = mmap(0, allocation_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);









PROT_READ = 0x1
PROT_WRITE = 0x2
MAP_PRIVATE = 0x2
MAP_ANONYMOUS = 0x1000
MAP_FAILED = 0xffffffffffffffff

)






















