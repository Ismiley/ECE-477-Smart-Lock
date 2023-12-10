#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI
#include "constants.h"
#include "utils.h"

void (*check_code_func_ptr)(char*);
void set_keypad_code_check_func_ptr(void (*func_ptr)(char*)) {
	check_code_func_ptr = func_ptr;
	return;
}

// const char keymap[] = "DCBA#9630852*741";
//const char keymap[] = "123A456B789C*0#D";
const char keymap[] = "147*2580369#ABCD";
uint8_t hist[16];
uint8_t col;
char queue[1];
int qin;
int qout;
// Buffer to store the entered password
char password_buffer[PASSWORD_LENGTH];



// ..........................................................................
// Configuring GPIO for Keypad
//===========================================================================

// pins: PA4567 PB 012 10

void enable_ports_keypad(void) {
    // Enable I/O for Port A and Port B
    RCC->AHBENR |= (1 << 17); // Enable clock for GPIOA
    RCC->AHBENR |= (1 << 18); // Enable clock for GPIOB

    GPIOA->MODER &= ~(0xFF00);   // Clear mode bits for PA4-PA7
//    GPIOA->MODER |= 0x5500;      // Set PA4-PA7 as output (01)

    GPIOB->MODER &= ~(0x3F);     // Clear mode bits for PB0-PB2
    GPIOB->MODER |= 0x15;        // Set PB0-PB2 as output (01)
    GPIOB->MODER &= ~(0x300000);  // Clear mode bits for PB10
    GPIOB->MODER |= 0x100000;     // Set PB10 as output (01)

    GPIOA->PUPDR &= ~(0xFF00);   // Clear pull-up/pull-down bits for PA4-PA7
//    GPIOA->PUPDR |= 0xAA00;      // Set PA4-PA7 to pull-down (10)
    GPIOA->PUPDR |= 0x5500; // for pull up
}

void drive_column(int c) {

	if(c == 3){
		c = 10;
	}
	GPIOB->BSRR = 0x407;
	GPIOB->BRR = 1 << c;
	return;
}

int read_rows() {
    int row_val = 0;

    // Read the state of the row pins.
    row_val |= ((GPIOA->IDR & (1 << 4)) ? 1 : 0) << 0;  // Read PA4
    row_val |= ((GPIOA->IDR & (1 << 5)) ? 1 : 0) << 1;  // Read PA5
    row_val |= ((GPIOA->IDR & (1 << 6)) ? 1 : 0) << 2;  // Read PA6
    row_val |= ((GPIOA->IDR & (1 << 7)) ? 1 : 0) << 3;  // Read PA7

    return row_val;
}

void push_queue(int n) {
    n = (n & 0xff) | 0x80;
    queue[qin] = n;
    qin = 0;
}

uint8_t pop_queue() {
    uint8_t tmp = queue[qout] & 0x7f;
    queue[qout] = 0;
    qout = 0;
    return tmp;
}

void update_history(int c, int rows) {
    for(int i = 0; i < 4; i++) {
        hist[4*c+i] = (hist[4*c+i]<<1) + ((rows>>i)&1);
        if (hist[4*c+i] == 1)
          push_queue(4*c+i);
    }
}

void init_tim6() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 48 - 1;
    TIM6->ARR = 1000 - 1;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] |= 1 << TIM6_DAC_IRQn;
}

void TIM6_DAC_IRQHandler(void) {
	if (TIM6->SR & TIM_SR_UIF){
		TIM6->SR &= ~TIM_SR_UIF;
		int rows = read_rows();
		update_history(col, rows);
		col = (col + 1) & 3;
		drive_column(col);
	}
}

void process_keypress(char key) {
    static int count = 0; // static, to keep its value between calls

    if(key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0'){
		password_buffer[count] = key;
		count++;
	}
    if(key == 'A') {
        count = 0;
        memset(password_buffer, 0, PASSWORD_LENGTH);
    }

    if (count >= PASSWORD_LENGTH) {
    	check_code_func_ptr(password_buffer);
        count = 0;
        memset(password_buffer, 0, PASSWORD_LENGTH);
    }
}

void check_keypress() {
	if (queue[qout] > 0)
		process_keypress(keymap[pop_queue()]);
}


