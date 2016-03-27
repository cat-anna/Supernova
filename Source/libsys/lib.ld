ENTRY("LibMain")
SECTIONS{
	.text 0x0:
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
