.code16
.global get_bios_e801_memsize

/**********************************************
 *
 * get_bios_e801_memsize - 
 *		Read the total memory size using BIOS interrupt 0x15, function 0xe801.
 *
 * @return %ax - number of KiB between 1MB and 16MB
 * @return %bx - number of 64K blocks above 16MB
 * @return %bx = 0 and %ax = -1 on error
 *
**********************************************/
 

 .type   get_bios_e801_memsize, @function
get_bios_e801_memsize:

	//! saved and cleared for testing later
	
	push    %ecx
	push    %edx
	xor     %ecx, 		%ecx
	xor     %edx, 		%edx

	movw    $0xe801, 	%ax  	/* %ax = 0xe801 to call the e801 function */
	int     $0x15

	//! error checking

	jc      .failure

	cmpb    $0x86,  	%ah		/* unsupported func */
	je	    .failure
	
	cmpb    $0x80,  	%ah		/* invalid command */
	je	    .failure

	jcxz    .use_ax

	mov     %cx, 		%ax		/* move the value in %cx to %ax */
	mov     %dx, 		%bx		/* move the value in %dx to %bx */

.use_ax:

	//! restore the earlier saved registers

	pop     %edx
	pop     %ecx
	ret

.failure:
	movw    $0xffff, 	%ax		/* set %ax to -1 */
	xor     %bx, 		%bx	    /* set %bx to 0 */
	pop     %edx
	pop     %ecx
	ret
