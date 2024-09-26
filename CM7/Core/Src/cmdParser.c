/*
 * cmdParser.c
 *
 *  Created on: Sep 13, 2024
 *      Author: cody.chiang
 */

#include "cmdParser.h"
#include "w25q256.h"
#include "memory.h"

uint8_t checkSum(uint8_t *data, int size)
{
    uint8_t sum = 0;
    for(int i = 0; i < size; i++){
        sum += data[i];
    }
    sum = 0x100 - sum;

    return sum;
}

bool checkProtocol(uint8_t *pCmd, int cmdSize)
{
    if(cmdSize < 8) return false;

    uint8_t header = pCmd[0];
    //printf("header=0x%x", header);
    if(header != 0xAB) return false;

    uint16_t len = ((uint16_t)pCmd[3] << 8) | (uint16_t)pCmd[2];
    //printf("len=0x%x", len);
    if(cmdSize != (len + 6)) return false;
    
    //uint16_t cmd = ((uint16_t)pCmd[5] << 8) | (uint16_t)pCmd[4];
    //printf("cmd=0x%x", cmd);
    
//    int dataLen = (len - 2);
//    uint8_t *pData = &pCmd[6];

    int checkSumPos = (len + 4);
    uint8_t getSum = pCmd[checkSumPos];
    if(checkSum(pCmd, checkSumPos) != getSum){
        return false;
    }

    return true;
}


#define spiRcvData_maxSize (2100)
volatile uint8_t spiRcvData[spiRcvData_maxSize];

#define tcp_snd_cmd_maxSize (2000 + 100)
volatile uint8_t tcp_snd_cmd[tcp_snd_cmd_maxSize];
int tcp_snd_cmd_idx = 0;
bool tcpEncodeRspData(uint8_t tag, uint16_t errorCode, uint8_t *pCmd, uint8_t *pData, int dataSize, uint8_t **retData, int *retSize)
{
    uint16_t len = dataSize + 4;
    uint16_t cmdId = 0xFFFF;
    uint8_t ED = 0;

    pCmd[0] = 0xAB;
    pCmd[1] = tag;
    pCmd[2] = (uint8_t)len;
    pCmd[3] = (uint8_t)(len >> 8);
    pCmd[4] = (uint8_t)cmdId;
    pCmd[5] = (uint8_t)(cmdId >> 8);
    pCmd[6] = (uint8_t)errorCode;
    pCmd[7] = (uint8_t)(errorCode >> 8);

    int dataPos = 8;
    if( (&pCmd[dataPos] != pData) && (dataSize > 0)){
        memcpy(&pCmd[dataPos], pData, dataSize);
    }
    int cmdLen = dataSize + dataPos;
    uint8_t sum = checkSum(pCmd, cmdLen);
    pCmd[cmdLen++] = sum;
    pCmd[cmdLen++] = ED;

    *retData = pCmd;
    *retSize = cmdLen;
    return true;
}

bool tcpCmdParser(uint8_t *pCmd, int cmdSize, uint8_t **retData, int *retSize)
{
    uint16_t cmd = ((uint16_t)pCmd[5] << 8) | (uint16_t)pCmd[4];
    uint8_t *data = &pCmd[6];
    int dataSize = cmdSize - 6;
    uint8_t tag = pCmd[1];
    bool rlt = true;    

    switch(cmd){
        case SXCMD_SPI_OPTIONS:{//__u32 param[]={ channel, clock, mode };
            struct sSpiOption_t{
                int channel;
                int clock;
                int mode;
            }sSpiOption;
            memcpy(&sSpiOption, data, sizeof(sSpiOption));
            printf("SPI_OPTIONS=>channel=%d, clock=%d, mode=%d\n", sSpiOption.channel, sSpiOption.clock, sSpiOption.mode);
            W25Q256_QspiDriver_SetPara(sSpiOption.clock, sSpiOption.mode);
            
            tcpEncodeRspData(tag, CMD_ERR_OK, tcp_snd_cmd, 0, 0, retData, retSize);
        }
            break;
        case SXCMD_SPI_ENABLE:{
            printf("SPI_ENABLE\n"); 
            W25Q256_Enable();
            tcpEncodeRspData(tag, CMD_ERR_OK, tcp_snd_cmd, 0, 0, retData, retSize);
        }
            break;
        case SXCMD_SPI_DISABLE:{
            printf("SPI_DISABLE\n");
            W25Q256_Disable();
            tcpEncodeRspData(tag, CMD_ERR_OK, tcp_snd_cmd, 0, 0, retData, retSize);
        }
            break;
        case SXCMD_SPI_XFER:{//__u32 param[]={ (__u32)txLen, (__u32)rxLen };
            struct sSpiXfer_t{
                int txLen;
                int rxLen;                
            }sSpiXfer;
            memcpy(&sSpiXfer, data, sizeof(sSpiXfer));
            uint8_t *txData = &data[sizeof(sSpiXfer)];
            //printf("SPI_XFER=>txLen=%d, rxLen=%d\n", sSpiXfer.txLen, sSpiXfer.rxLen);
            uint8_t *rxData = &tcp_snd_cmd[8];
            
            if(W25Q256_XFER(txData, sSpiXfer.txLen, rxData, sSpiXfer.rxLen)){                 
                tcpEncodeRspData(tag, CMD_ERR_OK, tcp_snd_cmd, rxData, sSpiXfer.rxLen, retData, retSize);
            }
            else{
                tcpEncodeRspData(tag, CMD_ERR_FAIL, tcp_snd_cmd, 0, 0, retData, retSize);
            }
        }
            break;
        default:
        	tcpEncodeRspData(tag, CMD_ERR_NOTSUPPORT, tcp_snd_cmd, 0, 0, retData, retSize);
            break;
    }

    
    return rlt;
}



