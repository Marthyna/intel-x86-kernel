#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <stdint.h>
#include "console.h"

#define MEM_VIDEO 0xB8000
#define MAX_LIG 25
#define MAX_COL 80
#define T_SIZE 2  // tamanho do bloco caractere
#define STD_TEXT_COLOR VERT
#define STD_BG_COLOR NOIR
#define TAB_SIZE 8

enum couleur {
    NOIR,
    BLUE,
    VERT,
    CYAN,
    ROUGE,
    MAGENTA,
    MARRON,
    GRIS,
    GRIS_FONCE,
    BLUE_CLAIR,
    VERT_CLAIR,
    CYAN_CLAIR,
    ROUGE_CLAIR,
    MAGENTA_CLAIR,
    JAUNE,
    BLANC
};

typedef struct cursor {
    uint32_t lig;
    uint32_t col;
} Cursor;
Cursor get_cursor();

uint32_t get_position(uint32_t lig, uint32_t col);
uint16_t* get_ptr_mem(uint32_t lig, uint32_t col);
void write_colored_char(
    uint32_t lig, uint32_t col, char c,
    uint32_t c_texte, uint32_t c_fond, uint32_t clignote);
void write_char(uint32_t lig, uint32_t col, char c);
void clear_screen(void);
void place_cursor(uint32_t lig, uint32_t col);
char get_char(uint32_t lig, uint32_t col);
void treat_char(char c);
void scroll_down(void);
void console_putbytes(const char* s, int len);
void cons_write(const char *str, long size);

#endif
