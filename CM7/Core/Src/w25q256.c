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

#define W25Q256_DBGINFO_GENERALCMD_ENABLE (0)
#define W25Q256_DBGINFO_READ_ENABLE (0)
#define W25Q256_DBGINFO_WRITE_ENABLE (0)
#define W25Q256_DBGINFO_CMD_ENABLE (0)
#define PseudoFlash  (0) 

 
uint8_t W25Q256_QPI_MODE=0;		//QSPI模式标志:0,SPI模式;1,QPI模式.
uint8_t W25Q256_Addr32Bits_MODE=0;		//QSPI模式标志:0,SPI模式;1,QPI模式.

#define W25Q256_Addr_MODE (W25Q256_Addr32Bits_MODE? QSPI_ADDRESS_32_BITS: QSPI_ADDRESS_24_BITS)

//4Kbytes为一个Sector
//16个扇区为1个Block
//W25Q256
//容量为32M字节,共有512个Block,8192个Sector 

void showStatus()
{
	uint8_t state1, state2, state3;
    state1=W25Q256_ReadSR(1);    printf("state1=0x%x, ", state1);
    state2=W25Q256_ReadSR(2);    printf("state2=0x%x, ", state2);
    state3=W25Q256_ReadSR(3);    printf("state3=0x%x\n", state3);
}

//初始化SPI FLASH的IO口
bool W25Q256_Init(uint16_t preferKind)
{ 
    
    uint16_t W25Q256_TYPE = 0;	//默认是W25Q256
    uint32_t W25Q256_JEDECID = 0;	//默认是W25Q256

    W25Q256_Qspi_Enable();			//使能QSPI模式
    showStatus();

    if(!W25Q256_JedecID(&W25Q256_JEDECID)) return false;	//读取FLASH ID.
    printf("JEDECID=0x%x\n", (unsigned int)W25Q256_JEDECID);


    if(!W25Q256_ReadID(&W25Q256_TYPE)) return false;	//读取FLASH ID.
    printf("TYPE=0x%x\n", W25Q256_TYPE);
	//printf("ID:%x\r\n",W25Q256_TYPE);
	if(W25Q256_TYPE==preferKind)        //SPI FLASH为W25Q256
    {
// 	      uint8_t temp;
//        temp=W25Q256_ReadSR(3);      //读取状态寄存器3，判断地址模式
//        if((temp&0X01)==0)			//如果不是4字节地址模式,则进入4字节地址模式
//		{
//			W25Q256_Write_Enable();	//写使能
//        	if(W25Q256_QPI_MODE){
//    			QSPI_Send_CMD(W25X_Enable4ByteAddr,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,使能4字节地址指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
//            }
//            else{
//    			QSPI_Send_CMD(W25X_Enable4ByteAddr,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,使能4字节地址指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
//            }
//		}
//		W25Q256_Write_Enable();		//写使能

//    	if(W25Q256_QPI_MODE){		
//    		QSPI_Send_CMD(W25X_SetReadParam,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES); 		//QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
//        }
//        else{
//    		QSPI_Send_CMD(W25X_SetReadParam,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE); 		//QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
//        }
//		temp=3<<4;					//设置P4&P5=11,8个dummy clocks,104M
//		QSPI_Transmit(&temp,1);		//发送1个字节

        return true;
    }

    return false;
}  
//W25Q256进入QSPI模式 
void W25Q256_Qspi_Enable(void)
{
#if 1
	printf("%s\n", __func__);
#endif
	uint8_t stareg1, stareg2;
    stareg2=W25Q256_ReadSR(2);		//先读出状态寄存器2的原始值
#if W25Q256_DBGINFO_CMD_ENABLE
	printf("%s=>1 stareg2=0x%x\n", __func__, stareg2);    
#endif
	//if((stareg2&0X02)==0x00)			//QE位未使能
	{
		//W25Q256_WriteVolatile_Enable();		//写使能 
		W25Q256_Write_Enable();
        stareg1=W25Q256_ReadSR(1);		//先读出状态寄存器2的原始值
#if W25Q256_DBGINFO_CMD_ENABLE
    	printf("%s=>1 stareg1=0x%x\n", __func__, stareg1);
#endif
        
//		stareg2 &= 0x87;
//		stareg2 |= 0x02;				//使能QE位
		stareg2 = 0x02;
		W25Q256_Write_SR(2,stareg2);	//写状态寄存器2
		
        stareg1=W25Q256_ReadSR(1);		//先读出状态寄存器2的原始值
#if W25Q256_DBGINFO_CMD_ENABLE
    	printf("%s=>1 stareg1=0x%x\n", __func__, stareg1);
#endif
	}
    stareg2=W25Q256_ReadSR(2);		//先读出状态寄存器2的原始值
#if W25Q256_DBGINFO_CMD_ENABLE
	printf("%s=>2 stareg2=0x%x\n", __func__, stareg2);
#endif
    W25Q256_setReadParam();

//	QSPI_Send_CMD(W25X_EnterQPIMode,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//写command指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
//	W25Q256_QPI_MODE=1;				//标记QSPI模式
}

//W25Q256退出QSPI模式 
void W25Q256_Qspi_Disable(void)
{ 
	QSPI_Send_CMD(W25X_ExitQPIMode,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//写command指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
	W25Q256_QPI_MODE=0;				//标记SPI模式
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
uint8_t W25Q256_ReadSR(uint8_t regno)   
{  
	uint8_t byte=0,cmd=0; 
    switch(regno)
    {
        case 1:
            cmd=W25X_ReadStatusReg1;    //读状态寄存器1指令
            break;
        case 2:
            cmd=W25X_ReadStatusReg2;    //读状态寄存器2指令
            break;
        case 3:
            cmd=W25X_ReadStatusReg3;    //读状态寄存器3指令
            break;
        default:
            cmd=W25X_ReadStatusReg1;    
            break;
    }   

#if 1
	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(cmd, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                   0/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, &byte/*rxData*/, 1/*rxSize*/)) return false;
	}
    else{
        if(!W25Q256_GeneralCmd(cmd, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                  0/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, &byte/*rxData*/, 1/*rxSize*/)) return false;
    }

#else
	if(W25Q256_QPI_MODE)
		QSPI_Send_CMD(cmd,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES);	//QPI,写command指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
	else
		QSPI_Send_CMD(cmd,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);				//SPI,写command指令,地址为0,单线传数据_8位地址_无地址_单线传输指令,无空周期,1个字节数据
	QSPI_Receive(&byte,1);	        
#endif
	return byte;   
}   

//写W25Q256状态寄存器
bool W25Q256_Write_SR(uint8_t regno,uint8_t sr)   
{   
    uint8_t command=0;
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

#if 1
	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(command, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                   0/*address*/, 0/*dummyCycles*/, &sr/*txData*/, 1/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
	}
    else{
        if(!W25Q256_GeneralCmd(command, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                  0/*address*/, 0/*dummyCycles*/, &sr/*txData*/, 1/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }

#else
	if(W25Q256_QPI_MODE)
        QSPI_Send_CMD(command,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES);    //QPI,写command指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
	else 
        QSPI_Send_CMD(command,0,0, QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);             //SPI,写command指令,地址为0,单线传数据_8位地址_无地址_单线传输指令,无空周期,1个字节数据
	QSPI_Transmit(&sr,1);	         	      

#endif

	return true;
}  

bool W25Q256_setReadParam()
{
	uint8_t readParam=3<<4;					//设置P4&P5=11,8个dummy clocks,104M
#if 1
	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(W25X_SetReadParam, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                   0/*address*/, 0/*dummyCycles*/, &readParam/*txData*/, 1/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
	}
    else{
        if(!W25Q256_GeneralCmd(W25X_SetReadParam, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                  0/*address*/, 0/*dummyCycles*/, &readParam/*txData*/, 1/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }

#else
	if(W25Q256_QPI_MODE){		
		QSPI_Send_CMD(W25X_SetReadParam,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES); 		//QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
    }
    else{
		QSPI_Send_CMD(W25X_SetReadParam,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE); 		//QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
    }
	readParam=3<<4;					//设置P4&P5=11,8个dummy clocks,104M
	QSPI_Transmit(&readParam,1);		//发送1个字节
#endif

    W25Q256_Wait_Busy();

    return true;
}

//W25Q256写使能	
//将S1寄存器的WEL置位   
void W25Q256_Write_Enable(void)   
{
#if W25Q256_DBGINFO_CMD_ENABLE
    printf("%s\n", __func__);
#endif
    
	if(W25Q256_QPI_MODE)
        QSPI_Send_CMD(W25X_WriteEnable,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);  //QPI,写使能指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
	else 
        QSPI_Send_CMD(W25X_WriteEnable,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);               //SPI,写使能指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
} 

//W25Q256写禁止	
//将WEL清零  
void W25Q256_Write_Disable(void)   
{  
	if(W25Q256_QPI_MODE)
        QSPI_Send_CMD(W25X_WriteDisable,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,写禁止指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
	else 
        QSPI_Send_CMD(W25X_WriteDisable,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);              //SPI,写禁止指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据 
} 

bool W25Q256_GeneralCmd(uint8_t cmd, uint32_t instructionMode, uint32_t dataMode, uint32_t addressMode, uint32_t addressSize, 
                        uint32_t address, uint8_t dummyCycles, uint8_t *pTxData, int txSize, uint8_t *pRxData, int rxSize)
{
#if PseudoFlash
    memset(pTxData, 0, rxSize);
    return true;
#endif

    QSPI_CommandTypeDef s_command;
    s_command.Instruction       = cmd;
    s_command.Address           = address;
    s_command.DummyCycles       = dummyCycles;
    s_command.InstructionMode   = instructionMode;
    s_command.AddressMode       = addressMode;
    s_command.AddressSize       = addressSize;
    s_command.DataMode          = dataMode;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.NbData            = (txSize > 0)? txSize: rxSize;

    if(HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        printf("command wrong ....\r\n");
        return false;
    }

    if(txSize > 0){
        if(HAL_QSPI_Transmit(&hqspi, pTxData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
            printf("tx wrong ....\r\n");
            return false;
        }

        if(cmd == W25X_PageProgram){
#if W25Q256_DBGINFO_WRITE_ENABLE
            printf("spi tx: ");
    		 for(int i = 0; i < txSize; i++){
    		    printf("%02X ", pTxData[i]);
                if(i % 32 == 21) printf("\n");
            }
            printf("\n");      
#endif
        }
        else{
#if W25Q256_DBGINFO_GENERALCMD_ENABLE
            printf("spi tx: ");
    		 for(int i = 0; i < txSize; i++){
    		    printf("%02X ", pTxData[i]);
                if(i % 32 == 21) printf("\n");
            }
            printf("\n");
#endif
        }
    }

    if(rxSize > 0){
        if(HAL_QSPI_Receive(&hqspi, pRxData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK) {
            printf("rx wrong ....\r\n");
            return false;
        }


        if(cmd == W25X_FastReadData){
#if W25Q256_DBGINFO_READ_ENABLE
            printf("spi rx: ");
    		 for(int i = 0; i < rxSize; i++){
    		    printf("%02X ", pRxData[i]);
                if(i % 32 == 31) printf("\n");
            }
            printf("\n");             
#endif
        }
        else{
#if W25Q256_DBGINFO_GENERALCMD_ENABLE
            printf("spi rx: ");
    		 for(int i = 0; i < rxSize; i++){
    		    printf("%02X ", pRxData[i]);
                if(i % 32 == 31) printf("\n");
            }
            printf("\n");             
#endif         
        }
    }

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
	uint8_t rxData[2];
	*pDeviceid = 0;

#if 1  
	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(W25X_ManufactDeviceID, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_4_LINES, W25Q256_Addr_MODE,
                                   0/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, rxData/*rxData*/, 2/*rxSize*/)) return false;
	}
    else{
        if(!W25Q256_GeneralCmd(W25X_ManufactDeviceID, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_1_LINE, QSPI_ADDRESS_1_LINE, W25Q256_Addr_MODE,
                                  0/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, rxData/*rxData*/, 2/*rxSize*/)) return false;
    }

	*pDeviceid=(rxData[0]<<8)|rxData[1];
    return true;

#else
    uint16_t deviceid;
	if(W25Q256_QPI_MODE)
        QSPI_Send_CMD(W25X_ManufactDeviceID,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_ADDRESS_24_BITS,QSPI_DATA_4_LINES);//QPI,读id,地址为0,4线传输数据_24位地址_4线传输地址_4线传输指令,无空周期,2个字节数据
	else 
        QSPI_Send_CMD(W25X_ManufactDeviceID,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_24_BITS,QSPI_DATA_1_LINE);         //SPI,读id,地址为0,单线传输数据_24位地址_单线传输地址_单线传输指令,无空周期,2个字节数据
	QSPI_Receive(rxData,2);
	*pDeviceid=(rxData[0]<<8)|rxData[1];
	return true;
#endif
}    

bool W25Q256_JedecID(uint32_t *pJedecDeviceId)
{
#if W25Q256_DBGINFO_CMD_ENABLE
    printf(infoMsg("%s %dLine\n"), __func__, (W25Q256_QPI_MODE? 4: 1));
#endif

    *pJedecDeviceId = 0;
	uint8_t rxData[3];

#if 1
	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(W25X_JedecDeviceID, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                  0/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, rxData/*rxData*/, 3/*rxSize*/)) return false;
	}
    else{
        if(!W25Q256_GeneralCmd(W25X_JedecDeviceID, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                  0/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, rxData/*rxData*/, 3/*rxSize*/)) return false;
    }

  	*pJedecDeviceId = (rxData[0]<<16)|(rxData[1]<<8)|rxData[2];
    return true;
#else
    HAL_StatusTypeDef rltStatus;
	if(W25Q256_QPI_MODE){
        rltStatus = QSPI_Send_CMD(W25X_JedecDeviceID,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_ADDRESS_24_BITS,QSPI_DATA_4_LINES);//QPI,读id,地址为0,4线传输数据_24位地址_4线传输地址_4线传输指令,无空周期,2个字节数据
	}
	else{
        rltStatus = QSPI_Send_CMD(W25X_JedecDeviceID,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);         //SPI,读id,地址为0,单线传输数据_24位地址_单线传输地址_单线传输指令,无空周期,2个字节数据
	}

    if(HAL_OK != rltStatus) return false;
    if(HAL_OK != QSPI_Receive(rxData, 3)) return false;
  	*pJedecDeviceId = (rxData[0]<<16)|(rxData[1]<<8)|rxData[2];
#endif
    return true;
}    


//读取SPI FLASH,仅支持QPI模式  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(最大32bit)
//NumByteToRead:要读取的字节数(最大65535)
#if 0

bool W25Q256_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{
    if(W25Q256_QPI_MODE){
    	QSPI_Send_CMD(W25X_FastReadData,ReadAddr,8,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,W25Q256_Addr_MODE,QSPI_DATA_4_LINES);	//QPI,快速读数据,地址为ReadAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,8空周期,NumByteToRead个数据
    }
    else{
    	QSPI_Send_CMD(W25X_FastReadData,ReadAddr,16,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_24_BITS,QSPI_DATA_1_LINE);	//QPI,快速读数据,地址为ReadAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,8空周期,NumByteToRead个数据
    }
	QSPI_Receive(pBuffer,NumByteToRead); 

#if W25Q256_DBGINFO_GENERALCMD_ENABLE
    printf("spi rx: ");
	 for(int i = 0; i < NumByteToRead; i++){
	    printf("%02X ", pBuffer[i]);
        if(i % 30 == 29) printf("\n");
    }
    printf("\n");             
#endif         


    return true;
}  

#else

bool W25Q256_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{
#if W25Q256_DBGINFO_READ_ENABLE
    printf("%s\n", __func__);
#endif
    if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(W25X_FastReadData, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_4_LINES, W25Q256_Addr_MODE,
                                   ReadAddr/*address*/, 8/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, pBuffer/*rxData*/, NumByteToRead/*rxSize*/)) return false;
    }
    else{
        if(!W25Q256_GeneralCmd(W25X_FastReadData, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_1_LINE, QSPI_ADDRESS_1_LINE, W25Q256_Addr_MODE,
                                   ReadAddr/*address*/, 8/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, pBuffer/*rxData*/, NumByteToRead/*rxSize*/)) return false;
    }

    return true;
}

#endif

bool W25Q256_Quad_Read(uint8_t* pRxData,uint32_t ReadAddr,uint16_t rxSize)   
{
#if 1
    QSPI_CommandTypeDef s_command;
    s_command.Instruction       = W25X_QuadFastReadData;
    s_command.Address           = ReadAddr;
    s_command.DummyCycles       = 4;
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
    s_command.DataMode          = QSPI_DATA_4_LINES;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;
    s_command.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
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

#if W25Q256_DBGINFO_GENERALCMD_ENABLE
        printf("spi rx: ");
		 for(int i = 0; i < rxSize; i++){
		    printf("%02X ", pRxData[i]);
            if(i % 30 == 29) printf("\n");
        }
        printf("\n");             
#endif         
    }
#else
    if(!W25Q256_GeneralCmd(W25X_QuadFastReadData, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_32_BITS,
                               ReadAddr/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, pBuffer/*rxData*/, NumByteToRead/*rxSize*/)) return false;
#endif
    return true;
}  

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 

#if 0
bool W25Q256_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
	W25Q256_Write_Enable();					//写使能

	if(W25Q256_QPI_MODE)
    	QSPI_Send_CMD(W25X_PageProgram,WriteAddr,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,W25Q256_Addr_MODE,QSPI_DATA_4_LINES);	//QPI,页写指令,地址为WriteAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据
  	else
    	QSPI_Send_CMD(W25X_PageProgram,WriteAddr,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,W25Q256_Addr_MODE,QSPI_DATA_1_LINE);	//QPI,页写指令,地址为WriteAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据

    
	QSPI_Transmit(pBuffer,NumByteToWrite);	   

#if W25Q256_DBGINFO_GENERALCMD_ENABLE
    printf("spi tx: ");
	 for(int i = 0; i < NumByteToWrite; i++){
	    printf("%02X ", pBuffer[i]);
        if(i % 30 == 29) printf("\n");
    }
    printf("\n");             
#endif         
    
	W25Q256_Wait_Busy();					   //等待写入结束

    return true;
} 

bool W25Q256_Write_PageBase(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
	if(W25Q256_QPI_MODE)
    	QSPI_Send_CMD(W25X_PageProgram,WriteAddr,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,W25Q256_Addr_MODE,QSPI_DATA_4_LINES);	//QPI,页写指令,地址为WriteAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据
  	else
    	QSPI_Send_CMD(W25X_PageProgram,WriteAddr,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,W25Q256_Addr_MODE,QSPI_DATA_1_LINE);	//QPI,页写指令,地址为WriteAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据
        
	QSPI_Transmit(pBuffer,NumByteToWrite);	   
#if W25Q256_DBGINFO_GENERALCMD_ENABLE
    printf("spi tx: ");
	 for(int i = 0; i < NumByteToWrite; i++){
	    printf("%02X ", pBuffer[i]);
        if(i % 30 == 29) printf("\n");
    }
    printf("\n");             
#endif         
    
    return true;
} 

#else
bool W25Q256_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
#if W25Q256_DBGINFO_CMD_ENABLE
    printf("%s\n", __func__);
#endif
	W25Q256_Write_Enable();					//写使能

	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(W25X_PageProgram, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_4_LINES, W25Q256_Addr_MODE,
                                   WriteAddr/*address*/, 0/*dummyCycles*/, pBuffer/*txData*/, NumByteToWrite/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }
  	else{
        if(!W25Q256_GeneralCmd(W25X_PageProgram, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_1_LINE, QSPI_ADDRESS_1_LINE, W25Q256_Addr_MODE,
                                   WriteAddr/*address*/, 0/*dummyCycles*/, pBuffer/*txData*/, NumByteToWrite/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }
        
	W25Q256_Wait_Busy();					   //等待写入结束

    return true;
} 

bool W25Q256_Write_PageBase(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
#if W25Q256_DBGINFO_WRITE_ENABLE
    printf("%s\n", __func__);
#endif
	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(W25X_PageProgram, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_4_LINES, W25Q256_Addr_MODE,
                                   WriteAddr/*address*/, 0/*dummyCycles*/, pBuffer/*txData*/, NumByteToWrite/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }
  	else{
        if(!W25Q256_GeneralCmd(W25X_PageProgram, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_1_LINE, QSPI_ADDRESS_1_LINE, W25Q256_Addr_MODE,
                                   WriteAddr/*address*/, 0/*dummyCycles*/, pBuffer/*txData*/, NumByteToWrite/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }
    return true;
} 
#endif

bool W25Q256_Quad_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
	W25Q256_Write_Enable();					//写使能

    if(!W25Q256_GeneralCmd(W25X_QuadPageProgram, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_4_LINES, W25Q256_Addr_MODE,
                               WriteAddr/*address*/, 0/*dummyCycles*/, pBuffer/*txData*/, NumByteToWrite/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
        
	W25Q256_Wait_Busy();					   //等待写入结束

    return true;
} 

bool W25Q256_Quad_Write_PageBase(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    if(!W25Q256_GeneralCmd(W25X_QuadPageProgram, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_4_LINES, QSPI_ADDRESS_4_LINES, W25Q256_Addr_MODE,
                               WriteAddr/*address*/, 0/*dummyCycles*/, pBuffer/*txData*/, NumByteToWrite/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
        
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
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
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
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	}   

    return true;
} 

//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
uint8_t W25Q256_BUFFER[4096];		 
bool W25Q256_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
	uint8_t * W25Q256_BUF;	  
   	W25Q256_BUF=W25Q256_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		if(W25Q256_Read(W25Q256_BUF,secpos*4096,4096)) return false;//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25Q256_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			W25Q256_Erase_Sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				W25Q256_BUF[i+secoff]=pBuffer[i];	  
			}
			W25Q256_Write_NoCheck(W25Q256_BUF,secpos*4096,4096);//写入整个扇区  

		}else W25Q256_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};

	return true;
}

//擦除整个芯片		  
//等待时间超长...
void W25Q256_Erase_Chip(void)   
{                                   
    W25Q256_Write_Enable();					//SET WEL 
    W25Q256_Wait_Busy();   
    if(W25Q256_QPI_MODE){
        QSPI_Send_CMD(W25X_ChipErase,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,写全片擦除指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    }
    else{
        QSPI_Send_CMD(W25X_ChipErase,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,写全片擦除指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    }
    W25Q256_Wait_Busy();						//等待芯片擦除结束
} 

bool W25Q256_32KB_Block_Erase(uint32_t addr)   
{  	 
#if W25Q256_DBGINFO_CMD_ENABLE
    printf(infoMsg("%s\n"), __func__);
#endif

	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(W25X_32KB_BlockErase, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_NONE, QSPI_ADDRESS_4_LINES, W25Q256_Addr_MODE,
                                   addr/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }
  	else{
        if(!W25Q256_GeneralCmd(W25X_32KB_BlockErase, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_NONE, QSPI_ADDRESS_1_LINE, W25Q256_Addr_MODE,
                                   addr/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }

    return true;
}

//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个扇区的最少时间:150ms
void W25Q256_Erase_Sector(uint32_t sectorNo)   
{  
	 
 	//printf("fe:%x\r\n",Dst_Addr);			//监视falsh擦除情况,测试用  	  
 	uint32_t Dst_Addr = sectorNo * W25_oneSector;
    W25Q256_Write_Enable();                  //SET WEL 	 
    W25Q256_Wait_Busy();  
    if(W25Q256_QPI_MODE){
        QSPI_Send_CMD(W25X_SectorErase,Dst_Addr,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,W25Q256_Addr_MODE,QSPI_DATA_NONE);//QPI,写扇区擦除指令,地址为0,无数据_32位地址_4线传输地址_4线传输指令,无空周期,0个字节数据
    }
    else{
        QSPI_Send_CMD(W25X_SectorErase,Dst_Addr,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,W25Q256_Addr_MODE,QSPI_DATA_NONE);//QPI,写扇区擦除指令,地址为0,无数据_32位地址_4线传输地址_4线传输指令,无空周期,0个字节数据
    }
    W25Q256_Wait_Busy();   				    //等待擦除完成
}

//等待空闲
void W25Q256_Wait_Busy(void)   
{   
	while((W25Q256_ReadSR(1)&0x01)==0x01);   // 等待BUSY位清空
}   

bool W25Q256_WriteVolatile_Enable()
{

	if(W25Q256_QPI_MODE){
        if(!W25Q256_GeneralCmd(W25X_WriteVolatileStatusRegister, QSPI_INSTRUCTION_4_LINES, QSPI_DATA_NONE, QSPI_ADDRESS_NONE, W25Q256_Addr_MODE,
                                   0/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
	}
    else{
        if(!W25Q256_GeneralCmd(W25X_WriteVolatileStatusRegister, QSPI_INSTRUCTION_1_LINE, QSPI_DATA_NONE, QSPI_ADDRESS_NONE, W25Q256_Addr_MODE,
                                  0/*address*/, 0/*dummyCycles*/, 0/*txData*/, 0/*txSize*/, 0/*rxData*/, 0/*rxSize*/)) return false;
    }
    return true;   
}

bool W25Q256_XFER(uint8_t *pTxData, int txSize, uint8_t *pRxData, int rxSize)
{
//    HAL_StatusTypeDef rltStatus;
    uint8_t cmd = pTxData[0];
    uint8_t *txParaPtr = &pTxData[1];
    int txParaLen = txSize - 1;

    if(cmd == W25X_FastReadData){
#if W25Q256_DBGINFO_READ_ENABLE
        printf(infoMsg("command:0x%x\r\n"), cmd);
#endif
    }
    else if(cmd == W25X_PageProgram){
#if W25Q256_DBGINFO_WRITE_ENABLE
        printf(infoMsg("command:0x%x\r\n"), cmd);
#endif
    }
    else{
#if W25Q256_DBGINFO_CMD_ENABLE
        printf(infoMsg("command:0x%x\r\n"), cmd);
#endif
    }

#if 1
    if(cmd == W25X_FastReadData || cmd == W25X_QuadFastReadData){//fast read
        uint32_t address = (txParaPtr[0] << 16) + (txParaPtr[1] << 8) + (txParaPtr[2] << 0); 
#if W25Q256_DBGINFO_READ_ENABLE
        printf("addr=0x%x\n", (unsigned int)address);
#endif

#if PseudoFlash
        return true;
#else
        return W25Q256_Read(pRxData, address, rxSize);
#endif
    }
#if 0//fail to use the cmd
    else if(cmd == 0xeb){//fast read
        uint32_t address = (txParaPtr[0] << 16) + (txParaPtr[1] << 8) + (txParaPtr[2] << 0);
        address = (address << 8) | 0xFF;
#if W25Q256_DBGINFO_CMD_ENABLE
        printf("addr=0x%x\n", (unsigned int)address);
#endif
        return W25Q256_Quad_Read(pRxData, address, rxSize);
    }
#endif    
    else if(cmd == W25X_32KB_BlockErase){//32KB Block Erase
        uint32_t address = (txParaPtr[0] << 16) + (txParaPtr[1] << 8) + (txParaPtr[2] << 0); 
#if W25Q256_DBGINFO_CMD_ENABLE
        printf("addr=0x%x\n", (unsigned int)address);
#endif
        return W25Q256_32KB_Block_Erase(address);
    }
    else if(cmd == W25X_PageProgram || cmd == W25X_QuadPageProgram){//page program
        int addrSize = 3;
        uint32_t address = (txParaPtr[0] << 16) + (txParaPtr[1] << 8) + (txParaPtr[2] << 0); 
        uint32_t dataLen = txParaLen - addrSize/*address*/;
        uint8_t *dataPtr = txParaPtr + addrSize;
#if W25Q256_DBGINFO_WRITE_ENABLE
        printf("addr=0x%x, dataLen=%d\n", (unsigned int)address, (int)dataLen);
#endif    
        return W25Q256_Write_PageBase(dataPtr, (unsigned int)address, (int)dataLen);
    }
#if 0//fail to use the cmd
    else if(cmd == 0x32){//quad input page program
        int addrSize = 3;
        uint32_t address = (txParaPtr[0] << 16) + (txParaPtr[1] << 8) + (txParaPtr[2] << 0); 
        uint32_t dataLen = txParaLen - addrSize/*address*/;
        uint8_t *dataPtr = txParaPtr + addrSize;
#if W25Q256_DBGINFO_CMD_ENABLE
        printf("addr=0x%x, dataLen=%d\n", (unsigned int)address, (int)dataLen);
#endif    
        return W25Q256_Quad_Write_PageBase(dataPtr, (unsigned int)address, (int)dataLen);
    }
#endif    
#if PseudoFlash
    else if(cmd == W25X_JedecDeviceID){
        pRxData[0] = 0xEF;
        pRxData[1] = 0x40;
        pRxData[2] = 0x19;
        pRxData[3] = 0x0;
        return true;
    }
#endif    
    else{
    	if(W25Q256_QPI_MODE){
            if(!W25Q256_GeneralCmd(cmd, QSPI_INSTRUCTION_4_LINES, ((txParaLen == 0 && rxSize == 0)? QSPI_DATA_NONE: QSPI_DATA_4_LINES), QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                       0/*address*/, 0/*dummyCycles*/, txParaPtr/*txData*/, txParaLen/*txSize*/, pRxData/*rxData*/, rxSize/*rxSize*/)) return false;
    	}
        else{
            if(!W25Q256_GeneralCmd(cmd, QSPI_INSTRUCTION_1_LINE, ((txParaLen == 0 && rxSize == 0)? QSPI_DATA_NONE: QSPI_DATA_1_LINE), QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS,
                                      0/*address*/, 0/*dummyCycles*/, txParaPtr/*txData*/, txParaLen/*txSize*/, pRxData/*rxData*/, rxSize/*rxSize*/)) return false;
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

bool W25Q256_Enable()
{
    HAL_GPIO_WritePin(SpiFlashEnable_GPIO_Port, SpiFlashEnable_Pin, GPIO_PIN_SET);
    W25Q256_Qspi_Enable();    

//    uint8_t temp = 0;
//	if(W25Q256_QPI_MODE){		
//		QSPI_Send_CMD(W25X_SetReadParam,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES); 		//QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
//    }
//    else{
//		QSPI_Send_CMD(W25X_SetReadParam,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE); 		//QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
//    }
//	temp=3<<4;					//设置P4&P5=11,8个dummy clocks,104M
//	QSPI_Transmit(&temp,1);		//发送1个字节


    return true;
}

bool W25Q256_Disable()
{
    HAL_GPIO_WritePin(SpiFlashEnable_GPIO_Port, SpiFlashEnable_Pin, GPIO_PIN_RESET);
    W25Q256_Qspi_Disable();    
    return true;
}

void W25Q256_QspiDriver_SetPara(uint32_t speed, uint32_t clockMode)
{
    /*
        speed = 160,000,000 / (ClockPrescaler + 1);
    */
    uint32_t spiClock = 160000000;
    uint32_t clockPrescaler = ((spiClock + (speed - 1)) / speed) - 1;
    printf("prefer speed=%u => physical speed=%u, clockPrescaler=%u, clockMode=%u\n", (unsigned int)speed, (unsigned int)(spiClock / (clockPrescaler + 1) ), (unsigned int)clockPrescaler, (unsigned int)clockMode);
    setQspiPara(clockPrescaler, clockMode);
}

