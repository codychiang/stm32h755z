#include "w25q256.h"
#include "quadspi.h"
#include "usart.h" 

#define MCOLOR_NONE  "\033[m"
#define MCOLOR_RED   "\033[0;32;31m"
#define MCOLOR_GREEN "\033[0;32;32m"
#define BrightGREEN  "\033[1;32m"
#define BrightYELLOW "\033[1;33m"
#define BrightBLUE   "\033[1;34m"
#define BrightPURPLE "\033[1;35m"
#define BrightCYAN   "\033[1;36m"

#define infoMsg(X)     MCOLOR_GREEN X MCOLOR_NONE
#define errMsg(X)      MCOLOR_RED X MCOLOR_NONE
#define warnMsg(X)     BrightYELLOW X MCOLOR_NONE
#define brGREENMsg(X)  BrightGREEN X MCOLOR_NONE
#define brBLUEMsg(X)   BrightBLUE X MCOLOR_NONE
#define brPURPLEMsg(X) BrightPURPLE X MCOLOR_NONE
#define mPlayerMsg(X)  BrightCYAN X MCOLOR_NONE

 
uint8_t W25Q256_QPI_MODE=0;		//QSPI模式标志:0,SPI模式;1,QPI模式.

//4Kbytes为一个Sector
//16个扇区为1个Block
//W25Q256
//容量为32M字节,共有512个Block,8192个Sector 
													 
//初始化SPI FLASH的IO口
bool W25Q256_Init(uint16_t preferKind)
{ 
    uint8_t temp;    
    
    uint16_t W25Q256_TYPE = 0;	//默认是W25Q256
    uint32_t W25Q256_JEDECID = 0;	//默认是W25Q256
//    if(!W25Q256_ResetMemory_Enable()) return false;
 	if(!W25Q256_Qspi_Enable()) return false;			//使能QSPI模式
    if(!W25Q256_ReadID(&W25Q256_TYPE)) return false;	//读取FLASH ID.
	printf("TYPE:0x%X\r\n", W25Q256_TYPE);
	if(W25Q256_TYPE!=preferKind) return false;        //SPI FLASH为W25Q256
    if(!W25Q256_JedecID(&W25Q256_JEDECID)) return false;	//读取FLASH ID.
	printf("JEDEC:0x%X\r\n", (unsigned int)W25Q256_JEDECID);
        
    if(!W25Q256_ReadSR(3, &temp)) return false;      //读取状态寄存器3，判断地址模式
    if((temp&0X01)==0)			//如果不是4字节地址模式,则进入4字节地址模式
	{ 
		if(!W25Q256_Write_Enable()) return false;	//写使能
		if(HAL_OK != QSPI_Send_CMD(W25X_Enable4ByteAddr,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE)) return false;//QPI,使能4字节地址指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据 
	}
    
	if(!W25Q256_Write_Enable()) return false;		//写使能
	if(HAL_OK != QSPI_Send_CMD(W25X_SetReadParam,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES)) return false; 		//QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
	temp=3<<4;					//设置P4&P5=11,8个dummy clocks,104M
	if(HAL_OK != QSPI_Transmit(&temp,1)) return false;		//发送1个字节	   

    return true;
}  

bool W25Q256_SimpleInit()
{ 
    if(!W25Q256_ResetMemory_Enable()) return false;
	W25Q256_QPI_MODE=0;				//标记QSPI模式

	return true;
}

//W25Q256进入QSPI模式 
bool W25Q256_Qspi_Enable(void)
{
	uint8_t stareg2;
    if(!W25Q256_ReadSR(2, &stareg2)) return false;		//先读出状态寄存器2的原始值
	if((stareg2&0X02)==0)			//QE位未使能
	{
		if(!W25Q256_Write_Enable()) return false;		//写使能 
		stareg2|=1<<1;				//使能QE位		
		if(!W25Q256_Write_SR(2,stareg2)) return false;	//写状态寄存器2
	}
    
	if(HAL_OK != QSPI_Send_CMD(W25X_EnterQPIMode,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE)) return false;//写command指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
	W25Q256_QPI_MODE=1;				//标记QSPI模式
    return true;
}

//W25Q256退出QSPI模式 
bool W25Q256_Qspi_Disable(void)
{ 
	if(HAL_OK != QSPI_Send_CMD(W25X_ExitQPIMode,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE)) return false;//写command指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
	W25Q256_QPI_MODE=0;				//标记SPI模式
    return true;
}

//读取W25Q256的状态寄存器，W25Q256一共有3个状态寄存器
//状态寄存器1：
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
//状态寄存器2：
//BIT7  6   5   4   3   2   1   0
//SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
//状态寄存器3：
//BIT7      6    5    4   3   2   1   0
//HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
//regno:状态寄存器号，范:1~3
//返回值:状态寄存器值
bool W25Q256_ReadSR(uint8_t regno, uint8_t *pByte)   
{  
    printf(infoMsg("%s%d\n"), __func__, regno);

    HAL_StatusTypeDef rltStatus = HAL_OK; 
	uint8_t command=0; 
    *pByte = 0;
    switch(regno)
    {
        case 1:
            command=W25X_ReadStatusReg1;    //读状态寄存器1指令
            break;
        case 2:
            command=W25X_ReadStatusReg2;    //读状态寄存器2指令
            break;
        case 3:
            command=W25X_ReadStatusReg3;    //读状态寄存器3指令
            break;
        default:
            command=W25X_ReadStatusReg1;    
            break;
    }   

    
	if(W25Q256_QPI_MODE)
		rltStatus = QSPI_Send_CMD(command,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES);	//QPI,写command指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
	else
		rltStatus = QSPI_Send_CMD(command,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);				//SPI,写command指令,地址为0,单线传数据_8位地址_无地址_单线传输指令,无空周期,1个字节数据

    if(HAL_OK != rltStatus) return false;		
    if(HAL_OK != QSPI_Receive(pByte, 1)) return false;

	return true;
}   

bool W25Q256_ReadSR_1(uint8_t *pByte)   
{  
    printf(infoMsg("%s\n"), __func__);

    HAL_StatusTypeDef rltStatus = HAL_OK; 
    QSPI_CommandTypeDef s_command;
    s_command.Instruction       = W25X_ReadStatusReg1;
    s_command.Address           = 0;
    s_command.DummyCycles       = 0;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.NbData            = 1;

    if(HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        printf("command wrong ....\r\n");
        return false;
    }    

    if(HAL_QSPI_Receive(&hqspi, pByte, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
        printf("tx wrong ....\r\n");
        return false;
    }
    return true;
}   


bool W25Q256_ReadSR_2(uint8_t *pByte)   
{  
    printf(infoMsg("%s\n"), __func__);
    HAL_StatusTypeDef rltStatus = HAL_OK; 
    QSPI_CommandTypeDef s_command;
    s_command.Instruction       = W25X_ReadStatusReg2;
    s_command.Address           = 0;
    s_command.DummyCycles       = 0;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.NbData            = 1;

    if(HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        printf("command wrong ....\r\n");
        return false;
    }    

    if(HAL_QSPI_Receive(&hqspi, pByte, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
        printf("tx wrong ....\r\n");
        return false;
    }
    return true;
}   

//写W25Q256状态寄存器
bool W25Q256_Write_SR(uint8_t regno,uint8_t sr)   
{   
    printf(infoMsg("%s%d\n"), __func__, regno);

    HAL_StatusTypeDef rltStatus = HAL_OK; 
    uint8_t command=0;
    sr = 0;
    switch(regno)
    {
        case 1:
            command=W25X_WriteStatusReg1;    //写状态寄存器1指令
            break;
        case 2:
            command=W25X_WriteStatusReg2;    //写状态寄存器2指令
            break;
        case 3:
            command=W25X_WriteStatusReg3;    //写状态寄存器3指令
            break;
        default:
            command=W25X_WriteStatusReg1;    
            break;
    }   
	if(W25Q256_QPI_MODE)
        rltStatus = QSPI_Send_CMD(command,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES);    //QPI,写command指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
	else 
        rltStatus = QSPI_Send_CMD(command,0,0, QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);             //SPI,写command指令,地址为0,单线传数据_8位地址_无地址_单线传输指令,无空周期,1个字节数据

    if(HAL_OK != rltStatus) return false;
    if(HAL_OK != QSPI_Transmit(&sr,1)) return false;
    
    return true;
}  

bool W25Q256_Write_SR_2(uint8_t sr)   
{   
    printf(infoMsg("%s\n"), __func__);

    HAL_StatusTypeDef rltStatus = HAL_OK; 
    QSPI_CommandTypeDef s_command;
    s_command.Instruction       = W25X_WriteStatusReg2;
    s_command.Address           = 0;
    s_command.DummyCycles       = 0;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.NbData            = 1;

    if(HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        printf("command wrong ....\r\n");
        return false;
    }    

    if(HAL_QSPI_Transmit(&hqspi, &sr, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
        printf("tx wrong ....\r\n");
        return false;
    }
    return true;
}  

bool W25Q256_ResetMemory_Enable(void)   
{
    printf(infoMsg("%s\n"), __func__);

    HAL_StatusTypeDef rltStatus;
	if(W25Q256_QPI_MODE)
        rltStatus = QSPI_Send_CMD(W25X_ResetMemory,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);  //QPI,写使能指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
	else 
        rltStatus =  QSPI_Send_CMD(W25X_ResetMemory,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);               //SPI,写使能指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据

    return (HAL_OK == rltStatus);
} 

//W25Q256写使能	
//将S1寄存器的WEL置位   
bool W25Q256_Write_Enable(void)   
{
    printf(infoMsg("%s\n"), __func__);
    HAL_StatusTypeDef rltStatus;
	if(W25Q256_QPI_MODE)
        rltStatus = QSPI_Send_CMD(W25X_WriteEnable,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);  //QPI,写使能指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
	else 
        rltStatus = QSPI_Send_CMD(W25X_WriteEnable,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);               //SPI,写使能指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据

    return (HAL_OK == rltStatus);
} 

bool W25Q256_WriteVSR_Enable(void)  
{
    printf(infoMsg("%s\n"), __func__);

    HAL_StatusTypeDef rltStatus;
	if(W25Q256_QPI_MODE)
        rltStatus = QSPI_Send_CMD(W25X_WriteVolatileStatusRegister,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);  //QPI,写使能指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
	else 
        rltStatus = QSPI_Send_CMD(W25X_WriteVolatileStatusRegister,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);               //SPI,写使能指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据

    return (HAL_OK == rltStatus);
} 

//W25Q256写禁止	
//将WEL清零  
bool W25Q256_Write_Disable(void)   
{  
    printf(infoMsg("%s\n"), __func__);

    HAL_StatusTypeDef rltStatus;
	if(W25Q256_QPI_MODE)
        rltStatus = QSPI_Send_CMD(W25X_WriteDisable,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,写禁止指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
	else 
        rltStatus = QSPI_Send_CMD(W25X_WriteDisable,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);              //SPI,写禁止指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据 

    return (HAL_OK == rltStatus);
} 

bool W25Q256_XFER(uint8_t *pTxData, int txSize, uint8_t *pRxData, int rxSize)
{
//    HAL_StatusTypeDef rltStatus;
    uint8_t cmd = pTxData[0];
    uint8_t *txParaPtr = &pTxData[1];
    int txParaLen = txSize - 1;

    printf(infoMsg("command:0x%x\r\n"), cmd);

#if 1
    if(cmd == 0x0b){//fast read
        uint32_t address = (txParaPtr[0] << 16) + (txParaPtr[1] << 8) + (txParaPtr[2] << 0); 
        printf("addr=0x%x\n", address);
    
        QSPI_CommandTypeDef s_command;
        s_command.Instruction       = cmd;
        s_command.Address           = address;
        s_command.DummyCycles       = 8;
        s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
        s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
        s_command.DataMode          = QSPI_DATA_1_LINE;
        s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
        s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
        s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
        s_command.NbData            = rxSize;
   
        if(HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            printf("command wrong ....\r\n");
            return false;
        }   
   
        if(rxSize > 0){
            if(HAL_QSPI_Receive(&hqspi, pRxData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
                printf("rx wrong ....\r\n");
                return false;
            }
   
            printf("spi rx: ");
     		for(int i = 0; i < rxSize; i++){
    		    printf("%02X ", pRxData[i]);
                if(i % 30 == 29) printf("\n");
            }
            printf("...\n");
        }
    }
    else if(cmd == 0x52){//32KB Block Erase
        uint32_t address = (txParaPtr[0] << 16) + (txParaPtr[1] << 8) + (txParaPtr[2] << 0); 
        printf("addr=0x%x\n", address);
    
        QSPI_CommandTypeDef s_command;
        s_command.Instruction       = cmd;
        s_command.Address           = address;
        s_command.DummyCycles       = 0;
        s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
        s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
        s_command.DataMode          = QSPI_DATA_NONE;
        s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
        s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
        s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
        s_command.NbData            = 0;
   
        if(HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            printf("command wrong ....\r\n");
            return false;
        }
    }
    else if(cmd == 0x02){//page program
        int addrSize = 3;
        uint32_t address = (txParaPtr[0] << 16) + (txParaPtr[1] << 8) + (txParaPtr[2] << 0); 
        uint32_t dataLen = txParaLen - addrSize/*address*/;
        uint8_t *dataPtr = txParaPtr + addrSize;
        printf("addr=0x%x, dataLen=%d\n", address, dataLen);
    
        QSPI_CommandTypeDef s_command;
        s_command.Instruction       = cmd;
        s_command.Address           = address;
        s_command.DummyCycles       = 0;
        s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
        s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
        s_command.DataMode          = QSPI_DATA_1_LINE;
        s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
        s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
        s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
        s_command.NbData            = dataLen;

        if(HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            printf("command wrong ....\r\n");
            return false;
        }    
   
        if(dataLen > 0){
            if(HAL_QSPI_Transmit(&hqspi, dataPtr, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
                printf("tx wrong ....\r\n");
                return false;
            }
   
            printf("spi tx: ");
     		 for(int i = 0; i < dataLen; i++){
    		    printf("%02X ", dataPtr[i]);
                if(i % 30 == 29) printf("\n");
            }
            printf("\n");             
   
        }        
    }
    else{
        QSPI_CommandTypeDef s_command;
        s_command.Instruction       = cmd;
        s_command.Address           = 0;
        s_command.DummyCycles       = 0;
        s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        s_command.AddressMode       = QSPI_ADDRESS_NONE;
        s_command.AddressSize       = QSPI_ADDRESS_8_BITS;
        s_command.DataMode          = (txParaLen == 0 && rxSize == 0)? QSPI_DATA_NONE: QSPI_DATA_1_LINE;
        s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
        s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
        s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
        s_command.NbData            = (txParaLen > 0)? txParaLen: rxSize;
   
        if(HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            printf("command wrong ....\r\n");
            return false;
        }
   
        if(txParaLen > 0){
            if(HAL_QSPI_Transmit(&hqspi, txParaPtr, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
                printf("tx wrong ....\r\n");
                return false;
            }
   
            printf("spi tx: ");
     		 for(int i = 0; i < txParaLen; i++){
    		    printf("%02X ", txParaPtr[i]);
                if(i % 30 == 29) printf("\n");
            }
            printf("\n");             
   
        }
   
        if(rxSize > 0){
            if(HAL_QSPI_Receive(&hqspi, pRxData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
                printf("rx wrong ....\r\n");
                return false;
            }
   
            printf("spi rx: ");
     		 for(int i = 0; i < rxSize; i++){
    		    printf("%02X ", pRxData[i]);
                if(i % 30 == 29) printf("\n");
            }
            printf("\n");             
        }
    }
#else
	if(W25Q256_QPI_MODE)
        rltStatus = QSPI_Send_CMD(cmd,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES);//QPI,读id,地址为0,4线传输数据_24位地址_4线传输地址_4线传输指令,无空周期,2个字节数据
	else 
        rltStatus = QSPI_Send_CMD(cmd,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);         //SPI,读id,地址为0,单线传输数据_24位地址_单线传输地址_单线传输指令,无空周期,2个字节数据

    if(HAL_OK != rltStatus) return false;
    if(txParaLen > 0){
        if(HAL_OK != QSPI_Transmit(txParaPtr, txParaLen)) return false;
    }
    if(rxSize > 0){
        if(HAL_OK != QSPI_Receive(pRxData, rxSize)) return false;

        printf("spi rcv: ");
		for(int i = 0; i < rxSize; i++){
		    printf("%02X ", pRxData[i]);
            if(i % 20 == 19) printf("\n");
        }
        printf("\n");    
    }    

    if(cmd == W25X_JedecDeviceID){
        uint16_t W25Q256_TYPE = 0;
        if(!W25Q256_ReadID(&W25Q256_TYPE)) return false;	//读取FLASH ID.
    	printf("TYPE:0x%X\r\n", W25Q256_TYPE);
    }
#endif

    return true;
}    

//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 	  
//0XEF18,表示芯片型号为W25Q256
bool W25Q256_ReadID(uint16_t *pDeviceid)
{
    printf(infoMsg("%s\n"), __func__);

    *pDeviceid = 0;
    HAL_StatusTypeDef rltStatus;
	uint8_t temp[2];

	if(W25Q256_QPI_MODE)
        rltStatus = QSPI_Send_CMD(W25X_ManufactDeviceID,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_ADDRESS_24_BITS,QSPI_DATA_4_LINES);//QPI,读id,地址为0,4线传输数据_24位地址_4线传输地址_4线传输指令,无空周期,2个字节数据
	else 
        rltStatus = QSPI_Send_CMD(W25X_ManufactDeviceID,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_24_BITS,QSPI_DATA_1_LINE);         //SPI,读id,地址为0,单线传输数据_24位地址_单线传输地址_单线传输指令,无空周期,2个字节数据

    if(HAL_OK != rltStatus) return false;
    if(HAL_OK != QSPI_Receive(temp, 2)) return false;
  	*pDeviceid = (temp[0]<<8)|temp[1];

    return true;
}    

bool W25Q256_JedecID(uint32_t *pJedecDeviceId)
{
    printf(infoMsg("%s\n"), __func__);

    *pJedecDeviceId = 0;
    HAL_StatusTypeDef rltStatus;
	uint8_t temp[3];

	//if(W25Q256_QPI_MODE)
        //rltStatus = QSPI_Send_CMD(W25X_JedecDeviceID,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_ADDRESS_24_BITS,QSPI_DATA_4_LINES);//QPI,读id,地址为0,4线传输数据_24位地址_4线传输地址_4线传输指令,无空周期,2个字节数据
	//else
        rltStatus = QSPI_Send_CMD(W25X_JedecDeviceID,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);         //SPI,读id,地址为0,单线传输数据_24位地址_单线传输地址_单线传输指令,无空周期,2个字节数据

    if(HAL_OK != rltStatus) return false;
    if(HAL_OK != QSPI_Receive(temp, 3)) return false;
  	*pJedecDeviceId = (temp[0]<<16)|(temp[1]<<8)|temp[2];

    return true;
}    


//读取SPI FLASH,仅支持QPI模式  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(最大32bit)
//NumByteToRead:要读取的字节数(最大65535)
bool W25Q256_FastRead(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
    printf(infoMsg("%s\n"), __func__);

	if(HAL_OK != QSPI_Send_CMD(W25X_FastReadData,ReadAddr,8,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_ADDRESS_32_BITS,QSPI_DATA_4_LINES)) return false;	//QPI,快速读数据,地址为ReadAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,8空周期,NumByteToRead个数据
  	if(HAL_OK != QSPI_Receive(pBuffer,NumByteToRead)) return false;

    return true;
}  

bool W25Q256_FastRead_1L(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
    printf(infoMsg("%s\n"), __func__);

	if(HAL_OK != QSPI_Send_CMD(W25X_FastReadData,ReadAddr,8,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_24_BITS,QSPI_DATA_1_LINE)) return false;	//QPI,快速读数据,地址为ReadAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,8空周期,NumByteToRead个数据
  	if(HAL_OK != QSPI_Receive(pBuffer,NumByteToRead)) return false;

    return true;
}  

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
bool W25Q256_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    printf(infoMsg("%s\n"), __func__);

	if(!W25Q256_Write_Enable()) return false;					//写使能
	if(HAL_OK != QSPI_Send_CMD(W25X_PageProgram,WriteAddr,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_ADDRESS_32_BITS,QSPI_DATA_4_LINES)) return false;	//QPI,页写指令,地址为WriteAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据
	if(HAL_OK != QSPI_Transmit(pBuffer,NumByteToWrite)) return false;        	      
	if(!W25Q256_Wait_Busy()) return false;					   //等待写入结束

	return true;
} 

//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
bool W25Q256_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
    printf(infoMsg("%s\n"), __func__);

	uint16_t pageremain;	   
	pageremain=W25_onePage-WriteAddr%W25_onePage; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		if(!W25Q256_Write_Page(pBuffer,WriteAddr,pageremain)) return false;
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>W25_onePage)pageremain=W25_onePage; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	}
    
    return true;
} 

bool W25Q256_Write_OnePage(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    printf(infoMsg("%s\n"), __func__);

	if(HAL_OK != QSPI_Send_CMD(W25X_PageProgram,WriteAddr,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_24_BITS,QSPI_DATA_1_LINE)) return false;	//QPI,页写指令,地址为WriteAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据
	if(HAL_OK != QSPI_Transmit(pBuffer,NumByteToWrite)) return false;        	      

	return true;
} 


//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
uint8_t W25Q256_BUFFER[W25_oneSector];		 
bool W25Q256_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
    printf(infoMsg("%s\n"), __func__);

	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
	uint8_t * W25Q256_BUF;	  
   	W25Q256_BUF=W25Q256_BUFFER;	     
 	secpos=WriteAddr/W25_oneSector;//扇区地址  
	secoff=WriteAddr%W25_oneSector;//在扇区内的偏移
	secremain=W25_oneSector-secoff;//扇区剩余空间大小   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		if(!W25Q256_FastRead(W25Q256_BUF,secpos*W25_oneSector,W25_oneSector)) return false;//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25Q256_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			if(!W25Q256_Erase_Sector(secpos)) return false;//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				W25Q256_BUF[i+secoff]=pBuffer[i];	  
			}
			if(!W25Q256_Write_NoCheck(W25Q256_BUF,secpos*W25_oneSector,W25_oneSector)) return false;//写入整个扇区
		}
        else{ 
            if(!W25Q256_Write_NoCheck(pBuffer,WriteAddr,secremain)) return false;//写已经擦除了的,直接写入扇区剩余区间.                 
        }
        
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>W25_oneSector)secremain=W25_oneSector;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 

    return true;
}

//擦除整个芯片		  
//等待时间超长...
bool W25Q256_Erase_Chip(void)   
{                  
    printf(infoMsg("%s\n"), __func__);

    if(!W25Q256_Write_Enable()) return false;					//SET WEL 
    if(!W25Q256_Wait_Busy()) return false;
    if(HAL_OK != QSPI_Send_CMD(W25X_ChipErase,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE)) return false;//QPI,写全片擦除指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    if(!W25Q256_Wait_Busy()) return false;						//等待芯片擦除结束

    return true;
} 

//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个扇区的最少时间:150ms
bool W25Q256_Erase_Sector(uint32_t sectorNo)   
{  
    printf(infoMsg("%s\n"), __func__);
	 
 	//printf("fe:%x\r\n",Dst_Addr);			//监视falsh擦除情况,测试用  	  
 	uint32_t Dst_Addr = sectorNo * W25_oneSector;
    if(!W25Q256_Write_Enable()) return false;                  //SET WEL 	 
    if(!W25Q256_Wait_Busy()) return false;  
    if(HAL_OK != QSPI_Send_CMD(W25X_SectorErase,Dst_Addr,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_ADDRESS_32_BITS,QSPI_DATA_NONE)) return false;//QPI,写扇区擦除指令,地址为0,无数据_32位地址_4线传输地址_4线传输指令,无空周期,0个字节数据
    if(!W25Q256_Wait_Busy()) return false;   				    //等待擦除完成

    return true;
}


bool W25Q256_32KB_Block_Erase(uint32_t addr)   
{  	 
    printf(infoMsg("%s\n"), __func__);

    if(HAL_OK != QSPI_Send_CMD(W25X_32KB_BlockErase,addr,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_24_BITS,QSPI_DATA_NONE)) return false;//QPI,写扇区擦除指令,地址为0,无数据_32位地址_4线传输地址_4线传输指令,无空周期,0个字节数据

    return true;
}


//等待空闲
bool W25Q256_Wait_Busy(void)   
{   
    printf(infoMsg("%s\n"), __func__);

    uint8_t data = 0x01;
    while((data&0x01)==0x01){// 等待BUSY位清空
        if(!W25Q256_ReadSR(1, &data)) return false;
    }

    return true;   
}   

bool W25Q256_Write_ExtendedAddress(uint8_t ear)
{
    printf(infoMsg("%s\n"), __func__);

	if(HAL_OK != QSPI_Send_CMD(W25X_WriteExtendedAddressRegister,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_32_BITS,QSPI_DATA_1_LINE)) return false;	//QPI,页写指令,地址为WriteAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据
	if(HAL_OK != QSPI_Transmit(&ear, 1)) return false;        	      

	return true;
} 







