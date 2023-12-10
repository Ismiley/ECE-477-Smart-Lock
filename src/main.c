#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI
#include "wifi_uart.h"
#include "keypad.h"
#include "constants.h"
#include "utils.h"
#include "motor.h"
#include "prox.h"
#include "flash.h"

extern char password_buffer[PASSWORD_LENGTH];

// memory location in flash memory: 0x0800F000
char *default_password_memory_location = (char *) 0x0800F000;
int isDoorOpen;

void unlock(){
	// unlock lock
	setMotorDirection(1);
	motorON();
	// 9 seconds
	for(int i = 0; i <= 150; i++){
		nano_wait(50000000);
	}
	motorOFF();

	// wait
	// 9 seconds
	for(int i = 0; i <= 150; i++){
		nano_wait(50000000);
	}

	// check 4 times if
	int relock = 0;
	for(int i = 0; i <= 4; i++){
		relock = process_prox();
	}

	// lock if door closed
	if(relock == 1){
		setMotorDirection(0);
		motorON();
		// 9 seconds
		for(int i = 0; i <= 150; i++){
			nano_wait(50000000);
		}
		motorOFF();
	}
	else{
		// send door open notification
	}

	return;
}


int check_if_code_is_valid(char* password_buffer) {
    if(compare_passcode_values(password_buffer, default_password_memory_location)) {
        // Password matched
    	unlock();
    	return 1;
    } else {
        // Password did not match
    	return 0;
    }
}

// params: take in two memory addresses
// return: 1 if equivalent, 0 if not
// later extend to check multiple password locations (multiple stored)
int compare_passcode_values(char *buffer_entry, char *buffer_password){
    for(int i = 0; i < PASSWORD_LENGTH; i++){
        if(buffer_entry[i] != buffer_password[i*2]){
            return 0;
        }
    }
    return 1;
}


void all_initializations(){

    // Set priority for TIM6_DAC_IRQn interrupt (keypad)
    NVIC_SetPriority(TIM6_DAC_IRQn, 1); // Highest priority

	// Prox
	init_i2c();
	VCNL4010_Init();

	// Keypad
	enable_ports_keypad();
	init_tim6(); // keyboard

	// Motor
	initializeMotorDriver();

	// Password buffer
	memset(password_buffer, 0, PASSWORD_LENGTH);

	NVIC_EnableIRQ(TIM6_DAC_IRQn);
}


int main(void)
{
//	all_initializations();
//	// Keypad
//	init_tim6(); // keyboard
//	enable_ports_keypad();
	test_motor();
////	while (1) {
//	init_wifi_uart();
////	}
////	init_wifi_uart();
//	set_wifi_code_check_func_ptr(check_if_code_is_valid);
//	set_save_wifi_credentials_ptr(store_wifi_credentials_in_flash);
//	set_get_wifi_credentials_ptr(get_wifi_credentials_from_flash);
	set_keypad_code_check_func_ptr(check_if_code_is_valid);

	isDoorOpen = 0;

	for(;;){
		check_keypress();
		int prox_val = process_prox();
		if (prox_val == 0) {
			if (isDoorOpen) {
				continue;
			}

			int count = 0;
			while (process_prox() == 0) {
				count++;
				if (count > 2500) {
					isDoorOpen = 1;
					send_door_open_notification();
					break;
				}
			}
		} else {
			isDoorOpen = 0;
		}

		// for 15 seconds:
		// trial 1: 26535
		// trial 2: 26544
		// trial 3: 25892

	}

	// erase flash
//	delete_password_from_flash();
//	char test_passcode[4] = {'1', '2', '3', '4'};
//	store_password_from_admin_in_flash(test_passcode);

//	// add passcode
//	char test_passcode[4] = {'4', '5', '6', '1'};
//	delete_password_from_flash(test_passcode);
////	store_password_from_admin_in_flash(test_passcode);
}
