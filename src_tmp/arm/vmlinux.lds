

OUTPUT_ARCH(arm)
ENTRY(stext)
SECTIONS
{
 . = 0xC0000000 + 0x00008000;
 .init : { /* Init code and data		*/
  _stext = .;
   *(.head.text)
   *(.init.text)

 }
 .text : { /* Real text segment		*/
 _text = .; /* Text and read-only data	*/
 __exception_text_start = .;
 *(.exception.text)
 __exception_text_end = .;
 . = ALIGN(8);
 *(.text)
 }
 . = ALIGN(8192);
 __data_loc = .;
 .data : AT(__data_loc) {
 _data = .; /* address in memory */
 . = ALIGN(8192); *(.data..init_task)
 }
 . = ALIGN(0);
  __bss_start = .; 
 . = ALIGN(0); .bss : AT(ADDR(.bss) - 0) { *(.bss) } . = ALIGN(0); __bss_stop = .;
 _end = .;
}
