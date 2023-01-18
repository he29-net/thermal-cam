/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef _MLX90640_I2C_Driver_H_
#define _MLX90640_I2C_Driver_H_
//#ifdef __cplusplus
// extern "C" {
//#endif
#include <stdint.h>
#include "bsp.h"
	 
//IO方向设置
#define SDA_IN()  {GPIO_CTL1(GPIOA)&=0x0FFFFFFF;GPIO_CTL1(GPIOA)|=8<<28;}
#define SDA_OUT() {GPIO_CTL1(GPIOA)&=0x0FFFFFFF;GPIO_CTL1(GPIOA)|=3<<28;}

//IO操作函数	
/*********************************** IIC_SCL ******************************/
#define IIC_SCL_PIN               GPIO_PIN_9
#define IIC_SCL_PORT              GPIOA
#define IIC_SCL_LOW()             gpio_bit_reset(IIC_SCL_PORT, IIC_SCL_PIN)
#define IIC_SCL_HIGH()            gpio_bit_set(IIC_SCL_PORT, IIC_SCL_PIN)
/*********************************** IIC_SDA ******************************/
#define IIC_SDA_PIN               GPIO_PIN_15
#define IIC_SDA_PORT              GPIOA
#define IIC_SDA_LOW()             gpio_bit_reset(IIC_SDA_PORT, IIC_SDA_PIN)
#define IIC_SDA_HIGH()            gpio_bit_set(IIC_SDA_PORT, IIC_SDA_PIN)

#define IIC_SDA_READ()            gpio_input_bit_get(IIC_SDA_PORT, IIC_SDA_PIN)


void MLX90640_I2CInit(void);
int  MLX90640_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data);
int  MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data);
void MLX90640_I2CFreqSet(int freq);
//#ifdef __cplusplus
//}
//#endif
#endif
