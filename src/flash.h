#ifndef FLASH_H_
#define FLASH_H_

void erase_flash_page(uint32_t page_address);
void set_default_password(char * default_password_memory_location);
void store_wifi_credentials_in_flash(const char* wifi_creds, uint16_t wifi_creds_length);
void store_password_from_admin_in_flash(const char* passcode);
//void delete_password_from_flash(const char* passcode);
void delete_password_from_flash();
uint8_t get_wifi_credentials_from_flash(char* buffer);
void set_password(char* password);
#endif
