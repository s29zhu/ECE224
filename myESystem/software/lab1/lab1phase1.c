#include "alt_types.h"
#include <stdio.h>
#include <unistd.h>
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"
#include "stdbool.h"

volatile int edge_capture;

static void LED_on(void)
{
  volatile alt_u8 led;
  /* Turn the LEDs on. */
  led = 0x1;
  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, led);
  usleep(1000000);
}
static void LED_off(void)
{
  volatile alt_u8 led;
  /* Turn the LEDs on. */
  led = 0x0;
  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, led);
  usleep(1000000);
}
/**Use interrupting synchronization**/
#ifdef BUTTON_PIO_NAME
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
static void handle_button_interrupts(void* context)
#else
static void handle_button_interrupts(void* context, alt_u32 id)
#endif
{
  /* Cast context to edge_capture's type.
   * It is important to keep this volatile,
   * to avoid compiler optimization issues.
   */
  volatile int* edge_capture_ptr = (volatile int*) context;
  /* Store the value in the Button's edge capture register in *context. */
  *edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE);
  /* Reset the Button's edge capture register. */
  IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0);

  /*
   * Read the PIO to delay ISR exit. This is done to prevent a spurious
   * interrupt in systems with high processor -> pio latency and fast
   * interrupts.
   */
  IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE);
}

/* Initialize the button_pio. */

static void init_button_pio()
{
  /* Recast the edge_capture pointer to match the alt_irq_register() function
  * prototype. */
  void* edge_capture_ptr = (void*) &edge_capture;
  /* Enable all 4 button interrupts. */
  IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTON_PIO_BASE, 0xf);
  /* Reset the edge capture register. */
  IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0x0);

#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
  alt_ic_isr_register(BUTTON_PIO_IRQ_INTERRUPT_CONTROLLER_ID, BUTTON_PIO_IRQ,
    handle_button_interrupts, edge_capture_ptr, 0x0);
#else
  alt_irq_register( BUTTON_PIO_IRQ, edge_capture_ptr,
    handle_button_interrupts);
#endif
}

/* Tear down the button_pio. */

static void disable_button_pio()
{
  /* Disable interrupts from the button_pio PIO component. */
  IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTON_PIO_BASE, 0x0);
  /* Un-register the IRQ handler by passing a null handler. */
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
  alt_ic_isr_register(BUTTON_PIO_IRQ_INTERRUPT_CONTROLLER_ID, BUTTON_PIO_IRQ,
    NULL, NULL, NULL);
#else
  alt_irq_register( BUTTON_PIO_IRQ, NULL, NULL );
#endif
}

static void TestButtons( void )
{
  volatile alt_u8 led;
  int read_result = 0;
  _Bool flag = true;
  /* Variable which holds the last value of edge_capture to avoid
   * "double counting" button/switch presses
   */
  /* Initialize the Buttons/Switches (SW0-SW3) */
  init_button_pio();
  edge_capture = 0;
  led = 0x0;

  while (1 == 1)
  {
	  read_result = IORD(LED_PIO_BASE,0);

      switch (edge_capture)
      {
        case 0x1:
        	//if flag == true
        	if(read_result && flag){
        		printf("1pressed button1 and edge_capture = %d.\n",edge_capture);
        		led = 0x0;
        		edge_capture = 0;
        		IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, led);
        		flag = false;
        	}

        	else if(flag){
        		printf("2pressed button1 and edge_capture = %d.\n",edge_capture);
        		led = 0x1;
        		IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, led);
        		edge_capture = 0;
        		flag = false;
        	//	usleep(1000000);
        	}
        	else{
        		flag = true;
        		edge_capture = 0;
        	}

          break;
          /*You can add other button controllers here
           *  with case 0x2, case 0x4, case 0x8
           * */
      }
  }
  /* Disable the button pio. */
  disable_button_pio();
  usleep(2000000);
  return;
}
#endif

#ifdef SWITCH_PIO_BASE
static int ReadSwitches(void){
	alt_16 switch_value = 0;
	alt_8 mask = 0x00ff;
	// read the 16-bit value of switches combinations, if a switch is on, then it is 1, otherwise, it is just 0.
	switch_value = IORD_ALTERA_AVALON_PIO_DATA(SWITCH_PIO_BASE);
	switch_value &= mask;
	return switch_value;
}
#endif

#ifdef SEVEN_SEG_PIO_BASE
/*set the seven segments to show '0'*/
static void SetSevenSeg_0(void){
	alt_32 bits = 0xff81;
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_PIO_BASE, bits);
	usleep(1000000);
}
/*set the seven segments to show '1'*/
static void SetSevenSeg_1(){
	alt_32 bits = 0xffcf;
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_PIO_BASE, bits);
	usleep(1000000);
}
static void SetSevenSeg_none(){
	alt_32 bits = 0xffff;
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_PIO_BASE, bits);
}
#endif

static void led_display(){
	alt_16 switch_value = 0;
	int i = 0;
	switch_value = ReadSwitches();
	_Bool press_flag = true;
	edge_capture = 0;
	init_button_pio();
	while(1 == 1){
		if(edge_capture && press_flag){
			press_flag = false;
			edge_capture = 0;
			switch_value = ReadSwitches();
			/*displaying switches' value to LED or seven-segment*/
			for(i = 0; i < 8; i ++){
				if (edge_capture && !press_flag){//get interruption from button release
					edge_capture = 0;
					press_flag = true;
				}else if(edge_capture)//get interruption from reseting the switches
					break;
				/*If the next bit is 1 then turn on the led*/
				if (switch_value & 1)
					LED_on();
				else
				/*If the next bit is 0 then turn off the led*/
					LED_off();
				switch_value >>=  1;
			}
			LED_off();
		}else if(edge_capture){//get interruption from button release
			edge_capture = 0;
			press_flag = true;
		}
	}
	disable_button_pio();
}

static void seven_seg_display(){
	alt_16 switch_value = 0;
	int i = 0;
	switch_value = ReadSwitches();
	_Bool press_flag = true;
	edge_capture = 0;
	init_button_pio();
	while(1 == 1){
		if(edge_capture && press_flag){
			press_flag = false;
			edge_capture = 0;
			switch_value = ReadSwitches();
			for(i = 0; i < 8; i ++){
				if (edge_capture && !press_flag){
					edge_capture = 0;
					press_flag = true;
				}else if(edge_capture)
					break;
				/*If the next bit is 1 then turn on the led*/
				if (switch_value & 1)
					SetSevenSeg_1();
				else
				/*If the next bit is 0 then turn off the led*/
					SetSevenSeg_0();
				switch_value >>=  1;
			}
			SetSevenSeg_none();
		}else if(edge_capture){
			edge_capture = 0;
			press_flag = true;
		}
	}
	disable_button_pio();
}
/*int main(void){
//	led_display();
	seven_seg_display();
	return 1;
}*/
