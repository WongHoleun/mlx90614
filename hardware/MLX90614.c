/**
****************************************************************************************
* @Description: �����¶ȴ�����MLX90614����ģ��
* @Author: Haolin Wang
* @Date: 2023-03-20 15:56:05
* @LastEditTime: 2023-03-25 18:00:59
* @Note:i2c.C�ļ����� __HAL_RCC_I2C1_CLK_ENABLE() ��  __HAL_RCC_GPIOB_CLK_ENABLE() ǰ��
****************************************************************************************
*/

#include "i2c.h"
#include "math.h"
#include "MLX90614.h"

I2C_HandleTypeDef *mlx90614_i2c;

/**
****************************************************************************************
* @Funticon name: ��ȡ RAM������д��
* @Berif:
* @Note: �û��޷�д�� RAM �ڴ棬ֻ�ܽ��ж�ȡ������ֻ�����޵�һ���� RAM �Ĵ����Կͻ������塣
* MLX90614 ��RAM�� 32 �� 17 λ�洢��Ԫ������TA,TOBJ1,TOBJ2�ǻ����¶Ⱥ������¶�,
* ��SMBus��ʽ�£����Դ��⼸���洢��Ԫ���������ͱ���������¶ȡ�
* @param {uint8_t} Register	RAM �Ĵ�����ַ
* @param {uint16_t} *Result	��ȡ������� �ĵ�ַ
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_RAM_Read(uint8_t Register, uint16_t *Result)
{
	uint8_t tmp[2];

	int res = HAL_I2C_Mem_Read(mlx90614_i2c, MLX90614_ADDRESS, Register, I2C_MEMADD_SIZE_8BIT, tmp, 2, 10); // ��ȡ RAM �Ĵ������ݣ�2�ֽڣ� ����������״̬

	if (HAL_OK != res)
	{
		return MLX90614_ERROR; // ���� ����ͨ�ű�ʶ
	}
	else
	{
		*Result = (uint16_t)tmp[1] << 8 | tmp[0]; // ȡ���Ĵ������� 16bit����
		return MLX90614_OK;						  // ���� ����ͨ�ű�ʶ
	}
}

/**
****************************************************************************************
* @Funticon name: ��ȡ EEPROM (���ֿ�д)
* @Berif: Briefly describe the function of your function
* @Note: ֻ��ĳЩ�洢��Ԫ�û��ܹ�д�룬���ǿ��Զ���ȫ���洢��Ԫ��
* MLX90614 �� EEPROM �� 32 �� 16 λ�洢��Ԫ�����д洢��Ԫ Tomax,Tomin,Ta
* �ֱ��� �û������¶������޺ͻ����¶ȷ�Χ��PWMCTRL �� PWM ���üĴ�����
* @param {uint8_t} Register	EEPROM �Ĵ�����ַ
* @param {uint16_t} *Result	��ȡ������� �ĵ�ַ
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_EEPROM_Read(uint8_t Register, uint16_t *Result)
{
	uint8_t tmp[2];
	int res = HAL_I2C_Mem_Read(mlx90614_i2c, MLX90614_ADDRESS, Register, I2C_MEMADD_SIZE_8BIT, tmp, 2, 10); // ��ȡ EEPROM �Ĵ�������(2�ֽ�) ����������״̬

	if (HAL_OK != res)
	{
		return MLX90614_ERROR; // ���� ����ͨ�ű�ʶ
	}
	else
	{
		*Result = (uint16_t)tmp[1] << 8 | tmp[0]; // ȡ���Ĵ������� 16bit����
		return MLX90614_OK;						  // ���� ����ͨ�ű�ʶ
	}
}

/**
****************************************************************************************
* @Funticon name: CRC-8У��
* @Berif: ��ѭ�����㷨��
* @Note: ���ڶ���ʽ�����Ϊ��Ϊ1�������ڴ����crc8�����У����λҲ�ǲ�ʹ�õģ�
* �����ڶ���ʽ��¼ʱ��ȥ�������λ������ʽ���λΪ1��������Ҫ����������λΪ1ʱ���Ž��������㣬
* �����������λ��Ϊ0�ˣ����λΪ0���´�Ҳ����Ҫ����ˣ��Ϳ��԰����λȥ����
* @param {uint8_t} IntCrc	CRC��ֵ
* @param {uint8_t} Data	����������
* @return {*}		CRCУ���� ��PEC�����ж���ʽG(X) = X^8+X^2+X^1+1 �� CRC-8��
****************************************************************************************
*/
uint8_t CRC_8(uint8_t IntCrc, uint8_t Data)
{
	uint8_t i;
	uint8_t data;
	data = IntCrc ^= Data; // CRCУ���루��һ�ֽ�����У����������Ҫ������������

	for (i = 0; i < 8; i++) // 8���жϡ���λ������������1���ֽڣ�
	{
		if ((data & 0x80) != 0) // �ж��������λΪ1����Ҫ���
		{
			data <<= 1;	  // ����һλ��ȥ���������λ1�����ɶ���ʽ���λҲ��1����һλ����Ҫ��� ������Ϣ����ʽ 0x11 ����һλ 1 0001 0000 (������8λ�����������һ��)
			data ^= 0x07; // ��0x07���       ����ʽG(X) = X^8+X^2+X^1+1  ת��Ϊ2����ȥ����λ�� 0x07      ���ɶ���ʽ0x107    1 0000 0111
		}
		else // ���λΪ0������Ҫ���
		{
			data <<= 1; // �������� ȥ����λ��0
		}
	}
	return data; // �õ��ĳ���������������Ϊ����Ҫ���CRCУ����
}

/**
****************************************************************************************
* @Funticon name: ����д�� EEPROM ����������LSB��MSB��PEC��
* @Berif:
* @Note: ���ڶ���ʽ�����Ϊ��Ϊ1�������ڴ����crc8�����У����λҲ�ǲ�ʹ�õģ������ڶ���ʽ��¼ʱ��ȥ�������λ��
* ����ʽ���λΪ1��������Ҫ����������λΪ1ʱ���Ž��������㣬�����������λ��Ϊ0�ˣ����λΪ0���´�Ҳ����Ҫ����ˣ��Ϳ��԰����λȥ����
* @param {uint8_t} Address	�Ĵ�����ַ
* @param {uint16_t} Value	д���������ݣ�2�ֽڣ�
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_Write_Data(uint8_t Address, uint16_t Value)
{
	uint8_t crc;
	uint8_t tmp[3];

	/*����PEC ���ݴ�����*/
	crc = CRC_8(0, MLX90614_ADDRESS); // �豸��ַУ�� ��ʼcrcУ���루�����㷨��ʼʱ�Ĵ���CRC�ĳ�ʼ��Ԥ��ֵΪ0��
	crc = CRC_8(crc, Address);		  // ���ӼĴ�����ַУ��
	crc = CRC_8(crc, Value & 0xFF);	  // ���ӵ��ֽ�У��
	crc = CRC_8(crc, Value >> 8);	  // ���Ӹ��ֽ�У��

	tmp[0] = Value & 0xFF; // д��LSB
	tmp[1] = Value >> 8;   // д��MSB
	tmp[2] = crc;		   // д��PEC

	int res = HAL_I2C_Mem_Write(mlx90614_i2c, MLX90614_ADDRESS, Address, 1, tmp, 3, 10); // ������(LSB MSB PEC)  д��EEPROM �Ĵ��� ����������״̬

	if (res != HAL_OK)
	{
		return MLX90614_ERROR; // ���� ͨ�Ŵ����ʶ
	}
	else
	{
		return MLX90614_OK; // ���� ͨ��������ʶ
	}
}

/**
****************************************************************************************
* @Funticon name: EEPROM �Ĵ��� д��
* @Berif: Briefly describe the function of your function
* @Note: �����޸�EEPROM �����¶������޺ͻ����¶ȷ�Χ������ϵ����SMBus�豸��ַ��PWMʹ��
* д��ǰ��Ҫ�Ȳ����Ĵ�������
* @param {uint8_t} Address		�Ĵ�����ַ
* @param {uint16_t} Value		д���������ݣ�2�ֽڣ�
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_Write_EEPROM(uint8_t Address, uint16_t Value)
{

	uint16_t Verify;

	/*���� EEPROM�Ĵ��� ����*/
	int res = MLX90614_Write_Data(Address, 0);
	if (MLX90614_OK != res) // д��0 ���ж�
	{
		return MLX90614_ERROR; // ���� ͨ�Ŵ����ʶ
	}
	HAL_Delay(10); // ����ʱ��

	/*�޸� EEPROM�Ĵ��� ����*/
	res = MLX90614_Write_Data(Address, Value);
	if (MLX90614_OK != res) // ��Ĵ���д������ ���ж�
	{
		return MLX90614_ERROR; // ���� ͨ�Ŵ����ʶ
	}
	HAL_Delay(10); // д��ʱ��

	/*���� EEPROM�Ĵ��� ����*/
	MLX90614_EEPROM_Read(Address, &Verify); // ��ȡ�Ĵ���ֵ �����Ƿ�洢��ȷ
	if (Verify != Value)
	{
		return MLX90614_ERROR; // ���� ͨ�Ŵ����ʶ
	}

	return MLX90614_OK; // ���� ͨ����ȷ��ʶ
}

/**
****************************************************************************************
* @Funticon name: �¶ȴ���������ת�����϶ȣ�
* @Berif:
* @Note:��������Ҫ�����¶Ȳ�����������ѹ���¶�Ӱ�죬����У׼��ѹΪ 3 V������ʹ�õĹ����ѹ 3.3V
*		�����������¶ȶ� VDD �ĵ���������Ϊ 0.6��C/V
* @param {uint16_t} Temperature	�������¶�
* @return {*}		���϶��¶�ֵ�������ͣ�
****************************************************************************************
*/
//float MLX90614_Kelvin_Celsius(uint16_t Temperature)
//{
//	float Result;
//
//	Result = (float)Temperature * 0.02;
//	Result = Result - 273.15;
//	Result = Result - ((3.3 - 3) * 0.6);	// �¶Ȳ���
//
//	return Result;
//}

int MLX90614_Kelvin_Celsius(uint16_t Temperature)
{
	int Result;

	Result = Temperature * 2;
	Result = Result - 27315;
	Result = Result - 18;	// �¶Ȳ���

	return Result;
}

/**
****************************************************************************************
* @Funticon name: ��ȡ�����¶�
* @Berif: Briefly describe the function of your function
* @Note: Need note condition
* @return {*} �����¶�ֵ
****************************************************************************************
*/
int Mlx90614_measure_temperature(void)
{
	uint16_t tmp;

	int res = MLX90614_RAM_Read(MLX90614_TOBJ1, &tmp); // ��ȡ RAM �����¶� To1 �Ĵ���
	if (MLX90614_OK != res)
	{
		return MLX90614_ERROR;
	}
	else
	{
		return (MLX90614_Kelvin_Celsius(tmp)); // ��������¶�
	}
}

/**
****************************************************************************************
* @Funticon name: �޸ķ�����
* @Berif:
* @Note: �޸�ǰ��Ҫ���в���
* @param {float} Emissivity	�·�����
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_Emissivity_Set(float Emissivity)
{
	uint16_t current_emissivity;
	uint16_t new_emissivity;

	if (Emissivity < 0.1 || Emissivity > 1) // �жϷ������Ƿ�����Ч��Χ
	{
		return MLX90614_ERROR;
	}

	int res = MLX90614_EEPROM_Read(MLX90614_EMISSIVITY, &current_emissivity); // ��ȡ �����ʼĴ��� ����
	if (MLX90614_OK != res)
	{
		return MLX90614_ERROR;
	}

	new_emissivity = (uint16_t)round((65535.0 * Emissivity - 1)); // �������� ������ֵ

	if (new_emissivity == current_emissivity) // �ж� �¾ɷ������Ƿ�һ�� һ������Ҫ�޸�
	{
		return MLX90614_OK;
	}

	res = MLX90614_Write_EEPROM(MLX90614_EMISSIVITY, new_emissivity); // д�� �µķ����� ��������״̬
	if (MLX90614_OK != res)
	{
		return MLX90614_ERROR;
	}

	return MLX90614_OK;
}

/**
****************************************************************************************
* @Funticon name: ��������ַ�޸�
* @Berif: ���ڶഫ������ַ���䣬�����豸��ַΪ0x00��ַ��
* �޸��µ�ַ���ô˵�ַ�����豸�ഫ����ʱ��Ĭ�ϵ�ַΪ0x5A
* @Note:
* @param {uint8_t} Address	�µ��豸��ַ
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_SMBusAddress_Set(uint8_t Address)
{
	if (Address > 0x7F)
	{
		return MLX90614_ERROR;
	}
	int res = MLX90614_Write_EEPROM(MLX90614_ADDR, Address); // д�� �µ��豸SMBus��ַ ��������״̬
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
* @Funticon name: ��������ʼ�������ô���������i2cʵ����
* @Berif: �ڹ���ִ��֮ǰ��ɳ�ʼ��
* @Note: ������-�������������������֮�ȡ�ϵ������0 ��1 ֮�䡣����Ƥ��һ��Ϊ0.98
* @param {I2C_HandleTypeDef} *hi2c	i2cʵ��
* @return {*}
****************************************************************************************
*/
MLX90614_StatusTypeDef MLX90614_Init(I2C_HandleTypeDef *hi2c)
{
	mlx90614_i2c = hi2c;
	MLX90614_Emissivity_Set(0.98); // ����Ƥ������������
	return MLX90614_OK;
}
