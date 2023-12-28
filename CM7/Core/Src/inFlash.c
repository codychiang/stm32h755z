/*
 * inFlash.c
 *
 *  Created on: Dec 26, 2023
 *      Author: cody.chiang
 */

#include "stm32h7xx_hal.h"

#if 1

/* FLASH大小：STM32F103VET6:256K */
#define STM32FLASH_SIZE         0x100000
/* FLASH起始地址 */
#define STM32FLASH_BANK         FLASH_BANK_1
#define STM32FLASH_BASE         FLASH_BANK1_BASE
/* FLASH结束地址 */
#define STM32FLASH_END          (STM32FLASH_BASE | STM32FLASH_SIZE)
/* FLASH页大小：128K */
#define STM32FLASH_PAGE_SIZE    FLASH_SECTOR_SIZE
/* FLASH总页数 */
#define STM32FLASH_PAGE_NUM     (STM32FLASH_SIZE / STM32FLASH_PAGE_SIZE)

#define STM32FLASH_WRITE_NUM    (FLASH_NB_32BITWORD_IN_FLASHWORD)

#define WRITE_START_ADDR        ((uint32_t)0x8000000)
#define WRITE_END_ADDR          ((uint32_t)0x80fffff)

static uint32_t FlashBuffer[STM32FLASH_PAGE_SIZE / sizeof(uint32_t)];

/**
 @brief 内部Flash读取
 @param address -[in] 读取的地址
 @param pData -[out] 指向需要操作的数据
 @param dataLen -[in] 数据长度
 @return 读出成功的字节数
*/
uint32_t Internal_ReadFlash(uint32_t addrStart, uint32_t *pData, uint32_t dataLen)
{
    uint32_t nread = dataLen;
    uint8_t *pBuffer = (uint8_t *)pData;
    const uint8_t *pAddr = (const uint8_t *)addrStart;

    if(!pData || addrStart < STM32FLASH_BASE || addrStart > STM32FLASH_END)
    {
        return 0;
    }

    while(nread >= 1 && (((uint32_t)pAddr) <= (STM32FLASH_END - 4)))
    {
        *(uint32_t *)pBuffer = *(uint32_t *)pAddr;
        pBuffer += sizeof(uint32_t);
        pAddr += sizeof(uint32_t);
        nread--;
    }

    while(nread && (((uint32_t)pAddr) < STM32FLASH_END))
    {
        *pBuffer++ = *pAddr++;
        nread--;
    }

    return dataLen - nread;
}

/**
 @brief 内部Flash无检查写入
 @param address -[in] 写入的地址
 @param pData -[in] 指向需要操作的数据
 @param dataLen -[in] 数据长度
 @return 实际写入的数据量，单位：字节
*/
uint32_t Internal_WriteFlashNoCheck(uint32_t addrStart, const uint32_t *pData, uint32_t dataLen)
{
    uint32_t nwrite = dataLen;
    uint32_t addrmax = STM32FLASH_END - 4;

    while(nwrite)
    {
        if(addrStart > addrmax)
        {
            break;
        }

        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addrStart, (uint32_t)pData);
        if((*(__IO uint32_t*) addrStart) != *pData)
        {
            break;
        }

        nwrite-=STM32FLASH_WRITE_NUM;
        pData+=STM32FLASH_WRITE_NUM;
        addrStart += sizeof(uint32_t) * STM32FLASH_WRITE_NUM;
    }
    return (dataLen - nwrite);
}

/**
 @brief 内部Flash写入
 @param address -[in] 写入的地址
 @param pData -[in] 指向需要操作的数据
 @param dataLen -[in] 数据长度
 @return 实际写入的数据量，单位：字节
*/
uint32_t Internal_WriteFlash(uint32_t addrStart, const uint32_t *pData, uint32_t dataLen)
{
    uint32_t i = 0;
    uint32_t pagepos = 0;         // 页位置
    uint32_t pageoff = 0;         // 页内偏移地址
    uint32_t pagefre = 0;         // 页内空余空间
    uint32_t offset = 0;          // Address在FLASH中的偏移
    uint32_t nwrite = dataLen;    // 记录剩余要写入的数据量
    const uint32_t *pBuffer = (const uint32_t *)pData;

    /* 非法地址 */
    if(addrStart < STM32FLASH_BASE || addrStart > (STM32FLASH_END - 4) || dataLen == 0 || pData == NULL)
    {
        return 0;
    }

    /* 解锁FLASH */
    HAL_FLASH_Unlock();

    /* 计算偏移地址 */
    offset = addrStart - STM32FLASH_BASE;
    /* 计算当前页位置 */
    pagepos = offset / STM32FLASH_PAGE_SIZE;
    /* 计算要写数据的起始地址在当前页内的偏移地址 */
    pageoff = ((offset % STM32FLASH_PAGE_SIZE) / sizeof(uint32_t));
    /* 计算当前页内空余空间 */
    pagefre = ((STM32FLASH_PAGE_SIZE / sizeof(uint32_t)) - pageoff);
    /* 要写入的数据量低于当前页空余量 */
    if(nwrite <= pagefre)
    {
        pagefre = nwrite;
    }

    while(nwrite != 0)
    {
        /* 检查是否超页 */
        if(pagepos >= STM32FLASH_PAGE_NUM)
        {
            break;
        }

        /* 读取一页 */
        Internal_ReadFlash(STM32FLASH_BASE + pagepos * STM32FLASH_PAGE_SIZE, FlashBuffer, (STM32FLASH_PAGE_SIZE / sizeof(uint32_t)));

        /* 检查是否需要擦除 */
        for(i = 0; i < pagefre; i++)
        {
            if(*(FlashBuffer + pageoff + i) != 0xFFFFFFFF) /* FLASH擦出后默认内容全为0xFF */
            {
                break;
            }
        }

        if(i < pagefre)
        {
            uint32_t count = 0;
            uint32_t index = 0;
            uint32_t PageError = 0;
            FLASH_EraseInitTypeDef pEraseInit;

            /* 擦除一页 */
            pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
            pEraseInit.Banks = STM32FLASH_BANK;
            pEraseInit.Sector = pagepos;
            pEraseInit.NbSectors = 1;
            if(HAL_FLASHEx_Erase(&pEraseInit, &PageError) != HAL_OK)
            {
                break;
            }

            /* 复制到缓存 */
            for(index = 0; index < pagefre; index++)
            {
                *(FlashBuffer + pageoff + index) = *(pBuffer + index);
            }

            /* 写回FLASH */
            count = Internal_WriteFlashNoCheck(STM32FLASH_BASE + pagepos * STM32FLASH_PAGE_SIZE, FlashBuffer, (STM32FLASH_PAGE_SIZE / sizeof(uint32_t)));
            if(count != (STM32FLASH_PAGE_SIZE / sizeof(uint32_t)))
            {
                nwrite -= count;
                break;
            }
        }
        else
        {
            /* 无需擦除，直接写 */
            uint32_t count = Internal_WriteFlashNoCheck(addrStart, pBuffer, pagefre);
            if(count != pagefre)
            {
                nwrite -= count;
                break;
            }
        }

        pBuffer += pagefre;         /* 读取地址递增         */
        addrStart += (pagefre << 1);  /* 写入地址递增         */
        nwrite -= pagefre;          /* 更新剩余未写入数据量 */

        pagepos++;     /* 下一页           */
        pageoff = 0;   /* 页内偏移地址置零  */

        /* 根据剩余量计算下次写入数据量 */
        pagefre = nwrite >= (STM32FLASH_PAGE_SIZE >> 1) ? (STM32FLASH_PAGE_SIZE >> 1) : nwrite;
    }

    /* 加锁FLASH */
    HAL_FLASH_Lock();

    return ((dataLen - nwrite) << 1);
}

/**
 @brief 内部Flash页擦除
 @param pageAddress -[in] 擦除的起始地址
 @param nbPages -[in] 擦除页数
 @return 0 - 成功；-1 - 失败
*/
int Internal_ErasePage(uint32_t pageAddress, uint32_t nbPages)
{
	uint32_t pageError = 0;
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	eraseInit.Sector = pageAddress / STM32FLASH_PAGE_SIZE;
	eraseInit.Banks = STM32FLASH_BANK;
	eraseInit.NbSectors = nbPages;
	if(HAL_FLASHEx_Erase(&eraseInit, &pageError) != HAL_OK)
	{
		return -1;
	}
	return 0;
}
#endif
