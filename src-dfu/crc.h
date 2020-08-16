/**********************************************************************
 *
 * Filename:    crc.h
 *
 * Description: A header file describing the various CRC standards.
 *
 * Notes:
 *
 *
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#ifndef _crc_h
#define _crc_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define FALSE 0
#define TRUE !FALSE

//! Select the CRC standard from the list that follows. CRC_CCITT, CRC16, or CRC32
#define CRC32

#if defined(CRC_CCITT)

typedef uint16_t crc;

#define CRC_NAME "CRC-CCITT"
#define POLYNOMIAL 0x1021u
#define INITIAL_REMAINDER 0xFFFFu
#define FINAL_XOR_VALUE 0x0000u
#define REFLECT_DATA FALSE
#define REFLECT_REMAINDER FALSE
#define CHECK_VALUE 0x29B1u

#elif defined(CRC16)

typedef uint16_t crc;

#define CRC_NAME "CRC-16"
#define POLYNOMIAL 0x8005u
#define INITIAL_REMAINDER 0x0000u
#define FINAL_XOR_VALUE 0x0000u
#define REFLECT_DATA TRUE
#define REFLECT_REMAINDER TRUE
#define CHECK_VALUE 0xBB3Du

#elif defined(CRC32)

typedef uint32_t crc;

#define CRC_NAME "CRC-32"
#define POLYNOMIAL 0x04C11DB7u
#define INITIAL_REMAINDER 0xFFFFFFFFu
#define FINAL_XOR_VALUE 0xFFFFFFFFu
#define REFLECT_DATA TRUE
#define REFLECT_REMAINDER TRUE
#define CHECK_VALUE 0xCBF43926u

#else

#error "One of CRC_CCITT, CRC16, or CRC32 must be #define'd."

#endif

/**
 * crcInit
 *
 * Populate the partial CRC lookup table.
 *
 * This function must be rerun any time the CRC standard is changed.
 * If desired, it can be run "offline" and the table results stored
 * in an embedded system's memory.
 *
 */
void crcInit(void);

/**
 * crcSlow
 *
 * Compute the CRC of a given message.
 * Calculates the CRC without the use of a CRC table, crcInit() doesn't have to be called!
 * (Slower performance)
 *
 * @param message[]: message/data for which the CRC will be calculated
 * @param nBytes: number of bytes in the message/data
 * @return crc: The CRC of the message.
 */
crc crcSlow(unsigned char const message[], size_t nBytes);

/**
 * crcFast
 *
 * Compute the CRC of a given message.
 * IMPORTANT:: crcInit() must be called first to use crcFast!
 * (Faster performance)
 *
 * @param message[]: message/data for which the CRC will be calculated
 * @param nBytes: number of bytes in the message/data
 * @return crc: The CRC of the message.
 */
crc crcFast(unsigned char const message[], size_t nBytes);

#ifdef __cplusplus
}
#endif

#endif /* _crc_h */