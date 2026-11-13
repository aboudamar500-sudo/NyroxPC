#ifndef NYROX_CONSOLE_H
#define NYROX_CONSOLE_H

#include "boot_info.h"

int console_init(const NyroxBootInfo *info);
void console_clear(void);
void console_putc(char c);
void console_write(const char *text);

#endif
