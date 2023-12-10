#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI
#include "constants.h"
#include "utils.h"

char* WIFI_CREDENTIALS_MEM_LOCATION = (char*) 0x0803F000;
char* PASSWORD_MEMORY_LOCATION = (char*) 0x0800F000;

void set_password(char* password) {
	erase_flash_page((uint32_t)PASSWORD_MEMORY_LOCATION);

    // Unlock the Flash memory
    if ((FLASH->CR & FLASH_CR_LOCK) != 0)
    {
        FLASH->KEYR = FLASH_FKEY1;
        FLASH->KEYR = FLASH_FKEY2;
    }

    uint32_t flash_addr = (uint32_t) PASSWORD_MEMORY_LOCATION;

    for (int i = 0; i < 4; i++)
    {
        uint16_t data = password[i];  // convert char to uint16_t
        // Use the flash programming sequence to write to it.
        FLASH->CR |= FLASH_CR_PG;
        *(__IO uint16_t*)(flash_addr) = data;

        if ((FLASH->SR & FLASH_SR_EOP) != 0)
        {
            FLASH->SR |= FLASH_SR_EOP;
        }

        FLASH->CR &= ~FLASH_CR_PG;
        flash_addr += 2;  // Move to next address location
    }

    // Locking the Flash control register access
    FLASH->CR |= FLASH_CR_LOCK;
}

void erase_flash_page(uint32_t page_address) {

    // Unlock the Flash memory
    if ((FLASH->CR & FLASH_CR_LOCK) != 0)
    {
        FLASH->KEYR = FLASH_FKEY1;
        FLASH->KEYR = FLASH_FKEY2;
    }

    // Erase the desired flash page
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = page_address;
    FLASH->CR |= FLASH_CR_STRT;

    if ((FLASH->SR & FLASH_SR_EOP) != 0)
    {
        FLASH->SR |= FLASH_SR_EOP;
    }

    FLASH->CR &= ~FLASH_CR_PER;

    // Locking the Flash control register access
    FLASH->CR |= FLASH_CR_LOCK;
}


// set default password to 1234
void set_default_password(char * default_password_memory_location) {

	erase_flash_page((uint32_t)default_password_memory_location);

    // Unlock the Flash memory
    if ((FLASH->CR & FLASH_CR_LOCK) != 0)
    {
        FLASH->KEYR = FLASH_FKEY1;
        FLASH->KEYR = FLASH_FKEY2;
    }

    uint32_t flash_addr = (uint32_t) default_password_memory_location;

    // Note: Assuming password is always 4 characters
    char default_password[4] = {'1', '2', '3', '4'};

    for (int i = 0; i < 4; i++)
    {
        uint16_t data = default_password[i];  // convert char to uint16_t
        // Use the flash programming sequence to write to it.
        FLASH->CR |= FLASH_CR_PG;
        *(__IO uint16_t*)(flash_addr) = data;

        if ((FLASH->SR & FLASH_SR_EOP) != 0)
        {
            FLASH->SR |= FLASH_SR_EOP;
        }

        FLASH->CR &= ~FLASH_CR_PG;
        flash_addr += 2;  // Move to next address location
    }

    // Locking the Flash control register access
    FLASH->CR |= FLASH_CR_LOCK;
}

// returns length of array
uint8_t get_wifi_credentials_from_flash(char* buffer) {
	char* wifi_credentials_memory_location = WIFI_CREDENTIALS_MEM_LOCATION;

	uint8_t length = wifi_credentials_memory_location[0];
	for (int i = 0; i < length; i++) {
		buffer[i] = wifi_credentials_memory_location[2 + i * 2];
	}
	buffer[length] = '\0';

	return length;
}


// ssid has max length of 32
// password has max length of 63 characters
void store_wifi_credentials_in_flash(const char* wifi_creds, uint16_t wifi_creds_length) {
	// clear flash memory in storage location
	char *wifi_credentials_memory_location = WIFI_CREDENTIALS_MEM_LOCATION;
	erase_flash_page((uint32_t) wifi_credentials_memory_location);

	// Unlock the Flash memory
	if ((FLASH->CR & FLASH_CR_LOCK) != 0)
	{
		FLASH->KEYR = FLASH_FKEY1;
		FLASH->KEYR = FLASH_FKEY2;
	}

	uint32_t flash_addr = (uint32_t) wifi_credentials_memory_location;

	// store wifi_creds_length as first 2 bytes
	FLASH->CR |= FLASH_CR_PG;
	*(__IO uint16_t*)(flash_addr) = wifi_creds_length;
	if ((FLASH->SR & FLASH_SR_EOP) != 0)
	{
		FLASH->SR |= FLASH_SR_EOP;
	}
	FLASH->CR &= ~FLASH_CR_PG;
	flash_addr += 2;

	// store each char of wifi_creds
	for (int i = 0; i < wifi_creds_length; i++)
	{
		uint8_t data = wifi_creds[i];  // convert char to uint16_t
		// Use the flash programming sequence to write to it.
		FLASH->CR |= FLASH_CR_PG;
		*(__IO uint16_t*)(flash_addr) = data;

		if ((FLASH->SR & FLASH_SR_EOP) != 0)
		{
			FLASH->SR |= FLASH_SR_EOP;
		}

		FLASH->CR &= ~FLASH_CR_PG;
		flash_addr += 2;  // Move to next address location
	}

//	for (int i = 0; i < wifi_password_length; i++)
//		{
//			uint16_t data = wifi_password[i];  // convert char to uint16_t
//			// Use the flash programming sequence to write to it.
//			FLASH->CR |= FLASH_CR_PG;
//			*(__IO uint16_t*)(flash_addr) = data;
//
//			if ((FLASH->SR & FLASH_SR_EOP) != 0)
//			{
//				FLASH->SR |= FLASH_SR_EOP;
//			}
//
//			FLASH->CR &= ~FLASH_CR_PG;
//			flash_addr += 2;  // Move to next address location
//		}

	// Locking the Flash control register access
	FLASH->CR |= FLASH_CR_LOCK;
}




// passcode length is always 4 chars
// can currently store 10 passcodes at once
// add in a new password every 2k bytes (1 page)
// character is two bytes (16 bits)
// 1 password per page
void store_password_from_admin_in_flash(const char* passcode){
	// 0x0801F000
	char *passwords_memory = (char *) 0x0801F000;

	uint32_t flash_addr = (uint32_t) passwords_memory;

//	// need to check for an open spot to store passcode
//	int count = 0;
//	while(count < 10){
//		// if 8 zero bytes are found in a row, then break
//		uint64_t data = *(__IO uint64_t*)(flash_addr);
//		if(data == 0xFFFFFFFFFFFFFFFF){
//			break;
//		}
//		// else increment
//		count += 1;
//		// increment by 2k bytes
//		flash_addr += 2048;
//	}

	// Unlock the Flash memory
	if ((FLASH->CR & FLASH_CR_LOCK) != 0)
	{
		FLASH->KEYR = FLASH_FKEY1;
		FLASH->KEYR = FLASH_FKEY2;
	}

	// at this point you have found the flash memory to use
    for (int i = 0; i < 4; i++)
    {
        uint16_t data = passcode[i];  // convert char to uint16_t
        // Use the flash programming sequence to write to it.
        FLASH->CR |= FLASH_CR_PG;
        *(__IO uint16_t*)(flash_addr) = data;

        if ((FLASH->SR & FLASH_SR_EOP) != 0)
        {
            FLASH->SR |= FLASH_SR_EOP;
        }

        FLASH->CR &= ~FLASH_CR_PG;
        flash_addr += 2;  // Move to next address location
    }


}


// delete password function
//void delete_password_from_flash(const char* passcode)
void delete_password_from_flash(){
	char *passwords_memory = (char *) 0x0801F000;
	uint32_t flash_addr = (uint32_t) passwords_memory;

//	// check 64 bit increments
//	int count = 0;
//	while(count < 10){
//		int found = 1;
//	    for (int i = 0; i < 4; i++)
//	    {
//	    	char * data_addr = (char *)flash_addr;
//	    	if(data_addr[i*2] != passcode[i]){
//	    		found = 0;
//	    	}
//	    }
//	    if(found == 1){
//	    	break;
//	    }
//
//		// else increment
//		count += 1;
//		// increment by 20 bytes
//		flash_addr += 2048;
//	}

	//password found
	erase_flash_page((uint32_t) flash_addr);

}
