/**
 * @file boot_print.s
 * @author Abdul Rafay (abdul.rafay@lums.edu.pk)
 * 
 * @brief BIOS print utilities.
 * This file contains utility functions for printing characters, strings,
 * newlines, and hexadecimal numbers to the screen in the x86 BIOS video mode.
 * Uses BIOS interrupt 0x10 for character output.
 *
 */

.global boot_print_line
.global boot_print_string
.global boot_print_hex

.global boot_print_newline
.global boot_print_char

.code16

/**
 * Print a string along with a new line.
 *
 * @param %bx Expects pointer to string in BX
 */
.type boot_print_line, @function
boot_print_line:
    call   boot_print_string    # print the string

    # falls through to newline

/**
 * Prints a newline on the screen by printing a carriage return and a newline character.
 *
 */
.type boot_print_newline, @function
boot_print_newline:
    
    movw   $0x0e0a, %ax         # AH=0x0e (teletype), AL=0x0a (newline)
    int    $0x10                # print newline
    movb   $0x0d,  %al          # load the carriage return character into AL ('\r' = 0x0d)
    int    $0x10

    ret


/**
 * Print a string on the screen (early bios cga)
 * 
 * @param string Expects pointer to string in BX
 */
.type boot_print_string, @function
boot_print_string:

    movw   %bx,    %si          # set %si to point to the string
    movb   $0x0e,  %ah          # set function number for BIOS teletype output

    .print_loop:
    lodsb                       # load the first character into %al (and inc %si)
    testb  %al,    %al          # check if the character is null (0)
    je     .done                # if null, jump to done
    int    $0x10                # print the character in AL
    jmp    .print_loop          # keep looping

    .done:
    ret

/**
 * Print a 16-bit hexadecimal number.
 *
 * @param number Expects number to print in AX
 */
boot_print_hex:
    pusha

    movw   %ax,    %dx          # copy the number to DX

    # write 0x before printing the digit
    movb   $0x0e,  %ah          # set function number for BIOS teletype output
    movb   $'0',   %al          # load '0' into AL
    int    $0x10                # print '0'
    movb   $'x',   %al          # load 'x' into AL
    int    $0x10                # print 'x'

    movw   $4,     %cx          # set the loop counter to 4
                                # will be clear in loop iters
    
    .print_hex_loop:
    rolw   $4,     %dx          # rotate the number left by 4 bits
    movb   %dl,    %al          # move the low nibble to AL
    andb   $0x0f,  %al          # mask the high nibble
    cmpb   $0x09,  %al          # check if the number is greater than 9
    jbe    .print_hex_digit     # if not, print the number

    addb   $0x07,  %al          # else convert the number to a letter
    
    .print_hex_digit:
    addb   $'0',   %al          # convert the number to ASCII
    int    $0x10

    loop   .print_hex_loop      # loop until all nibbles are printed

    popa
    ret
