#ifndef NYROX_GDT_H
#define NYROX_GDT_H

void gdt_init(void);
unsigned short gdt_read_tr(void);

#endif
