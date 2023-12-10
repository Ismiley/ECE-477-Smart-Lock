#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI
#include "constants.h"
#include "utils.h"

// Motor Code
void initializeMotorDriver(void) {
    // Enable the clock for GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Configure GPIOA 0, 1, and 2 as general outputs
    GPIOA->MODER |= (0b01 << (0*2)) | (0b01 << (1*2)) | (0b01 << (2*2));

    // Enable internal pull-up resistor for PA3
	GPIOA->PUPDR &= ~(0b11 << (6));
	GPIOA->PUPDR |= (0b01 << (6));

	GPIOA->BSRR = (1 << 3); //standby to high
}

void setMotorDirection(int direction) {
    if(direction == 0) {
        GPIOA->BSRR = (1 << 0);     // AIN1 = HIGH
        GPIOA->BRR = (1 << 1);      // AIN2 = LOW
    } else {
        GPIOA->BRR = (1 << 0);      // AIN1 = LOW
        GPIOA->BSRR = (1 << 1);     // AIN2 = HIGH
    }
}

void motorON() {
    GPIOA->BSRR = (1 << 2);         // PWMA = HIGH
}

void motorOFF() {
    GPIOA->BRR = (1 << 2);          // PWMA = LOW
}

void test_motor(){
	initializeMotorDriver();

	setMotorDirection(1);
	motorON();
	// 9 seconds
//	for(int i = 0; i <= 150; i++){
//		nano_wait(50000000);
//	}
	motorOFF();

//	for(int i = 0; i <= 150; i++){
//			nano_wait(50000000);
//	}

	setMotorDirection(0);
	motorON();
//	for(int i = 0; i <= 150; i++){
//			nano_wait(50000000);
//	}
	motorOFF();
}
