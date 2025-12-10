/**********************************************
*
* @file 	stage2.s
* @author  	Abdul Rafay (abdul.rafay@lums.edu.pk)
*
* @brief   	Stage 2 bootloader code for x86. Mainly enables paging to enable
*		   	higher half kernel: sets up the initial page directory and
*		   	page tables, loads the kernel into memory, and switches to
*		   	protected mode.
*
**********************************************/

.code16

.include 	"common.s"

KERNEL_BYTES 		= KERNEL_SECTORS * 512

.extern 	gdt_desc

.section 	.stage2,"ax",@progbits

.global 	stage2
stage2:

	//! first load the kernel at 0x10000
	call 	load_kernel

	//! pretty much don't need BIOS interrupts anymore so we switch

	xorw    %ax,               %ax
    movw    %ax,               %ds         /*  addr to gdt is %ds:gdt_desc */
    lgdt    (gdt_desc) 
    mov     %cr0,              %eax
    or      $0x1,              %eax        /* set the PE bit in CR0 to 1 */
    mov     %eax,              %cr0

    //!  jump to the protected mode code
    //!  implicitly updates the CS register to the code segment offset

    ljmp    $CODE_SEG_OFFSET,  $load_descriptors
	
	nop
	hlt

//! loads the kernel from the disk into memory at the address defined

load_kernel:

	movw    $_KERNEL_LOAD_SEG,  %ax        /* currently 0x1000 */
	movw    $KERNEL_SECTORS, 	%cx		   /* sector count */
	movl    $2, 				%ebx	   /* starting sector is 2 */

.read_sect:

	movw    %ax,                %es
	call    read_sector_ext

	// update segment and sector number
	
	addw    $0x20, 				%ax
	incw	%bx
	loop    .read_sect

	ret
	
	

//! Reads sector from disk into memory (uses LBA in extended mode). Works fine
//!   on qemu and bochs so no need to check for availability.
//! 	%ebx = starting sector number
//!     %dl = drive number (0x80 for hd0)
//! 	%es = memory segment to read into (segment:offset=0)

read_sector_ext:

	pusha

	/* this is an extended read  */

	xorw    %ax, 		%ax
	pushw   %ax                 /* LBA sectnum [48:63] */
	pushw   %ax                 /* LBA sectnum [32:47] */
	pushl   %ebx				/* LBA sectnum [0:31] */

	push    %es				   	/* addr segment  */
	push    %ax                	/* addr offset */
	push    $1                	/* number of sectors */
	push    $16				    /* size of packet */

	movb    $0x42, 		%ah     /* int 0x13 ext read */
	movw    %sp, 		%si     /* DS:SI ptr to packet */

	int     $0x13               /* BIOS interrupt to read from device */

	jc      .error

	popa						/* cleanup stack (16 bytes removed) */
	popa						/* restore registers */
	ret

.error:

	cli
	hlt

//! The older version, uses CHS parameters for reading sectors
//! 	%eax = starting sector number
//!

read_sectors_oldchs:

	//! specifies the buffer to use for the kernel to load into

    movw    $_KERNEL_LOAD_SEG,  %ax  	   /* currently 0x1000 */
    movw    %ax,                %es        /* cant directly move to seg reg */
    xorw    %bx,                %bx        /* clear %bx (offset = 0) */

    //! set up the registers for the disk read (int 0x13)

    movb    $0x2,               %ah        /* function select: read sectors */
    movb    $KERNEL_SECTORS,    %al        /* read all sectors needed */
    movb    $0x0,               %ch        /* cylinder 0 */
    movb    $0x3,               %cl        /* sector # 3 */
    movb    $0x0,               %dh        /* head 0 */
    movb    $0x80,              %dl        /* drive 0x00 (floppy disk) */

    /* read the kernel from the disk and check
		for errors (ah=0x00 means success) */
    int     $0x13
    cmpb    $0x00,              %ah        /* check if the read was ok */
    jne     ._error

    ret
    
    ._error:
    
    hlt


//! the code after this will be executed in protected mode

.code32

load_descriptors:

	movw 	$DATA_SEG_OFFSET, %ax
    movw 	%ax, 			  %ds
    movw 	%ax, 			  %es
    movw 	%ax, 			  %fs
    movw 	%ax, 			  %gs
    movw 	%ax, 			  %ss

	//! the stack is going to be the same

	movl 	$_STACK_TOP_PROT, %ebp
	movl 	%ebp, 			  %esp

//! rebase the kernel to 1MB first (paging structures setup near 0x10000)

rebase:

	movl 	$_INIT_PHYS0, 	%esi
	movl	$_INIT_PHYS1, 	%edi
	movl	$KERNEL_BYTES, 	%ecx

	.copy_loop:
	rep 	movsb

/* now that the kernel code is out of the way we can start setting up the 
	paging structures */ 

//! primarily need to setup page tables for identity mapping the first 1MB 
//! 	and then for the kernel at 3GB

set_identity_map:

	//! clear the pdir and the identity map table (right next to pd)

	xorl 	%eax, 			 %eax
	movl 	$_INIT_PDIR, 	 %edi
	movl    $2048, 			 %ecx		/* 2 * 1024 */

	rep 	stosl						/* stores dw %eax in (%edi++) */
	
	//! setup the identity mapping for the low memory (first MB)

	xorl 	%eax, 			 %eax       /* base phys addr in eax */
	movl 	$_INIT_IDENTMAP, %edi		/* setup at 0x5000 */
	movl    $256,            %ecx		/* 1024 entries in the PT */

	.fill_pt:
	
	movl 	%eax, 			 %ebx
	orl 	$0x3, 			 %ebx	 	/* set the present bit */
	movl 	%ebx, 			 (%edi)	 	/* store the phys addr in the PT */

	addl 	$0x1000, 		 %eax		/* increment the phys addr by 4KB */
	addl 	$4, 			 %edi		/* increment the PT entry addr */
	loop 	.fill_pt

	//! now set the page directory entry for the identity map
	//! 	note that it should be the first entry in the pdir, which is 
	//! 	responsible for the first 4MB of memory

	movl 	$_INIT_IDENTMAP, %eax
	movl	$_INIT_PDIR,	 %edi
	orl		$0x3, 			 %eax
	movl 	%eax,			 (%edi)


//! setup the page table for the kernel at 3GB (0xC0000000)
//!  this is a single page table that maps the kernel at 3GB
//!  corresponding PDE is 768

//! (removed currently for a physmap instead) 
// set_high_map:
	
// 	movl 	$_INIT_PHYS1,	 %eax       /* start physical base in eax */
// 	movl 	$_INIT_PTABLE, 	 %edi		/* setup at 0x4000 */
// 	movl    $1024,           %ecx		/* 1024 entries in the PT */

// 	.fill_pt_high:

// 	movl 	%eax, 			 %ebx
// 	orl 	$0x3, 			 %ebx	 	/* set the present bit */
// 	movl 	%ebx, 			 (%edi)	 	/* store the phys addr in the PT */

// 	addl 	$0x1000, 		 %eax		/* increment the phys addr by 4KB */
// 	addl 	$4, 			 %edi		/* increment the PT entry addr */
// 	loop 	.fill_pt_high

// 	//! map it to the corresponding directory entry

// 	movl 	$_INIT_PTABLE, 	 %eax
// 	movl	$_INIT_PDIR,	 %edi
// 	orl		$0x3, 			 %eax
// 	movl 	%eax,			 768*4(%edi)

//! setup a physmap for 1:1 mapping for 0x0 - 64 MB, mapped to 3GB onwards
//! 	this will be replaced with the one generated by the kernel 

set_physmap:

	//! first create the PDEs for the physmap

	movl 	$_INIT_PDIR, 	%edi       /* Page directory base */
    movl 	$_INIT_PTABLE, 	%esi       /* Start of page tables */
    movl 	$768, 			%ebx       /* Starting PDE index */
    movl 	$16, 			%ecx       /* Number of PDEs to create  */

	/* loops 16 times to setup PDEs for the contiguous PTs */
	pde_loop:
	movl    %esi, 			%eax			
	orl 	$0x3, 			%eax
	movl 	%eax,			(%edi, %ebx, 4)
	addl 	$0x1000,		%esi		/* move to the next PT addr */
	incl	%ebx						/* increment the PDE index */
	loop 	pde_loop

	/* once the PDEs are created, setup the page tables */
	movl 	$_INIT_PTABLE, 	%edi       # EDI = start of all PTs
    xorl 	%eax, 			%eax       # EAX = physical address
    movl 	$16384,		 	%ecx       # Total number of PTEs (16 * 1024)

	.pte_loop:
	movl 	%eax, 			%edx       # EDX = physical address
    orl  	$0x3, 			%edx       # Add flags
    movl 	%edx, 			(%edi)     # Store PTE
    addl 	$4, 			%edi       # Next PTE slot
    addl 	$0x1000, 		%eax       # Next physical page
    decl 	%ecx
    jnz 	.pte_loop

//! next steps: enable paging and jump to the kernel

paging_enable:

	movl 	$_INIT_PDIR, 	 %eax		/* load the page directory base */
	movl 	%eax, 			 %cr3		/* load the CR3 register */

	movl 	%cr0, 			 %ebx
	orl 	$0x80000000, 	 %ebx		/* bit31 is paging enable */
	movl	%ebx, 			 %cr0

.after:

	/* finally jump to the kernel entry point */
	jmp 	*_INIT_VIRTUAL+_INIT_PHYS1+0x18

	hlt
