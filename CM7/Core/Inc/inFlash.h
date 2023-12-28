/*
 * inFlash.h
 *
 *  Created on: Dec 26, 2023
 *      Author: cody.chiang
 */

#ifndef SRC_INFLASH_H_
#define SRC_INFLASH_H_

uint32_t Internal_ReadFlash(uint32_t addrStart, uint32_t *pData, uint32_t dataLen);
uint32_t Internal_WriteFlash(uint32_t addrStart, const uint32_t *pData, uint32_t dataLen);;

#endif /* SRC_INFLASH_H_ */
