
OUTPUT_ARCH(arm)
ENTRY(start)
SECTIONS
{
	. = 0x1000000;
	.text : {*(.text)}
	.data : {*(.data)}

}
