/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An
  * @version V1.0
  * @date    Nov 26, 2022
  * @brief   ECE 362 Lab 11 student template
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "wifi_uart.h"
#include <stdint.h>

#include <stdio.h>
#include "fifo.h"
#include "tty.h"

char wifi_credentials[128];

void (*check_code_func_ptr)(char*);
void (*save_wifi_credentials_ptr)(const char*, uint16_t);
uint8_t (*get_wifi_credentials_ptr)(char*);

void init_usart5();

void set_wifi_code_check_func_ptr(void (*func_ptr)(char*)) {
	check_code_func_ptr = func_ptr;
}

void set_save_wifi_credentials_ptr(void (*func_ptr)(const char*, uint16_t)) {
	save_wifi_credentials_ptr = func_ptr;
}

void set_get_wifi_credentials_ptr(uint8_t (*func_ptr)(char*)) {
	get_wifi_credentials_ptr = func_ptr;
}

void init_usart5() {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;
    GPIOC -> MODER = (GPIOC-> MODER & ~(GPIO_MODER_MODER12)) | GPIO_MODER_MODER12_1 ;
    GPIOD -> MODER = (GPIOD-> MODER & ~(GPIO_MODER_MODER2)) | GPIO_MODER_MODER2_1;
    GPIOC -> AFR[1] &= ~(0x000f0000); //PC12 TX
    GPIOC -> AFR[1] |=  (0x00020000);
    GPIOD -> AFR[0] &= ~(0x00000f00); //PD2 RX
    GPIOD -> AFR[0] |=  (0x00000200);

    RCC->APB1ENR |= RCC_APB1ENR_USART5EN; //RCC USART5

    USART5->CR1 &= ~USART_CR1_UE; //disable
    USART5->CR1 &= ~0x10001000;
    USART5->CR2 &= ~USART_CR2_STOP; //stop bit
    USART5->CR1 &= ~(USART_CR1_PCE); //no parity
//    USART5->CR1 &= ~(USART_CR1_OVER8);
    USART5->BRR &= ~0xffff;
    USART5->BRR = 0x8C; //115200 baud at 8 MHz clock

//    USART5->BRR = 0x1a1; //115200 baud
//    USART5->BRR = 0x341; //57600 baud
    USART5-> CR1 &= ~(USART_CR1_OVER8); //16x sampling
    USART5->CR1 |= USART_CR1_TE | USART_CR1_RE; //enable TE/RE
    USART5->CR1 |= USART_CR1_UE;
    while (!(USART5-> ISR & (USART_ISR_REACK)));
    while (!(USART5-> ISR & (USART_ISR_TEACK)));
}

// TODO DMA data structures

#define FIFOSIZE 128
char serfifo[FIFOSIZE];
volatile int seroffset = 0;

void enable_tty_interrupt(void) {

    NVIC -> ISER[0] |= (1<<USART3_8_IRQn);

    USART5->CR3 |= USART_CR3_DMAR;
    USART5->CR1 |= USART_CR1_RXNEIE;

    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->RMPCR |= DMA_RMPCR2_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;  // First make sure DMA is turned off

    DMA2_Channel2->CMAR    =     (uint32_t) serfifo;
    DMA2_Channel2->CPAR    =     (uint32_t)&(USART5->RDR);
    DMA2_Channel2->CNDTR   =     FIFOSIZE;
    DMA2_Channel2->CCR    |=     DMA_CCR_MINC;//MINC
    DMA2_Channel2->CCR    &=     ~DMA_CCR_DIR;//Dir
    DMA2_Channel2->CCR    &=     ~(DMA_CCR_MSIZE);//Msize
    DMA2_Channel2->CCR    &=    ~(DMA_CCR_PSIZE); //Psize
    DMA2_Channel2->CCR    |=     DMA_CCR_CIRC;//CIRC
    DMA2_Channel2->CCR    |=     DMA_CCR_PL;

    DMA2_Channel2->CCR |= DMA_CCR_EN;
}

int __io_putchar(int c) {
    while(!(USART5->ISR & USART_ISR_TXE));

    USART5->TDR = c;
    return c;
}

void write(char* buffer, int length) {
	for (int i = 0; i < length; i++) {
		putchar(buffer[i]);
	}

	return;
}

void USART3_4_5_6_7_8_IRQHandler(void) {
	seroffset = (seroffset + 1) % sizeof serfifo;

    const char deliminator[] = "\r\n";
    const int deliminator_len = (sizeof(deliminator) / sizeof(deliminator[0])) - 1;

	static int buffer_offset = 0;
	static int deliminator_offset = 0;

	static int read_offset = 0;
	static char input_buffer[FIFOSIZE];
	char ch = serfifo[read_offset];
	input_buffer[buffer_offset++] = ch;

	read_offset = (read_offset + 1) % sizeof serfifo;

	if (ch == deliminator[deliminator_offset++]) {
		if (deliminator_offset == deliminator_len) {
			// get credentials
			if (input_buffer[0] == 'G' && input_buffer[1] == 'C') {
				char* creds_buffer = (char*)malloc(128);
				int creds_buffer_len = get_wifi_credentials_ptr(creds_buffer);

				uint16_t index = 0;
				while (creds_buffer[index] != '\0') {
					putchar(creds_buffer[index]);
					index++;
				}
				if (index == 0) {
					char delim[] = "none";
					write(delim, sizeof(delim) - 1);
				}
				char delim[] = "\r\n";
				write(delim, sizeof(delim) - 1);
				free(creds_buffer);
			}
			// save credentials
			if (input_buffer[0] == 'S' && input_buffer[1] == 'C') {
				uint16_t index = 2;
				while (input_buffer[index] != '\r' && input_buffer[index + 1] != '\n') {
					index++;
				}
				save_wifi_credentials_ptr(input_buffer + 2, index - 2);
			}
			// set unlock code
			if (input_buffer[0] == 'S' && input_buffer[1] == 'U') {
				set_password(input_buffer + 2);
			}

			if (input_buffer[0] == 'U' && input_buffer[1] == 'D') {
				check_code_func_ptr(input_buffer + 2);
			}


			buffer_offset = 0;
			deliminator_offset = 0;
			for (int i = 0; i < FIFOSIZE; i++) {
				input_buffer[i] = '\0';
			}
//			check_code_func_ptr(input_buffer);
		}
	} else {
		deliminator_offset = 0;
	}
}

void send_init_message() {
    char init[] = "init\r\n";
//    while (1)  {
	write(init, sizeof(init) - 1);
//    	int count = 0;
//    	while (count++ < 500);
//    }

}

// screen /dev/tty.usbserial-A50285BI 115200
void init_wifi_uart() {
    init_usart5();
    enable_tty_interrupt();

	send_init_message();
}

void send_door_open_notification() {
	// SN: send notification
	char sn[] = "SN\r\n";
	write(sn, sizeof(sn) - 1);
//	volatile int x = 0;
}
