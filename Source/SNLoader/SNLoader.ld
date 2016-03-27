OUTPUT_FORMAT("binary")
ENTRY("Loader16")
SECTIONS{
	. = 0x2000;

	.setup :
	{
		*(.setup*)
		*(.bits16*)
	}

	.text :
	{
		*(.text)
	}

	.data :
	{
		*(.data)
		*(.rodata*)
	}

	.bss :
	{
		*(COMMON*)
		*(.bss*)
	}
	
	 /DISCARD/ :
	 {
		*(.comment)
	 }

}
