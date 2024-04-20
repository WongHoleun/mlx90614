/**
****************************************************************************************
* @Description: 红外温度传感器MLX90614驱动模块
* @Author: Haolin Wang
* @Date: 2023-03-20 15:56:05
* @LastEditTime: 2023-03-25 18:00:59
* @Note:i2c.C文件调整 __HAL_RCC_I2C1_CLK_ENABLE() 在  __HAL_RCC_GPIOB_CLK_ENABLE() 前面
****************************************************************************************
*/

#include "i2c.h"
#include "math.h"
#include "MLX90614.h"

I2C_HandleTypeDef *mlx90614_i2c;

/**
****************************************************************************************
* @Funticon name: 读取 RAM（不能写）
* @Berif:
* @Note: 用户无法写入 RAM 内存，只能进行读取，并且只有有限的一部分 RAM 寄存器对客户有意义。
* MLX90614 的RAM有 32 个 17 位存储单元，其中TA,TOBJ1,TOBJ2是环境温度和物体温度,
* 在SMBus方式下，可以从这几个存储单元读出环境和被测物体的温度。
* @param {uint8_t} Register	RAM 寄存器地址
* @param {uint16_t} *Result	读取结果变量 的地址
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_RAM_Read(uint8_t Register, uint16_t *Result)
{
	uint8_t tmp[2];

	int res = HAL_I2C_Mem_Read(mlx90614_i2c, MLX90614_ADDRESS, Register, I2C_MEMADD_SIZE_8BIT, tmp, 2, 10); // 读取 RAM 寄存器内容（2字节） 并返回总线状态

	if (HAL_OK != res)
	{
		return MLX90614_ERROR; // 返回 错误通信标识
	}
	else
	{
		*Result = (uint16_t)tmp[1] << 8 | tmp[0]; // 取出寄存器内容 16bit数据
		return MLX90614_OK;						  // 返回 正常通信标识
	}
}

/**
****************************************************************************************
* @Funticon name: 读取 EEPROM (部分可写)
* @Berif: Briefly describe the function of your function
* @Note: 只有某些存储单元用户能够写入，但是可以读出全部存储单元。
* MLX90614 的 EEPROM 有 32 个 16 位存储单元，其中存储单元 Tomax,Tomin,Ta
* 分别是 用户物体温度上下限和环境温度范围，PWMCTRL 是 PWM 配置寄存器。
* @param {uint8_t} Register	EEPROM 寄存器地址
* @param {uint16_t} *Result	读取结果变量 的地址
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_EEPROM_Read(uint8_t Register, uint16_t *Result)
{
	uint8_t tmp[2];
	int res = HAL_I2C_Mem_Read(mlx90614_i2c, MLX90614_ADDRESS, Register, I2C_MEMADD_SIZE_8BIT, tmp, 2, 10); // 读取 EEPROM 寄存器内容(2字节) 并返回总线状态

	if (HAL_OK != res)
	{
		return MLX90614_ERROR; // 返回 错误通信标识
	}
	else
	{
		*Result = (uint16_t)tmp[1] << 8 | tmp[0]; // 取出寄存器内容 16bit数据
		return MLX90614_OK;						  // 返回 正常通信标识
	}
}

/**
****************************************************************************************
* @Funticon name: CRC-8校验
* @Berif: （循环计算法）
* @Note: 由于多项式的最高为都为1，并且在代码的crc8计算中，最高位也是不使用的，
* 所以在多项式记录时都去掉了最高位。多项式最高位为1，遇到需要异或数据最高位为1时，才进行异或计算，
* 并且异或后，最高位就为0了，最高位为0，下次也不需要异或了，就可以把最高位去掉。
* @param {uint8_t} IntCrc	CRC初值
* @param {uint8_t} Data	待检验数据
* @return {*}		CRC校验码 即PEC（具有多项式G(X) = X^8+X^2+X^1+1 的 CRC-8）
****************************************************************************************
*/
uint8_t CRC_8(uint8_t IntCrc, uint8_t Data)
{
	uint8_t i;
	uint8_t data;
	data = IntCrc ^= Data; // CRC校验码（上一字节数据校验结果）与需要计算的数据异或

	for (i = 0; i < 8; i++) // 8次判断、移位、或异或操作（1个字节）
	{
		if ((data & 0x80) != 0) // 判断数据最高位为1，需要异或
		{
			data <<= 1;	  // 左移一位：去掉数据最高位1（生成多项式最高位也是1，这一位不需要异或） 例如信息多项式 0x11 左移一位 1 0001 0000 (与左移8位后待处理数据一样)
			data ^= 0x07; // 与0x07异或。       多项式G(X) = X^8+X^2+X^1+1  转化为2进制去掉高位得 0x07      生成多项式0x107    1 0000 0111
		}
		else // 最高位为0，不需要异或
		{
			data <<= 1; // 数据左移 去掉高位的0
		}
	}
	return data; // 得到的除不尽的余数，变为我们要求的CRC校验码
}

/**
****************************************************************************************
* @Funticon name: 生成写入 EEPROM 的数据流（LSB、MSB、PEC）
* @Berif:
* @Note: 由于多项式的最高为都为1，并且在代码的crc8计算中，最高位也是不使用的，所以在多项式记录时都去掉了最高位。
* 多项式最高位为1，遇到需要异或数据最高位为1时，才进行异或计算，并且异或后，最高位就为0了，最高位为0，下次也不需要异或了，就可以把最高位去掉。
* @param {uint8_t} Address	寄存器地址
* @param {uint16_t} Value	写入数据内容（2字节）
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_Write_Data(uint8_t Address, uint16_t Value)
{
	uint8_t crc;
	uint8_t tmp[3];

	/*生成PEC 数据错误检查*/
	crc = CRC_8(0, MLX90614_ADDRESS); // 设备地址校验 初始crc校验码（设置算法开始时寄存器CRC的初始化预设值为0）
	crc = CRC_8(crc, Address);		  // 叠加寄存器地址校验
	crc = CRC_8(crc, Value & 0xFF);	  // 叠加低字节校验
	crc = CRC_8(crc, Value >> 8);	  // 叠加高字节校验

	tmp[0] = Value & 0xFF; // 写入LSB
	tmp[1] = Value >> 8;   // 写入MSB
	tmp[2] = crc;		   // 写入PEC

	int res = HAL_I2C_Mem_Write(mlx90614_i2c, MLX90614_ADDRESS, Address, 1, tmp, 3, 10); // 数据流(LSB MSB PEC)  写入EEPROM 寄存器 并返回总线状态

	if (res != HAL_OK)
	{
		return MLX90614_ERROR; // 返回 通信错误标识
	}
	else
	{
		return MLX90614_OK; // 返回 通信正常标识
	}
}

/**
****************************************************************************************
* @Funticon name: EEPROM 寄存器 写入
* @Berif: Briefly describe the function of your function
* @Note: 可以修改EEPROM 物体温度上下限和环境温度范围、发射系数、SMBus设备地址、PWM使能
* 写入前需要先擦除寄存器内容
* @param {uint8_t} Address		寄存器地址
* @param {uint16_t} Value		写入数据内容（2字节）
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_Write_EEPROM(uint8_t Address, uint16_t Value)
{

	uint16_t Verify;

	/*擦除 EEPROM寄存器 内容*/
	int res = MLX90614_Write_Data(Address, 0);
	if (MLX90614_OK != res) // 写入0 并判断
	{
		return MLX90614_ERROR; // 返回 通信错误标识
	}
	HAL_Delay(10); // 擦除时间

	/*修改 EEPROM寄存器 内容*/
	res = MLX90614_Write_Data(Address, Value);
	if (MLX90614_OK != res) // 像寄存器写入内容 并判断
	{
		return MLX90614_ERROR; // 返回 通信错误标识
	}
	HAL_Delay(10); // 写入时间

	/*检验 EEPROM寄存器 内容*/
	MLX90614_EEPROM_Read(Address, &Verify); // 读取寄存器值 检验是否存储正确
	if (Verify != Value)
	{
		return MLX90614_ERROR; // 返回 通信错误标识
	}

	return MLX90614_OK; // 返回 通信正确标识
}

/**
****************************************************************************************
* @Funticon name: 温度处理（开尔文转换摄氏度）
* @Berif:
* @Note:输出结果需要进行温度补偿，消除电压对温度影响，出厂校准电压为 3 V，我们使用的供电电压 3.3V
*		环境和物体温度对 VDD 的典型依赖性为 0.6°C/V
* @param {uint16_t} Temperature	开尔文温度
* @return {*}		摄氏度温度值（浮点型）
****************************************************************************************
*/
//float MLX90614_Kelvin_Celsius(uint16_t Temperature)
//{
//	float Result;
//
//	Result = (float)Temperature * 0.02;
//	Result = Result - 273.15;
//	Result = Result - ((3.3 - 3) * 0.6);	// 温度补偿
//
//	return Result;
//}

int MLX90614_Kelvin_Celsius(uint16_t Temperature)
{
	int Result;

	Result = Temperature * 2;
	Result = Result - 27315;
	Result = Result - 18;	// 温度补偿

	return Result;
}

/**
****************************************************************************************
* @Funticon name: 读取物体温度
* @Berif: Briefly describe the function of your function
* @Note: Need note condition
* @return {*} 返回温度值
****************************************************************************************
*/
int Mlx90614_measure_temperature(void)
{
	uint16_t tmp;

	int res = MLX90614_RAM_Read(MLX90614_TOBJ1, &tmp); // 读取 RAM 物体温度 To1 寄存器
	if (MLX90614_OK != res)
	{
		return MLX90614_ERROR;
	}
	else
	{
		return (MLX90614_Kelvin_Celsius(tmp)); // 输出摄氏温度
	}
}

/**
****************************************************************************************
* @Funticon name: 修改发射率
* @Berif:
* @Note: 修改前需要进行擦除
* @param {float} Emissivity	新发射率
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_Emissivity_Set(float Emissivity)
{
	uint16_t current_emissivity;
	uint16_t new_emissivity;

	if (Emissivity < 0.1 || Emissivity > 1) // 判断发射率是否在有效范围
	{
		return MLX90614_ERROR;
	}

	int res = MLX90614_EEPROM_Read(MLX90614_EMISSIVITY, &current_emissivity); // 读取 发射率寄存器 内容
	if (MLX90614_OK != res)
	{
		return MLX90614_ERROR;
	}

	new_emissivity = (uint16_t)round((65535.0 * Emissivity - 1)); // 四舍五入 发射率值

	if (new_emissivity == current_emissivity) // 判断 新旧发射率是否一致 一致则不需要修改
	{
		return MLX90614_OK;
	}

	res = MLX90614_Write_EEPROM(MLX90614_EMISSIVITY, new_emissivity); // 写入 新的发射率 返回总线状态
	if (MLX90614_OK != res)
	{
		return MLX90614_ERROR;
	}

	return MLX90614_OK;
}

/**
****************************************************************************************
* @Funticon name: 传感器地址修改
* @Berif: 用于多传感器地址分配，单个设备地址为0x00地址，
* 修改新地址需用此地址搜索设备多传感器时，默认地址为0x5A
* @Note:
* @param {uint8_t} Address	新的设备地址
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_SMBusAddress_Set(uint8_t Address)
{
	if (Address > 0x7F)
	{
		return MLX90614_ERROR;
	}
	int res = MLX90614_Write_EEPROM(MLX90614_ADDR, Address); // 写入 新的设备SMBus地址 返回总线状态
	if (MLX90614_OK != res)
	{
		return MLX90614_ERROR;
	}
	else
	{
		return MLX90614_OK;
	}
}

/**
****************************************************************************************
* @Funticon name: 传感器初始化（设置传感器所用i2c实例）
* @Berif: 在功能执行之前完成初始化
* @Note: 发射率-物体辐射量与黑体辐射最之比。系数介于0 和1 之间。人体皮肤一般为0.98
* @param {I2C_HandleTypeDef} *hi2c	i2c实例
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_Init(I2C_HandleTypeDef *hi2c)
{
	mlx90614_i2c = hi2c;
	MLX90614_Emissivity_Set(0.98); // 人体皮肤发射率设置
	return MLX90614_OK;
}
