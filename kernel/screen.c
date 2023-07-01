#include "screen.h"

#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define CEIL(X) ((X - (int)(X)) > 0 ? (int)(X + 1) : (int)(X))

Cursor cursor = {
    .lig = 0,
    .col = 0};

Cursor get_cursor() {
    return cursor;
}

uint32_t get_position(uint32_t lig, uint32_t col) {
    return (uint32_t)(lig * MAX_COL + col);
}

uint16_t *get_ptr_mem(uint32_t lig, uint32_t col) {
    return (uint16_t *)(MEM_VIDEO + T_SIZE * get_position(lig, col));
}

void write_char(uint32_t lig, uint32_t col, char c) {
    write_colored_char(lig, col, c, STD_TEXT_COLOR, STD_BG_COLOR, 0);
}

void write_colored_char(
    uint32_t lig, uint32_t col, char c,
    uint32_t c_texte, uint32_t c_fond, uint32_t clignote) {
    uint16_t *ptr = get_ptr_mem(lig, col);
    *ptr = (uint16_t)c;
    *ptr = (*ptr) + (c_texte << 8) + (c_fond << 12) + (clignote << 15);
}

void clear_screen(void) {
    for (int j = 0; j < MAX_LIG; j++) {
        for (int i = 0; i < MAX_COL; i++) {
            write_char(j, i, 0);
        }
    }
}

char get_char(uint32_t lig, uint32_t col) {
    return (char)*get_ptr_mem(lig, col);
}

void place_cursor(uint32_t lig, uint32_t col) {
    cursor.lig = lig;
    cursor.col = col;
    uint32_t cursor_pos = get_position(lig, col);
    uint8_t low = (uint8_t)cursor_pos;
    uint8_t high = (uint8_t)(cursor_pos >> 8);
    /*
   send the command 0x0F on the command port to
   indicate to the board that we are going to send
   the the least-significant part of the cursor position;
    */
    outb(15, 0x3D4);  // 0x0F=15, 0x3D4=980

    // send the least-significant part to the data port
    outb(low, 0x3D5);  // 0x3D5=981

    /*
    send the command 0x0E on the command port to
    signal that we are now sending the most significant part
    */
    outb(14, 0x3D4);  // 0x0E=14, 0x3D4=980

    // send the most significant part of the position to the data port
    outb(high, 0x3D5);
}

void treat_char(char c) {
    Cursor cursor_tmp;
    switch (c) {
        case '\b':
            if (cursor.col > 0) {
                write_char(cursor.lig, cursor.col, 0);  // erases char
                cursor_tmp.lig = cursor.lig - 1;        // send cursor back one position
                cursor_tmp.col = cursor.col;
            }
            break;
        case '\t':
            cursor_tmp.lig = cursor.lig;
            cursor_tmp.col = (CEIL((cursor.col) / 8) + 1) * 8;
            break;
        case '\n':
            cursor_tmp.lig = cursor.lig + 1;
            cursor_tmp.col = 0;
            break;
        case '\f':
            clear_screen();
            cursor_tmp.lig = 0;
            cursor_tmp.col = 0;
            break;
        case '\r':
            cursor_tmp.lig = cursor.lig;
            cursor_tmp.col = 0;
            break;
        default:
            write_char(cursor.lig, cursor.col, c);
            cursor_tmp.lig = cursor.lig;
            cursor_tmp.col = cursor.col + 1;
            break;
    }
    if (cursor_tmp.lig >= 80) {
        cursor.lig = cursor_tmp.lig + 1;
        cursor.col = cursor_tmp.col % 80;
    } else if (cursor_tmp.lig >= 25) {
        scroll_down();
        cursor.lig = 24;
        cursor.col = cursor_tmp.col;
    } else {
        cursor.lig = cursor_tmp.lig;
        cursor.col = cursor_tmp.col;
    }
    place_cursor(cursor.lig, cursor.col);
}

void scroll_down(void) {
    memmove(
        get_ptr_mem(0, 0),
        get_ptr_mem(1, 0),
        T_SIZE * get_position(0, MAX_COL - 10));
    memmove(
        get_ptr_mem(1, 0),
        get_ptr_mem(2, 0),
        T_SIZE * get_position(MAX_LIG - 2, MAX_COL));
    memset(
        (void *)get_ptr_mem(MAX_LIG - 1, 0),
        0,
        MAX_COL);
}

void console_putbytes(const char *s, int len) {
    for (int i = 0; i < len; i++) {
        treat_char(s[i]);
    }
}

/*Display on the terminal
Sends to the terminal the sequence of characters of 
length size at the address str.*/
void cons_write(const char *str, long size){
    console_putbytes(str, size);
}
