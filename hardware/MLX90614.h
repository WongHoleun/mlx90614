/*
 * @Description: �����¶ȴ�����MLX90614����ģ��
 * @Author: Haolin Wang
 * @Date: 2023-03-20 15:56:05
 * @LastEditTime: 2023-03-26 15:59:04
 * @Note:
 */

#ifndef __MLX90614_H_
#define __MLX90614_H_

/* ��������ַ */
#define MLX90614_ADDRESS ((0x00) << 1)
// #define MLX90614_ADDRESS ((0x5A)<<1)
#define MLX90614_ADDRESS_R ((0x5A)<<1) | 0x01))	//��ַ ��

/* ������������ */
#define MLX90614_OP_RAM 0x00	// ����RAM
#define MLX90614_OP_EEPROM 0x20 // ����EEPROM
#define MLX90614_OP_SLEEP 0xFF	// ����˯��ģʽ

/* RAM �Ĵ�����ַ */
#define MLX90614_RAW1 (MLX90614_OP_RAM | 0x04)	// ��� IR�����⣩ͨ��1��ԭʼ����
#define MLX90614_RAW2 (MLX90614_OP_RAM | 0x05)	// ��� IR�����⣩ͨ��2��ԭʼ����
#define MLX90614_TA (MLX90614_OP_RAM | 0x06)	// ��� �����¶�
#define MLX90614_TOBJ1 (MLX90614_OP_RAM | 0x07) // ��� �����¶ȣ�����Դ��IR1��
#define MLX90614_TOBJ2 (MLX90614_OP_RAM | 0x08) // ��� �����¶ȣ�����Դ��IR2��

/* EEPROM �Ĵ�����ַ */
#define MLX90614_TOMAX (MLX90614_OP_EEPROM | 0x00)		// ��� �����¶�����
#define MLX90614_TOMIN (MLX90614_OP_EEPROM | 0x01)		// ��� �����¶�����
#define MLX90614_PWMCTRL (MLX90614_OP_EEPROM | 0x02)	// PWM���ʹ�� ���ƼĴ���
#define MLX90614_TARANGE (MLX90614_OP_EEPROM | 0x03)	// ��� �����¶ȷ�Χ
#define MLX90614_EMISSIVITY (MLX90614_OP_EEPROM | 0x04) // ��� ������У׼ϵ��
#define MLX90614_CFG1 (MLX90614_OP_EEPROM | 0x05)		// ��� �Ĵ���1����
#define MLX90614_ADDR (MLX90614_OP_EEPROM | 0x0E)		// ��� SMBus �豸��ַ

/* �豸����״̬ */
typedef enum
{
	MLX90614_OK = 0,
	MLX90614_ERROR = 1
} MLX90614_StatusTypeDef;

/* �ӿں������� */
MLX90614_StatusTypeDef MLX90614_RAM_Read(uint8_t Register, uint16_t *Result);	// ��ȡ RAM
MLX90614_StatusTypeDef MLX90614_EEPROM_Read(uint8_t Address, uint16_t *Result); // ��ȡ EEPROM
uint8_t CRC_8(uint8_t IntCrc, uint8_t Data);									// CRC-8 У��
MLX90614_StatusTypeDef MLX90614_Write_Data(uint8_t Address, uint16_t Value);	// ����д�� EEPROM ����������LSB��MSB��PEC��
MLX90614_StatusTypeDef MLX90614_Write_EEPROM(uint8_t Address, uint16_t Value);	// д�� EEPROM
int MLX90614_Kelvin_Celsius(uint16_t Temperature);							// �¶ȴ���������ת�����϶ȣ�
int Mlx90614_measure_temperature(void);										// ��ȡ �����¶�
MLX90614_StatusTypeDef MLX90614_Emissivity_Set(float Emissivity);				// �޸� ������
MLX90614_StatusTypeDef MLX90614_SMBusAddress_Set(uint8_t Address);				// �޸� �豸SMBus��ַ
MLX90614_StatusTypeDef MLX90614_Init(I2C_HandleTypeDef *hi2c);					// ������ ��ʼ��
#endif																			/* MLX90614_H_ */
