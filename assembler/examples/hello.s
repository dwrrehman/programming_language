
comment foundation include comment 


----- construct constants -----

	4 ctzero ctincr ctincr ctincr ctincr	

	1 ctzero
	0 1 2 ctadd 


	5 ctzero
	5 2 ctst4 
	4 2 2 ctadd


	5 ctzero ctincr
	5 2 ctst4 
	4 2 2 ctadd 


	5 ctzero ctincr ctincr ctincr ctincr
	5 2 ctst4 
	4 2 2 ctadd 





---- call the write system call ----

	1 0 0 movzx


	0 1 adr


	1 0 2 movzx
	

	2 0 16 movzw

	svc





---- call the exit system call ----

	0 0 0 movzx
	1 0 16 movzw
	svc







something




 /* syscall write(int fd, const void *buf, size_t count) */
    mov     x0, #1      /* fd := STDOUT_FILENO */
    ldr     x1, =msg    /* buf := msg */
    ldr     x2, =len    /* count := len */
    mov     w8, #64     /* write is syscall #64 */
    svc     #0          /* invoke syscall */



something



