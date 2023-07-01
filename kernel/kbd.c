#include "kbd.h"
#include "console.h"
#include "stdbool.h"
#include "stdio.h"
#include "timer.h"
#include "processManager.h"
#include "interrupts.h"
#include "isr.h"

#define BUFFER_SIZE 1000
#define IRQ_INDEX_KEYBOARD 33  

#define IO_PORT 0x60

char keyboard_buffer[BUFFER_SIZE];
bool echo = true;
int write_position = 0;
int read_position = 0;

/*KEYBOARD*/
void keyboard_config(){
    
    define_interrupt(IRQ_INDEX_KEYBOARD, keyboard_interrupt_handler, PL_USER);
    mask_IRQ(1, false);
}

//at each keyboard interrupt, the corresponding scancode is retrieved by an instruction 
void keyboard_interrupt(){
    int scancode = inb(IO_PORT);
    do_scancode(scancode);
    PIC_sendEOI(IRQ_INDEX_KEYBOARD);
}

void keyboard_data(char *str){
    int i = 0;

    while ((str[i] != '\0')){

        //write in the buffer
        keyboard_buffer[write_position] = str[i];
        write_position++;

        // if str reaches the end of the buffer, we go back to the beginning (overwrite)
        //Lorsque le tampon du clavier est plein, ces caractères sont ignorés. 
        //Un bip peut éventuellement signaler cet événement à l'utilisateur.
        //https://datamove.imag.fr/gregory.mounie/psys-doc/Aspects_techniques/
        if (write_position == BUFFER_SIZE)
            write_position = 0;

        if(echo)
            echo_command(str[i]);
        i++;
    }
}

void echo_command(char c){
    //characters 9 and 32 to 126 are displayed normally,
    if (c == 9 || (c >= 32 && c <= 126))
        printf("%c", c);
    //the end-of-line marker, 13, is displayed as character 10 in normal display,
    else if (c == '\r')
        printf("\n");
    //the characters lower than 32 are displayed by the character \^ followed by the character 64 + code 
    else if (c < 32)
        printf("^%c", 64 + c);
    //delete character
    else if (c == 127)
        printf("\b"); 
    else
        return;
}

// TO DO void kbd_leds(unsigned char leds);

/*CONSOLE*/
int cons_read(char *string, unsigned long length){
    //if length is null, this function returns 0.
    if(length == 0)
        return 0;
    
    unsigned long char_length = 0;
    
    while(char_length < length) {

        //while theres no character, wait for it
        while (keyboard_buffer[read_position] == 0) {
            wait_clock(current_clock()+1);
            // read_position = read_position == BUFFER_SIZE - 1 ? 0 : read_position + 1;
        }

        //if its an enter, don't count as a char 
        if (keyboard_buffer[read_position] == '\r') {
            read_position = read_position == BUFFER_SIZE - 1 ? 0 : read_position + 1;
            break;
        } 
        
        //if it's a backspace, subtract 
        else if ((int)keyboard_buffer[read_position] == 127) {
            char_length = char_length > 0 ? char_length - 1 : char_length;
        }
        
        //otherwise, increment
        else if ((int)keyboard_buffer[read_position] != 127) {
            string[char_length] = keyboard_buffer[read_position];
            char_length++;
        }

        read_position = read_position == BUFFER_SIZE - 1 ? 0 : read_position + 1;
    }

    return char_length;
}

void cons_echo(int on){
    if(on == 0)
        echo = false;
    else 
        echo = true;
}
