ENTRY("Start")
SECTIONS{
	. = 0xC0010000;

	.text : AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
	}

	.data : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data*)
		*(.rodata*)
		*(.IVolume*)
	}

	.bss : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON*)
		*(.bss*)
	}
	
	_end= . ;
		
	/DISCARD/ :
	{
		*(.comment)
		*(.stab*)
		*(.debug*)
	}
}
