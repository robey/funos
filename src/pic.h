#pragma once

#define PIC1_CMD        0x0020
#define PIC1_DATA       0x0021
#define PIC2_CMD        0x00a0
#define PIC2_DATA       0x00a1
#define APIC_BASE_MSR   0x1b

void pic_init(void);
