#ifndef KEYPAD_H_
#define KEYPAD_H_

void enable_ports_keypad(void);

void drive_column(int c);

int read_rows();

void push_queue(int n);

uint8_t pop_queue();

void update_history(int c, int rows);

void init_tim6();

void TIM6_DAC_IRQHandler(void);

void process_keypress(char key);

void set_keypad_code_check_func_ptr(void (*func_ptr)(char*));

void check_keypress();

#endif
