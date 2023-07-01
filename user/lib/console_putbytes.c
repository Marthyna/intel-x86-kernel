/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Stub for console_putbytes system call.
 */

#include "console.h"

void console_putbytes(const char *s, int len){
    (void) s;
    (void) len;
    cons_write(s, len);
}
