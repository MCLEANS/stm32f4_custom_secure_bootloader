#include "stm32f4xx.h"
#include "clockconfig.h"
#include "main.h"

#include <USART.h>
#include <GPIO.h>

custom_libraries::clock_config system_clock;
custom_libraries::USART serial_port(USART2,USART_PORT,USART_RX,USART_TX,USART_BAUD);
custom_libraries::_GPIO user_button(USER_BUTTON_PORT, USER_BUTTON_PIN);

int main(void) {

  /* Initialize the bootloader here */
  system_clock.initialize();
  serial_port.initialize();

  /* Configure user button as input */
  user_button.pin_mode(custom_libraries::INPUT);
  user_button.input_state(custom_libraries::PULL_UP);

  /**
   * Check whether the user button is pressed 
   * If pressed execute bootloader commands
   * If not pressed jump to user application
  **/
  if(user_button.digital_read()){
    /* Button is pressed, perform firmware update */
    serial_port.println("[Bootloader] - Performing Firmware Update");
    bootloader_uart_read_data();
  }
  else{
    /* Button is not pressed, jump to user application */
    serial_port.println("[Bootloader] - Jumping to user application");
    bootloader_jump_to_user_application();
  }
 
  while(1){}
}


void bootloader_jump_to_user_application(void){

}

void bootloader_uart_read_data(void){

}