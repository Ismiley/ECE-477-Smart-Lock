#ifndef PROX_H_
#define PROX_H_

void init_i2c(void);
void i2c_waitidle(void);
void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir);
void i2c_stop(void);
int i2c_checknack(void);
void i2c_clearnack(void);
int i2c_senddata(uint8_t devaddr, const void *data, uint8_t size);
int i2c_recvdata(uint8_t devaddr, void *data, uint8_t size);

#define VCNL4010_I2C_ADDRESS  // 7-bit address
#define VCNL4010_COMMAND_REG
#define VCNL4010_PROXIMITY_RATE_REG
#define VCNL4010_PROXIMITY_DATA_REG // proximity-result register

#define VCNL4010_PRODUCTID    ///< Product ID Revision

#define PROX_DEBOUNCE_COUNT

void VCNL4010_Init(void);
uint16_t VCNL4010_ReadProximity(void);
void TIM3_IRQHandler(void);
int process_prox();
void Timer3_Init(void);

#endif
