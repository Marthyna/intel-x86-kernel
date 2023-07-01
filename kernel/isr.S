#define interrupt_service_routine(function) \
    .globl function##_handler; \
function##_handler: \
    pushl %eax; \
    pushl %edx; \
    pushl %ecx; \
    call  function; \
    popl %ecx; \
    popl %edx; \
    popl %eax; \
    iret

interrupt_service_routine(clock_interrupt)
interrupt_service_routine(keyboard_interrupt)
