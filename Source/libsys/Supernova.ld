ENTRY("_start")
SECTIONS{
	.text 0x01000000:
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
