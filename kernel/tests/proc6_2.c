#if defined microblaze
    __asm__(
        ".text\n"
        ".globl proc6_2\n"
        "proc6_2:\n"
        "addk r3,r0,r5\n"
        "swi r3,r1,-4\n"
        "rtsd r15,8\n"
        "nop\n"
        ".previous\n");
#else
    __asm__(
        ".text\n"
        ".globl proc6_2\n"
        "proc6_2:\n"
        "movl 4(%esp),%eax\n"
        "pushl %eax\n"
        "popl %eax\n"
        "ret\n"
        ".previous\n");
#endif