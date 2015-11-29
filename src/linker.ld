
ENTRY(_start)

SECTIONS {
  /* reserve one page at 1MB for "saved state", then the bootloader. */
	. = 1M + 4K;

	.text BLOCK(4K) : ALIGN(4K) {
		*(.multiboot)
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K) {
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K) {
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K) {
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}

  /* finally, our 64-bit kernel image. */
  .kernel64 BLOCK(4K) : ALIGN(4K) {
    *(.kernel64)
  }
}
