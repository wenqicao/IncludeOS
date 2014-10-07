#include <os>
#include <stdio.h>
#include <assert.h>

#include <class_dev.hpp>
#include <class_service.hpp>

// A private class to handle IRQ
#include "class_irq_handler.hpp"
#include <class_pci_manager.hpp>

bool OS::power = true;

void OS::start()
{
  rsprint(">>> OS class started\n");
  
  __asm__("cli");  
  IRQ_handler::init();
  Dev::init();  
  
  //Everything is ready
  Service::start();
  
  __asm__("sti");
  halt();
};

extern "C" void halt_loop(){
  __asm__ volatile("hlt; jmp halt_loop;");
}

void OS::halt(){
  OS::rsprint("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  OS::rsprint(">>> System idle - everything seems OK \n");
  while(power){        
    //DEBUG Disable interrupts while notifying
    //__asm__ volatile("cli");
    IRQ_handler::notify(); 
    //__asm__ volatile("sti");    
    
    // Pending clear
    // ! IRQ! -> Flag set
        
    printf("<OS> Woke up! \n");
  }
  
  //Cleanup
  //Service::stop();
}

int OS::rsprint(const char* str){
  char* ptr=(char*)str;
  while(*ptr)
    rswrite(*(ptr++));  
  return ptr-str;
}


/* STEAL: Read byte from I/O address space */
uint8_t OS::inb(int port) {  
  int ret;

  __asm__ volatile ("xorl %eax,%eax");
  __asm__ volatile ("inb %%dx,%%al":"=a" (ret):"d"(port));

  return ret;
}


/*  Write byte to I/O address space */
void OS::outb(int port, uint8_t data) {
  __asm__ volatile ("outb %%al,%%dx"::"a" (data), "d"(port));
}



/* 
 * STEAL: Print to serial port 0x3F8
 */
int OS::rswrite(char c) {
  /* Wait for the previous character to be sent */
  while ((inb(0x3FD) & 0x20) != 0x20);

  /* Send the character */
  outb(0x3F8, c);

  return 1;
}





