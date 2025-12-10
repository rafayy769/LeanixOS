/**********************************************
*
* @file    boot.lds
* @author  Abdul Rafay (abdul.rafay@lums.edu.pk)
*
* @brief   This is convenient if only a single stage bootloader is needed.
*          A single bootsector reads the entire kernel and jumps to it.
*
***********************************************/

ENTRY (__bootsector_start);           /* Comes from stage1 entrypoint */

SECTIONS
{
  .         = 0x7c00;                 /* Code starts at 0x7c00 */

  .text     : { *(.text) }
  .data     : { *(.data) }
  
  .         = 0x7c00 + 510;
  
  .sig      : { SHORT(0xaa55); }      /* Boot signature at the end */

}
