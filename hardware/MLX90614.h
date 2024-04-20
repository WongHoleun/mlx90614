/*
 * @Description: 红外温度传感器MLX90614驱动模块
 * @Author: Haolin Wang
 * @Date: 2023-03-20 15:56:05
 * @LastEditTime: 2023-03-26 15:59:04
 * @Note:
 */

#ifndef __MLX90614_H_
#define __MLX90614_H_

/* 传感器地址 */
#define MLX90614_ADDRESS ((0x00) << 1)
// #define MLX90614_ADDRESS ((0x5A)<<1)
#define MLX90614_ADDRESS_R ((0x5A)<<1) | 0x01))	//地址 读

/* 传感器操作码 */
#define MLX90614_OP_RAM 0x00	// 访问RAM
#define MLX90614_OP_EEPROM 0x20 // 访问EEPROM
#define MLX90614_OP_SLEEP 0xFF	// 进入睡眠模式

/* RAM 寄存器地址 */
#define MLX90614_RAW1 (MLX90614_OP_RAM | 0x04)	// 存放 IR（红外）通道1的原始数据
#define MLX90614_RAW2 (MLX90614_OP_RAM | 0x05)	// 存放 IR（红外）通道2的原始数据
#define MLX90614_TA (MLX90614_OP_RAM | 0x06)	// 存放 环境温度
#define MLX90614_TOBJ1 (MLX90614_OP_RAM | 0x07) // 存放 物体温度（数据源于IR1）
#define MLX90614_TOBJ2 (MLX90614_OP_RAM | 0x08) // 存放 物体温度（数据源于IR2）

/* EEPROM 寄存器地址 */
#define MLX90614_TOMAX (MLX90614_OP_EEPROM | 0x00)		// 存放 物体温度上限
#define MLX90614_TOMIN (MLX90614_OP_EEPROM | 0x01)		// 存放 物体温度下限
#define MLX90614_PWMCTRL (MLX90614_OP_EEPROM | 0x02)	// PWM输出使能 控制寄存器
#define MLX90614_TARANGE (MLX90614_OP_EEPROM | 0x03)	// 存放 环境温度范围
#define MLX90614_EMISSIVITY (MLX90614_OP_EEPROM | 0x04) // 存放 发射率校准系数
#define MLX90614_CFG1 (MLX90614_OP_EEPROM | 0x05)		// 存放 寄存器1配置
#define MLX90614_ADDR (MLX90614_OP_EEPROM | 0x0E)		// 存放 SMBus 设备地址

/* 设备工作状态 */
typedef enum
{
	MLX90614_OK = 0,
	MLX90614_ERROR = 1
} MLX90614_StatusTypeDef;

/* 接口函数声明 */
MLX90614_StatusTypeDef MLX90614_RAM_Read(uint8_t Register, uint16_t *Result);	// 读取 RAM
MLX90614_StatusTypeDef MLX90614_EEPROM_Read(uint8_t Address, uint16_t *Result); // 读取 EEPROM
uint8_t CRC_8(uint8_t IntCrc, uint8_t Data);									// CRC-8 校验
MLX90614_StatusTypeDef MLX90614_Write_Data(uint8_t Address, uint16_t Value);	// 生成写入 EEPROM 的数据流（LSB、MSB、PEC）
MLX90614_StatusTypeDef MLX90614_Write_EEPROM(uint8_t Address, uint16_t Value);	// 写入 EEPROM
int MLX90614_Kelvin_Celsius(uint16_t Temperature);							// 温度处理（开尔文转换摄氏度）
int Mlx90614_measure_temperature(void);										// 读取 物体温度
MLX90614_StatusTypeDef MLX90614_Emissivity_Set(float Emissivity);				// 修改 发射率
MLX90614_StatusTypeDef MLX90614_SMBusAddress_Set(uint8_t Address);				// 修改 设备SMBus地址
MLX90614_StatusTypeDef MLX90614_Init(I2C_HandleTypeDef *hi2c);					// 传感器 初始化
#endif																			/* MLX90614_H_ */
