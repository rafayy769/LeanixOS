/**
 * @file load_gdt.s
 * @author Abdul Rafay (abdul.rafay@lums.edu.pk)
 * 
 * @brief Assembly implementation to load the Global Descriptor Table (GDT)
 * and set up the segment registers.    
 */

.global load_gdt
.global load_idt

SEGC_OFFS = 0x08                     # Offset for the kernel code segment in the GDT
SEGD_OFFS = 0x10                     # Offset for the kernel data segment in the GDT

.type load_gdt, @function
load_gdt:
    # Load the GDT
    movl    4(%esp),     %eax        # Load the GDT descriptor address from the stack
    lgdt     (%eax)                  # Load the GDT into the GDTR register

    # Set up the the data segment registers
    movw    $SEGD_OFFS,  %ax
    movw    %ax,         %ds
    movw    %ax,         %es
    movw    %ax,         %fs
    movw    %ax,         %gs
    movw    %ax,         %ss
    
    # long jump to the code segment offset (which sets up the CS register implicitly)
    ljmp   $SEGC_OFFS,   $.flush
    
.flush:    
    # Return from the function
    ret


.type load_idt, @function
load_idt:
    # Load the IDT
    movl    4(%esp),     %eax        # Load the IDT descriptor address from the stack
    lidt     (%eax)                  # Load the IDT into the IDTR register

    # Return from the function
    ret
