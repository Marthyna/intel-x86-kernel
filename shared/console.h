#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/*
 * This is the function called by printf to send its output to the screen. You
 * have to implement it in the kernel and in the user program.
 */
extern void console_putbytes(const char *s, int len);
extern void cons_write(const char *str, long size);
extern void cons_echo(int on);
extern int cons_read(char *string, unsigned long length);

#endif
