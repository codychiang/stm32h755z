#ifndef __W25Q256_H
#define __W25Q256_H
#include "gpio.h"
#include "sys.h"
#include "stdbool.h"


//W25X系列/Q系列芯片列表	   
 
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
#define W25Q256 0XEF18
#define W25Q512 0XEF19

#define W25_onePage    (256)
#define W25_oneSector  (W25_onePage * 16)//4K
#define W25_oneBlock   (W25_oneSector * 16)//64K
#define W25_oneChip    (W25_oneBlock * 512)//32M

////////////////////////////////////////////////////////////////////////////////// 
//指令表
#define W25X_ResetMemory   0x66
#define W25X_WriteEnable		  0x06 
#define W25X_WriteDisable		  0x04 
#define W25X_ReadStatusReg1		0x05 
#define W25X_ReadStatusReg2		0x35 
#define W25X_ReadStatusReg3		0x15 
#define W25X_WriteStatusReg1  0x01 
#define W25X_WriteStatusReg2  0x31 
#define W25X_WriteStatusReg3  0x11 
#define W25X_ReadData			    0x03 
#define W25X_FastReadData		  0x0B 
#define W25X_FastReadDual		  0x3B 
#define W25X_PageProgram		  0x02 
#define W25X_BlockErase			  0xD8 
#define W25X_SectorErase		  0x20 
#define W25X_ChipErase			  0xC7 
#define W25X_PowerDown			  0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			    0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
#define W25X_Enable4ByteAddr  0xB7
#define W25X_Exit4ByteAddr    0xE9
#define W25X_SetReadParam		  0xC0 
#define W25X_EnterQPIMode     0x38
#define W25X_ExitQPIMode      0xFF
#define W25X_WriteVolatileStatusRegister 0x50
#define W25X_WriteExtendedAddressRegister 0xC5
#define W25X_32KB_BlockErase 0x52



bool W25Q256_Init(uint16_t preferKind);					//初始化W25Q256
bool W25Q256_SimpleInit();
bool W25Q256_Qspi_Enable(void);			//使能QSPI模式
bool W25Q256_Qspi_Disable(void);			//关闭QSPI模式
bool W25Q256_ReadID(uint16_t *pDeviceid);	//读取FLASH ID
bool W25Q256_JedecID(uint32_t *pJedecDeviceId);
bool W25Q256_ReadSR(uint8_t regno, uint8_t *pByte);             //读取状态寄存器 
bool W25Q256_ReadSR_1(uint8_t *pByte);
bool W25Q256_ReadSR_2(uint8_t *pByte);
//void W25Q256_4ByteAddr_Enable(void);     //使能4字节地址模式
bool W25Q256_XFER(uint8_t *pTxData, int txSize, uint8_t *pRxData, int rxSize);
bool W25Q256_Write_SR(uint8_t regno,uint8_t sr);   //写状态寄存器
bool W25Q256_Write_SR_2(uint8_t sr);
bool W25Q256_ResetMemory_Enable(void);
bool W25Q256_WriteVSR_Enable(void);
bool W25Q256_Write_Enable(void);  		//写使能 
bool W25Q256_Write_Disable(void);		//写保护
bool W25Q256_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,u16_t NumByteToWrite);//写flash,不校验
bool W25Q256_Write_OnePage(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
bool W25Q256_FastRead(uint8_t* pBuffer,uint32_t ReadAddr,u16_t NumByteToRead);   //读取flash
bool W25Q256_FastRead_1L(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);
bool W25Q256_Write(uint8_t* pBuffer,uint32_t WriteAddr,u16_t NumByteToWrite);//写入flash
bool W25Q256_Erase_Chip(void);    	  	//整片擦除
bool W25Q256_Erase_Sector(uint32_t Dst_Addr);	//扇区擦除
bool W25Q256_Wait_Busy(void);           	//等待空闲
bool W25Q256_Write_ExtendedAddress(uint8_t ear);

#endif
