#include "stm32f4xx.h"
#include "clockconfig.h"
#include "main.h"

#include <USART.h>
#include <GPIO.h>

custom_libraries::clock_config system_clock;
custom_libraries::USART serial_port(USART2,SERIAL_PORT,SERIAL_RX,SERIAL_TX,SERIAL_BAUD);
custom_libraries::USART cell_modem(USART1,MODEM_PORT,MODEM_RX,MODEM_TX,MODEM_BAUD);
custom_libraries::_GPIO user_button(USER_BUTTON_PORT, USER_BUTTON_PIN);

/* USART data receive Interrupt */
extern "C" void USART1_IRQHandler(void){
  if(USART1->SR & USART_SR_RXNE){
    USART1->SR &= ~USART_SR_RXNE;
    cell_modem.read_string();
  }
}

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
    for(volatile int i = 0; i < 100000; i++){};
    bootloader_jump_to_user_application();
  }
 
  while(1){}
}


void bootloader_jump_to_user_application(void){
  /* Create function pointer to hold reset handler address of user application */
  void(*user_application_reset_handler)(void);

  /* Configure the Main Stack Pointer from reading the base adress of user application flash section (Sector 3)*/
  uint32_t main_stack_pointer_value = *(volatile uint32_t*)FLASH_SECTOR3_BASE_ADDRESS;

  /* This function comes from CMSIS */
  __set_MSP(main_stack_pointer_value);

  /* Fetch the reset handler address of the user applicaton, it is stored in (Base address + 4) */
  uint32_t reset_handler_address =  *(volatile uint32_t*)(FLASH_SECTOR3_BASE_ADDRESS+4);  
  user_application_reset_handler = (void (*)()) reset_handler_address;

  /* Jump to reset handler of the user application */
  user_application_reset_handler();

}

void bootloader_uart_read_data(void){
  /* Activate GPRS to read FTP data from AWS S3 */
  char AT[] = "AT\r\n";
  char ATSAPBR[] = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n"; //Configure bearer profile 1
  char ATSAPBR_APN[] = "AT+SAPBR=3,1,\"APN\",\"iot.safaricom.com\"\r\n"; 
  char ATSAPBR_1[] = "AT+SAPBR=1,1\r\n";
  char ATSAPBR_2[] = "AT+SAPBR=2,1\r\n";
  char ATSAPBR_0[] = "AT+SAPBR=0,1\r\n";
  char ATFTPCID[] = "AT+FTPCID=1\r\n";
  char ATFTPSERV[] = "AT+FTPSERV=\"sfs-us-east-1.docevent.io\"\r\n";
  char ATFTPUN[] = "AT+FTPUN=\"8ufd12uk/MCLEANS\"\r\n";
  char ATFTPPW[] = "AT+FTPPW=\"******\"\r\n";
  char ATFTPGETNAME[] = "AT+FTPGETNAME=\"1K.txt\"\r\n";
  char ATFTPGETPATH[] = "AT+FTPGETPATH=\"/\"\r\n";
  char ATFTPGET[] = "AT+FTPGET=1\r\n";
  char ATFTPGET_BYTES[] = "AT+FTPGET=2,1024\r\n";

  /* Initialize cell modem */
  cell_modem.initialize();
  /* Set Interrupt priority */
  NVIC_SetPriority(USART1_IRQn,0x01);
  /* Enable Interrupt */
  NVIC_EnableIRQ(USART1_IRQn);

  cell_modem.print(AT);
  cell_modem.print(ATSAPBR);
  cell_modem.print(ATSAPBR_APN);
  cell_modem.print(ATSAPBR_1);
  cell_modem.print(ATSAPBR_2);
  cell_modem.print(ATFTPCID);
  cell_modem.print(ATFTPSERV);
  cell_modem.print(ATFTPUN);
  cell_modem.print(ATFTPPW);
  cell_modem.print(ATFTPGETNAME);
  cell_modem.print(ATFTPGETPATH);
  cell_modem.print(ATFTPGET);
  cell_modem.print(ATSAPBR_0);
  cell_modem.flush_buffer();

  for(volatile int i = 0; i < 20 ; i++){
    cell_modem.print(ATFTPGET_BYTES);
    /* Wait for response */
    /* Print response */
    serial_port.println(cell_modem.receive_buffer);
    /* Flush the buffer */
    cell_modem.flush_buffer();
  }
  
} 