// Libraries required
#include "alt_types.h"  // define types used by Altera code, e.g. alt_u8
#include <stdio.h>
#include <unistd.h>
#include "system.h"  // constants such as the base addresses of any PIOs
                     // defined in your hardware
#include "sys/alt_irq.h"  // required when using interrupts
#include <io.h>
#include "altera_avalon_timer_regs.h"  // timer register constants

// declare global variable that is a flag for the main program
// use volatile so the compiler will not optomize away your code
volatile alt_u8 count_flag = (alt_u8)0x00;

#ifdef ALT_MODULE_CLASS_timer_0  // only compile this code if there is a sys_clk_timer
static void timer_ISR(void* context, alt_u32 id)
{
   // acknowledge the interrupt by clearing the TO bit in the status register
   IOWR(TIMER_0_BASE, 0, 0x0);
   // set the flag with a non zero value
   count_flag = 0xf;
}
#endif


int main(void)
{

   alt_u32 timerPeriod;  // 32 bit period used for timer
   int count;  // value that is printed by loop

	#ifdef ALT_MODULE_CLASS_timer_0
   // calculate timer period for 2 seconds
   timerPeriod = 2 * ALT_CPU_CPU_FREQ;

   // initialize timer interrupt vector
   alt_irq_register(TIMER_0_BASE, (void*)0, timer_ISR);

   // initialize timer period
   IOWR(TIMER_0_BASE, 2, (alt_u16)timerPeriod);
   IOWR(TIMER_0_BASE, 3, (alt_u16)(timerPeriod >> 16));

   // clear timer interrupt bit in status register
   IOWR(TIMER_0_BASE, 0, 0x0);

   // initialize timer control - start timer, run continuously, enable interrupts
   IOWR(TIMER_0_BASE, 1, 0x7);

   // start counting loop
   count = 20;
   printf("Starting count down\n");
   while (count >= 0)
   {
      if (count_flag != 0)
      {
          printf("%d\n",count);
          count = count - 2;
          count_flag = 0;  // don't forget to clear the flag!
      }
   }
   printf("Blast off!\n");
#endif

  return(0);
}
