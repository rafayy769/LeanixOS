/**********************************************
*
* @file    stage2.lds
* @author  Abdul Rafay (abdul.rafay@lums.edu.pk)
* @brief   Use this linker script if you want a 2 stage bootloader.
*          Each stage is 512bytes in size, to fit in a single flpy sector.
*
***********************************************/

ENTRY (__bootsector_start);


SECTIONS
{

  /* bootsector code starts at 0x7c00 */
  .         = 0x7c00;

  /* put all the text and data section together */
  .text     : { *(.text) }       /* coming from stage1 */
  .data     : { *(.data) }
  
  /* boot signature at the end of the sector */
  .         = 0x7c00 + 510;
  .sig      : { SHORT(0xaa55); }

  /* stage2 code is stored in a separate section, it is put next */
  .stage2   : { *(.stage2) }

}