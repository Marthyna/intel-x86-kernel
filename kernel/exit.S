    .text
    .globl exit_handler 

exit_handler:
    pushl %eax
    call exit