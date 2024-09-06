/*
 * zigbee_sbl.h
 *
 *  Created on: Sep 20, 2019
 *      Author: govind.mukundan
 */

#ifndef SOURCE_ZIGBEE_SBL_H_
#define SOURCE_ZIGBEE_SBL_H_
#include <stdint.h>

#ifndef BV
#define BV(n)      (1 << (n))
#endif

#ifndef BF
#define BF(x,b,s)  (((x) & (b)) >> (s))
#endif

#ifndef MIN
#define MIN(n,m)   (((n) < (m)) ? (n) : (m))
#endif

#ifndef MAX
#define MAX(n,m)   (((n) < (m)) ? (m) : (n))
#endif

#ifndef ABS
#define ABS(n)     (((n) < 0) ? -(n) : (n))
#endif

#define IsNULL(x)				(((x) == (NULL)))

/* takes a byte out of a uint32_t : var - uint32_t,  ByteNum - byte to take out (0 - 3) */
#define BREAK_UINT32( var, ByteNum ) \
          (uint8)((uint32_t)(((var) >>((ByteNum) * 8)) & 0x00FF))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
          ((uint32_t)((uint32_t)((Byte0) & 0x00FF) \
          + ((uint32_t)((Byte1) & 0x00FF) << 8) \
          + ((uint32_t)((Byte2) & 0x00FF) << 16) \
          + ((uint32_t)((Byte3) & 0x00FF) << 24)))

#define BUILD_UINT16(loByte, hiByte) \
          ((uint16)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define BUILD_UINT8(hiByte, loByte) \
          ((uint8)(((loByte) & 0x0F) + (((hiByte) & 0x0F) << 4)))

#define HI_UINT8(a) (((a) >> 4) & 0x0F)
#define LO_UINT8(a) ((a) & 0x0F)

// Write the 32bit value of 'val' in little endian format to the buffer pointed
// to by pBuf, and increment pBuf by 4
#define UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,val) \
	do { \
		*(pBuf)++ = ((((uint32_t)(val)) >>  0) & 0xFF); \
		*(pBuf)++ = ((((uint32_t)(val)) >>  8) & 0xFF); \
		*(pBuf)++ = ((((uint32_t)(val)) >> 16) & 0xFF); \
		*(pBuf)++ = ((((uint32_t)(val)) >> 24) & 0xFF); \
	} while (0)

// Return the 32bit little-endian formatted value pointed to by pBuf, and increment pBuf by 4
#define BUF_TO_UINT32_LITTLE_ENDIAN(pBuf) (((pBuf) += 4), BUILD_UINT32((pBuf)[-4], (pBuf)[-3], (pBuf)[-2], (pBuf)[-1]))

#ifndef GET_BIT
#define GET_BIT(DISCS, IDX)  (((DISCS)[((IDX) / 8)] & BV((IDX) % 8)) ? TRUE : FALSE)
#endif
#ifndef SET_BIT
#define SET_BIT(DISCS, IDX)  (((DISCS)[((IDX) / 8)] |= BV((IDX) % 8)))
#endif
#ifndef CLR_BIT
#define CLR_BIT(DISCS, IDX)  (((DISCS)[((IDX) / 8)] &= (BV((IDX) % 8) ^ 0xFF)))
#endif

  typedef struct __attribute__((__packed__))
  {
	 uint8_t status;
	 uint32_t bootloaderRev;
	 uint8_t deviceType;       // 0x01 – CC2538
	 uint32_t maxDataLen;     // The maximum data size to use with Read / Write command
	 uint32_t pageSize;       // 0x800 – CC2538 flash page size & others are 0x20
  }rx_handshake;

  typedef struct __attribute__((__packed__))
  {
	 uint32_t firstAddress;
	 uint32_t opLength;
	 uint8_t data[128];
  }write_payload;

  typedef struct __attribute__((__packed__)){
	  uint8_t transportRev;
	  uint8_t productRev;
	  uint8_t majorRev;
	  uint8_t minorRev;
	  uint8_t hwRev;
	  uint32_t swRev;
  }mt_revision;


  typedef void (rx_callback) (uint8_t datum, void* context);

#define SBL_BOOT_DELAY_US 					(uint32_t)(1*1000*1000)

  /* This time must be LESS THAN the SBL boot time, otherwise the firmware will enter
   * applicatiom mode before we start talking to it */
#define SBL_HARD_RESET_USB_RESTART_TIME		(uint32_t)(1*1000*1000)

#define SBL_DELAY_1S_US						(uint32_t)(1000*1000)
#define DEFAULT_ZIGBEE_DEVICE 				"/dev/zigbee"

#define SBL_HARD_RESET_CMD_PIN_LOW	  "sudo raspi-gpio set 33 op pu dl"
#define SBL_HARD_RESET_CMD_PIN_HIGH	  "sudo raspi-gpio set 33 op pu dh"


int execute_fw_update(char* file, char* device);
int execute_read_sys_vsn(char* device, mt_revision* pmtv);
int execute_soft_reset(char* device);

#endif /* SOURCE_ZIGBEE_SBL_H_ */
