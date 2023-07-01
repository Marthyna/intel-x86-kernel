/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 */

#include <stdio.h>

#include "processManager.h"

int assert_failed(const char *cond, const char *file, int line) {
    printf("%s:%d: assertion '%s' failed.\n", file, line, cond);
    *(char *)0 = 0;

    exit(-1);
    while (1)
        ;
}
