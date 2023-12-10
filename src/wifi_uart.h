#ifndef WIFI_UART_H_
#define WIFI_UART_H_

void init_wifi_uart();

int check_if_code_is_valid();

void set_wifi_code_check_func_ptr(void (*func_ptr)(char*));

void set_save_wifi_credentials_ptr(void (*func_ptr)(const char*, uint16_t));

void set_get_wifi_credentials_ptr(uint8_t (*func_ptr)(char*));

void send_door_open_notification();

#endif
