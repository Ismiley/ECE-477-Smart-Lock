#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for MA_PI
#include "constants.h"
#include "utils.h"

#define VCNL4010_I2C_ADDRESS 0x13  // 7-bit address
#define VCNL4010_COMMAND_REG        0x80
#define VCNL4010_PROXIMITY_RATE_REG 0x82
#define VCNL4010_PROXIMITY_DATA_REG 0x87 // proximity-result register

#define VCNL4010_PRODUCTID 0x81        ///< Product ID Revision

#define PROX_DEBOUNCE_COUNT 5

// Proximity Sensor Code
void init_i2c(void) {
    // Enable the clock for the GPIOB peripheral
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // Configure pins PB6 and PB7 as Alternate Function mode
    GPIOB->MODER &= ~(0xf000);    // Reset the configuration of pins PB6 and PB7 (clearing bits)
    GPIOB->MODER |= 0xa000;      // Set pins PB6 and PB7 to Alternate Function mode (0b10 for each pin)

    // Set the alternate function of PB6 and PB7 to be I2C1_SCL and I2C1_SDA respectively
    GPIOB->AFR[0] |= 1<<(4*6) | 1<<(4*7);

//    // Assuming SDA is on PB7 and SCL is on PB6
//    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7); // Clear bits
//    GPIOB->PUPDR |= (1 << (6 * 2)) | (1 << (7 * 2)); // Set pull-up for PB6 and PB7


    // Enable the clock for I2C1 peripheral
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // Disable the I2C1 peripheral (set it in a disabled state to configure it)
    I2C1->CR1 &= ~I2C_CR1_PE;

    // Disable analog noise filter
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;

    // Disable the error interrupt (this setup is not using interrupt mode)
    I2C1->CR1 &= ~I2C_CR1_ERRIE;

    // Disable clock stretching
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;

    // Setup I2C timing register (configure timing for I2C communication)
    I2C1->TIMINGR = 0;                    // Clear the timing register
    I2C1->TIMINGR &= ~I2C_TIMINGR_PRESC;  // Clear the prescaler bits
    I2C1->TIMINGR |= 0 << 28;             // Prescaler value
    I2C1->TIMINGR |= 3 << 20;             // Data setup time
    I2C1->TIMINGR |= 1 << 16;             // Data hold time
    I2C1->TIMINGR |= 3 << 8;              // SCL high period (master mode)
    I2C1->TIMINGR |= 9 << 0;              // SCL low period (master mode)

    // Disable own address 1 and own address 2 (not using address match features)
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR2 &= ~I2C_OAR2_OA2EN;

    // Set to 7-bit addressing mode (most common mode for I2C devices)
    I2C1->CR2 &= ~I2C_CR2_ADD10;

    // Enable automatic end mode (after data is sent/received, an automatic STOP is sent without software intervention)
    I2C1->CR2 |= I2C_CR2_AUTOEND;

    // Enable the I2C1 peripheral (ready for operation)
    I2C1->CR1 |= I2C_CR1_PE;

}

void i2c_waitidle(void) {
    while ( (I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
}

void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir) {
    uint32_t tmpreg = I2C1->CR2;
    tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES |
                I2C_CR2_RELOAD | I2C_CR2_AUTOEND |
                I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);
    if (dir == 1)
        tmpreg |= I2C_CR2_RD_WRN;
    else
        tmpreg &= ~I2C_CR2_RD_WRN;
    tmpreg |= ((devaddr<<1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);
    tmpreg |= I2C_CR2_START;
    I2C1->CR2 = tmpreg;
}

void i2c_stop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        return;

    I2C1->CR2 |= I2C_CR2_STOP;

    while( (I2C1->ISR & I2C_ISR_STOPF) == 0);
    I2C1->ICR |= I2C_ICR_STOPCF;
}

int i2c_checknack(void) {
    return (int)((1<<4) & (I2C1->ISR));
}

void i2c_clearnack(void) {
    I2C1->ICR |= 1<<4;
}

int i2c_senddata(uint8_t devaddr, const void *data, uint8_t size) {
    int i;
    if (size <= 0 || data == 0) return -1;
    uint8_t *udata = (uint8_t*)data;
    i2c_waitidle();

    i2c_start(devaddr, size, 0);

    for(i=0; i<size; i++) {
        int count = 0;
        while( (I2C1->ISR & I2C_ISR_TXIS) == 0) {
            count +=1;
            if (count > 1000000) return -1;
            if (i2c_checknack()) { i2c_clearnack(); i2c_stop(); return -1; }
        }
        I2C1->TXDR = udata[i] & I2C_TXDR_TXDATA;
    }

    while((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    if ( (I2C1->ISR & I2C_ISR_NACKF) != 0)
        return -1;
    i2c_stop();
    return 0;
}

int i2c_recvdata(uint8_t devaddr, void *data, uint8_t size) {
    int i;
    if (size <= 0 || data == 0) return -1;
    uint8_t *udata = (uint8_t*)data;
    i2c_waitidle();

    i2c_start(devaddr, size, 1);

    for(i=0; i<size; i++){

        int count = 0;
        while( (I2C1->ISR & I2C_ISR_RXNE) == 0) {
            count += 1;
            if (count > 1000000) return -1;
            if (i2c_checknack()) {i2c_clearnack(); i2c_stop(); return -1; }
        }
        udata[i] = I2C1->RXDR;
    }

    while((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);
    if ( (I2C1->ISR & I2C_ISR_NACKF) != 0)
        return -1;
    i2c_stop();
    return 0;
}

void VCNL4010_Init(void)
{
    uint8_t commandData[2];

    // Set proximity measurement rate
    commandData[0] = VCNL4010_PROXIMITY_RATE_REG;
    commandData[1] = 0x02;  // Sample value for proximity rate setting
    i2c_senddata(VCNL4010_I2C_ADDRESS, commandData, 2);

    // Start continuous proximity measurement --> PROX_EN and SELFTIMED_EN bits
    commandData[0] = VCNL4010_COMMAND_REG;
    commandData[1] = 0x03;
    i2c_senddata(VCNL4010_I2C_ADDRESS, commandData, 2);

}

uint16_t VCNL4010_ReadProximity(void)
{
    uint8_t regAddr = VCNL4010_PROXIMITY_DATA_REG;
    i2c_senddata(VCNL4010_I2C_ADDRESS, &regAddr, 1);  // Send register address first

    // less than 5000 == open
    uint8_t buf[2];
    i2c_recvdata(VCNL4010_I2C_ADDRESS, buf, 2);  // Then read the data
    return (buf[0] << 8) | buf[1];
}

volatile int timerFlag = 0;

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF) // Check update interrupt flag
    {
//      timerFlag = 1;
        TIM3->SR &= ~TIM_SR_UIF; // Clear interrupt flag
        uint16_t proximity_value = VCNL4010_ReadProximity();
        static uint8_t prox_below_threshold_count = 0;

		if(proximity_value < 3000){
			prox_below_threshold_count++;

			if(prox_below_threshold_count >= PROX_DEBOUNCE_COUNT){
				// send notification here
				prox_below_threshold_count = 0;  // Reset after notification to avoid continuous notifications
			}
		}
		else{
			prox_below_threshold_count = 0;  // Reset counter if proximity goes above 3000
		}
    }
}

int process_prox(){
	uint8_t prox_below_threshold_count = 0;
	for(int i = 0; i < 4; i++){
		uint16_t proximity_value = VCNL4010_ReadProximity();
		if(proximity_value < 3000){
			prox_below_threshold_count++;

			if(prox_below_threshold_count >= 4){
				// door is open
				return 0;
			}
		}
	}
	return 1;
}

void Timer3_Init(void)
{
    // Enable clock for TIM3
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Configure the timer for 5ms interval assuming 48MHz system clock
    // Prescaler = 480
    TIM3->PSC = 479; // Set prescaler to 479 (PSC register value is PSC - 1)

    // Auto-reload value for 5ms
    // Reload value = (Interval * Timer Clock) / Prescaler
    // For 100ms with 48MHz and prescaler 480, it's 500.
    TIM3->ARR = 2000-1; // Auto-reload is set to 999 (ARR value is ARR - 1)

    // Enable update interrupt
    TIM3->DIER |= TIM_DIER_UIE;

    // Start the timer
    TIM3->CR1 |= TIM_CR1_CEN;
}
