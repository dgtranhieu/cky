#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#define __USE_BSD
#include <unistd.h>

/* Linux includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sched.h>
#include <linux/serial.h>       /* for struct serial_struct */
#include <math.h>       /* for calculation of custom divisor */
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include "zigbee_sbl_uart.h"
#include "zigbee_sbl.h"
#include "Logger.h"

/* SmartLiving_RAS/zigbee_gw/tools/gw_soc_fw_version_query.bin /dev/zigbee
 *
 * 		sudo raspi-gpio set 33 op pu dl
 sleep 1
 sudo raspi-gpio set 33 op pu dh
 sleep 5

 RX - 19 - FE:0E:4D:84:00:01:00:00:00:01:00:08:00:00:00:08:00:00:C7:

[3309.41]zsbl Soft Reset successful - entered SBL mode!

RX - 19 - FE:0E:4D:84:00:01:00:00:00:01:00:08:00:00:00:08:00:00:C7:
[3309.44]zsbl BootRev:1
DevType=1
MaxBuffer=2048
PageSize=2048

[3309.44]zsbl Handshake OK

RX - 6 - FE:01:4D:81:00:CD:
RX - 6 - FE:01:4D:81:00:CD:
RX - 6 - FE:01:4D:81:00:CD:
RX - 6 - FE:01:4D:81:00:CD:
RX - 6 - FE:01:4D:81:00:CD:
RX - 6 - FE:01:4D:81:00:CD:
RX - 6 - FE:01:4D:81:00:CD:

 */

#define SB_CC2538_FLASH_START_ADDR  		(0x00200000u)
// Last address before the serial bootloader, NV and lock bits pages
#define SB_CC2538_CODE_FLASH_END_ADDRESS 	(0x0027AFFF)

#define SB_DEFAULT_RW_BUF_LEN                   64
#define SB_DEVICE_TYPE_UNKNOWN                  0
#define SB_WRITE_CMD                            0x01
#define SB_READ_CMD                             0x02
#define SB_ENABLE_CMD                           0x03
#define SB_HANDSHAKE_CMD                        0x04
#define SB_VERIFICATION_IND                     0x05
#define SB_SOF_CHAR					            0xFE
#define SB_FRAME_ID_CHAR			            0x4D
#define SB_EX_LEN_CHAR				            0xFF
#define MT_SYS_RESET_REQ                        0x00
#define MT_SYS_VERSION_REQ                      0x02

// #define SBL_SUCCESS                             0
// #define SBL_INTERNAL_ERROR                      1
// #define SBL_BUSY                                2
// #define SBL_OUT_OF_MEMORY                       3
// #define SBL_PENDING                             4
// #define SBL_ABORTED_BY_USER                     5
// #define SBL_NO_ACTIVE_DOWNLOAD                  6
// #define SBL_ERROR_OPENING_FILE                  7
// #define SBL_TARGET_WRITE_FAILED                 8
// #define SBL_EXECUTION_FAILED                    9
// #define SBL_HANDSHAKE_FAILED                    10
// #define SBL_LOCAL_READ_FAILED                   11
// #define SBL_VERIFICATION_FAILED                 12
// #define SBL_COMMUNICATION_FAILED                13
// #define SBL_ABORTED_BY_ANOTHER_USER             14
// #define SBL_REMOTE_ABORTED_BY_USER              15
// #define SBL_TARGET_READ_FAILED                  16
// #define SBL_SWITCH_BAUDRATE_FAILED              17
// #define SBL_SWITCH_BAUDRATE_CONFIRMATION_FAILED 18

#define ZBSOC_SBL_MAX_FRAME_SIZE    255u
#define DEFAULT_WRITE_SIZE		    (128u) // multiple of 32 and less than 255
#define TIMEOUT_ERROR			    25u
#define SB_RPC_CMD_AREQ             0x40
#define SB_SYS_RESP_MASK            0x60
#define RX_DELAY                    (1000u)
#define TIMEOUT_DEFAULT_MS		    (1000u)
#define TIMEOUT_WRITE_FLASH_MS		(2000u)

typedef enum
{
    MT_RPC_SYS_RES0, /* Reserved. */
    MT_RPC_SYS_SYS,
    MT_RPC_SYS_MAC,
    MT_RPC_SYS_NWK,
    MT_RPC_SYS_AF,
    MT_RPC_SYS_ZDO,
    MT_RPC_SYS_SAPI, /* Simple API. */
    MT_RPC_SYS_UTIL,
    MT_RPC_SYS_DBG,
    MT_RPC_SYS_APP,
    MT_RPC_SYS_OTA,
    MT_RPC_SYS_ZNP,
    MT_RPC_SYS_SPARE_12,
    MT_RPC_SYS_SBL = 13, // 13 to be compatible with existing RemoTI. // AKA MT_RPC_SYS_UBL
    MT_RPC_SYS_MAX // Maximum value, must be last (so 14-32 available, not yet assigned).
} mtRpcSysType_t;

typedef enum
{
    Init,
    ResetModule,
    Handshake,
    Write,
    Verify,
    Enable,
    VerifyAppEntry,
    Error,
    Done
} e_sbl_states;

typedef enum
{
    SOF, Length, FID, CMD, ExLength, Payload, FCS, MaxFrameElements
} e_frame_elements;

typedef enum
{
    SBLFrame, MTFrame, MaxFrameTypes
} e_frame_types;

// This is an internal context maintained by the parser, caller does not have to mess with it
typedef struct
{
    e_frame_elements state;
    e_frame_types type;
    uint32_t len;
    bool exLen;
    uint8_t respId;
    uint8_t fcs;
    uint8_t* payload; // Pointer to payload
    uint32_t payloadMaxSiz; // Size of above buffer
    uint32_t payloadCurSiz;
} s_resp_context;



/**-------------------------------------------------------------------------------------------------
 * [description]
 *------------------------------------------------------------------------------------------------*/

// timer_t g_timer;
e_sbl_states g_sbl_state = Init;
e_sbl_states g_last_state_bef_error = Init;
uint32_t g_start_address = 0;
uint8_t g_firmware_buffer[512 * 1024];	// A copy of firmware file to write
size_t g_firmware_buffer_size; // Stores the size of firmware file to write in bytes
uint8_t g_firmware_read_buffer[512 * 1024];	// The firmware read from module
size_t g_read_size;
char* g_zigbee_device;
mt_revision g_current_revision;

uint8_t g_all_ones_512[512] = { 0xFF };

static void calcFcs(uint8_t *msg, int size);
static int verify_mt_response(uint8_t cmd, int timeout_ms);
static int mt_send_soft_reset(void);
static int mt_send_get_system_vsn(void);
int execute_read_sys_vsn(char* device, mt_revision* pmtv);
static int read_file2buffer(char* fname, char* buf, size_t* size);
static void get_timestamp(struct timespec* pts);
static bool is_time_expired(struct timespec* ts2, uint32_t timeout_ms);
int parse_handshake(uint8_t* pdata, int len);
static int parse_resp_frame(uint8_t datum, s_resp_context* pr);
static int verify_response(uint8_t cmd, uint8_t* pbuff, size_t rlen, int timeout_ms);
static int send_handshake(void);
static int send_enable(void);
static int send_write(uint8_t* buf, uint8_t size, int delay_ms);
static int send_read(uint32_t address, uint8_t* buf, size_t* plen);
static int check_ff(uint8_t* start, size_t siz);
static int build_next_write_payload(uint32_t address, uint8_t* pin, size_t remaining_bytes, uint8_t* pframe, uint8_t* pff_packet);
static void sbl_init(void);
int execute_fw_update(char* file, char* device);

// Exports
static int execute_sbl(char* in_file, char* device);
int execute_soft_reset(char* device);
static void module_hard_reset(void);




/**-------------------------------------------------------------------------------------------------
 * MT
 *------------------------------------------------------------------------------------------------*/

static void calcFcs(uint8_t *msg, int size)
{
    uint8_t result = 0;
    int idx = 1; //skip SOF
    int len = (size - 2);  // skip FCS

    while ((len--) != 0)
    {
        result ^= msg[idx++];
    }

    msg[(size - 1)] = result;
}

static int verify_mt_response(uint8_t cmd, int timeout_ms)
{
    uint8_t rx[2048] = { 0 };
    int rx_len = 0;
    // Setup response context
    uint8_t payload[512] = { 0 };
    s_resp_context r = { 0 };
    r.type = MTFrame;
    r.payload = payload;
    r.payloadMaxSiz = sizeof(payload);

    uint8_t exit = TIMEOUT_ERROR; // timeout error

    struct timespec ts;
    get_timestamp(&ts);

    while (exit > 0 )
    {
        if(is_time_expired(&ts, timeout_ms))
        {
            exit = TIMEOUT_ERROR;
            break;
        }

        if (zsbl_uart_read(rx, &rx_len, RX_DELAY) != 0)
        {
            exit = -1;
            break;
        }

        for (int i = 0; i < rx_len; i++)
        {
            int parse = parse_resp_frame(rx[i], &r);
            if (parse <= 0)
            {
                exit = parse;
                break;
            }
        }
    }

    if (exit < 0)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Error parsing - exit", __FUNCTION__);
        return -1;
    }

    if (exit == TIMEOUT_ERROR)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Timeout error reading - exit", __FUNCTION__);
        return -1;
    }

    if (cmd != r.respId)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Unexpected response expected %02X, rxed %02X- exit", __FUNCTION__, cmd, r.respId);
        return -1;
    }

    switch (cmd)
    {
        case MT_SYS_VERSION_REQ:
            memcpy(&g_current_revision.transportRev, &r.payload[0], sizeof(g_current_revision));
            Logger.writeLog(LOG_WARN, "Platform: %s - TransportRev = %d", __FUNCTION__, r.payload[0]);
            Logger.writeLog(LOG_WARN, "Platform: %s - ProductRev   = %d", __FUNCTION__, r.payload[1]);
            Logger.writeLog(LOG_WARN, "Platform: %s - MajorRev     = %d", __FUNCTION__, r.payload[2]);
            Logger.writeLog(LOG_WARN, "Platform: %s - MinorRev     = %d", __FUNCTION__, r.payload[3]);
            Logger.writeLog(LOG_WARN, "Platform: %s - HWRev        = %d", __FUNCTION__, r.payload[4]);

            uint32_t sw_rev = 0;
            if (r.payloadCurSiz > 5)
            {
                sw_rev = r.payload[5] + (r.payload[6] << 8) + (r.payload[7] << 16) + (r.payload[8] << 24);
                // g_current_revision.swRev = sw_rev;
            }

            Logger.writeLog(LOG_WARN, "Platform: %s - Software Revision: %u", __FUNCTION__, sw_rev);
            if (r.payloadCurSiz <= 5)
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - Revision not specified", __FUNCTION__);
            }
            break;

        default:
            Logger.writeLog(LOG_ERROR, "Platform: %s - Unknown Command %d", __FUNCTION__, cmd);
            return -1;
            break;
    }

    return 0;
}

int zbSocSblSendMtFrame(uint8_t cmd, uint8_t * payload, uint8_t payload_len)
{
    uint8_t buf[ZBSOC_SBL_MAX_FRAME_SIZE];

    buf[0] = 0xFE;
    buf[1] = payload_len;
    buf[2] = SB_RPC_CMD_AREQ | MT_RPC_SYS_SBL; // == 0x4D
    buf[3] = cmd;

    if (payload_len > 0)
    {
        memcpy(buf + 4, payload, payload_len);
    }
    calcFcs(buf, payload_len + 5);
    zsbl_uart_flush();
    zsbl_uart_write(buf, payload_len + 5);

    return 0;
}

static int mt_send_soft_reset(void)
{
    int result;
    uint8_t cmd[] = {
        0xFE, 1,   //RPC payload Len
        SB_RPC_CMD_AREQ | MT_RPC_SYS_SYS,
        MT_SYS_RESET_REQ, 0x01,   //activate serial bootloader
        0x00   //FCS - fill in later
    };

    calcFcs(cmd, sizeof(cmd));
    zsbl_uart_flush();
    result = zsbl_uart_write(cmd, sizeof(cmd));

    if (result != 0)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Error writing sys vsn!", __FUNCTION__);
        return result;
    }
    // No response for reset request

    return result;
}

// This is a MT Command (not SBL) - execute in application mode
static int mt_send_get_system_vsn(void)
{
    int result;
    uint8_t cmd[] = {
        0xFE, 0,   //RPC payload Len
        SB_RPC_CMD_AREQ | MT_RPC_SYS_SYS,
        MT_SYS_VERSION_REQ, 0x00   //FCS - fill in later
    };

    calcFcs(cmd, sizeof(cmd));
    zsbl_uart_flush();
    result = zsbl_uart_write(cmd, sizeof(cmd));

    if (result != 0)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Error writing sys vsn!", __FUNCTION__);
        return result;
    }

    result = verify_mt_response(MT_SYS_VERSION_REQ, TIMEOUT_DEFAULT_MS);

    return result;
}

// This is a MT Command (not SBL) - execute in application mode
int execute_soft_reset(char* device)
{
    int result;
    if(device == NULL)
    {
        device = DEFAULT_ZIGBEE_DEVICE;
    }
    result = zsbl_uart_init(device, 115200);

    if(result != 0)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Error initializing UART", __FUNCTION__);
        return result;
    }

    if ((result = mt_send_soft_reset()) == 0)
    {
        usleep(SBL_DELAY_1S_US);
        Logger.writeLog(LOG_WARN, "Platform: %s - Soft Reset successfully sent!", __FUNCTION__);
    }
    else
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Soft Reset failed!", __FUNCTION__);
    }

    zsbl_uart_deinit();

    return result;
}

int execute_read_sys_vsn(char* device, mt_revision* pmtv)
{
    int result;
    if(device == NULL)
    {
        device = DEFAULT_ZIGBEE_DEVICE;
    }
    result = zsbl_uart_init(device, 115200);

    if(result != 0)
    {
        Logger.writeLog(LOG_ERROR, "Error initializing UART");
        return result;
    }

    if ((result = mt_send_get_system_vsn()) == 0)
    {
        Logger.writeLog(LOG_WARN, "Platform: %s - Sys version aquired!", __FUNCTION__);
        if(pmtv != NULL)
        {
            memcpy(pmtv, &g_current_revision.transportRev, sizeof(g_current_revision));
        }
    }
    else
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Sys version failed!", __FUNCTION__);
    }

    zsbl_uart_deinit();

    return result;
}



/**-------------------------------------------------------------------------------------------------
 * [description]
 *------------------------------------------------------------------------------------------------*/

static int read_file2buffer(char* fname, char* buf, size_t* size)
{
    int result = 0;

    if (IsNULL(buf) || IsNULL(size))
    {
        return -1;
    }

    struct stat fileStat;

    if (stat(fname, &fileStat))
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Error opening file %s", __FUNCTION__, fname);
    }

    #if 1
    if ((size_t)fileStat.st_size >= *size)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Not enough memory to read file %d %d", __FUNCTION__, (int)fileStat.st_size, *size);
    }
    #endif
    FILE* f = fopen(fname, "r");
    if (!IsNULL(f))
    {
        if ((size_t)fileStat.st_size != fread(buf, 1, fileStat.st_size, f))
        {
            Logger.writeLog(LOG_ERROR, "Platform: %s - Reading File %s", __FUNCTION__, fname);
            result = -1;
            fileStat.st_size = 0;
        }
    }

    *size = fileStat.st_size;
    if (!IsNULL(f))
    {
        fclose(f);
    }

    return result;
}

// Use the monotonic timer for time comparisons
// timerspec is allowed to contain -ve values, so we just deal directly with timespec without conversions
static void get_timestamp(struct timespec* pts)
{
    if (clock_gettime(CLOCK_MONOTONIC, pts))
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Error reading time!", __FUNCTION__);
    }

    return;
}

// true it current time is greater than timestamp
static bool is_time_expired(struct timespec* ts2, uint32_t timeout_ms)
{
    struct timespec ts1;
    if (clock_gettime(CLOCK_MONOTONIC, &ts1))
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Error reading time!", __FUNCTION__);
    }

    time_t t2_sec = ts2->tv_sec + (timeout_ms / 1000);
    long t2_nsec = ts2->tv_nsec + (timeout_ms % 1000) * 1000 * 1000;

    return (ts1.tv_sec > t2_sec || (ts1.tv_sec == t2_sec && ts1.tv_nsec >= t2_nsec));
}

int parse_handshake(uint8_t* pdata, int len)
{
    if (len < 14)
        return -1;

    rx_handshake hs;
    hs.status = *pdata++;
    hs.bootloaderRev = BUF_TO_UINT32_LITTLE_ENDIAN(pdata);
    hs.deviceType = *pdata++;
    hs.maxDataLen = BUF_TO_UINT32_LITTLE_ENDIAN(pdata);
    hs.pageSize = BUF_TO_UINT32_LITTLE_ENDIAN(pdata);

    Logger.writeLog(LOG_WARN, "Platform: %s - BootRev =%d DevType=%d MaxBuffer=%d PageSize=%d", __FUNCTION__, hs.bootloaderRev, hs.deviceType, hs.maxDataLen, hs.pageSize);

    return 0;
}

// return 0 if a frame was successfully parsed, +ve if parsing is still going on, -ve if invalid frame detected
// Timeouts have to be handled by the caller
static int parse_resp_frame(uint8_t datum, s_resp_context* pr)
{
    int result = 1;
    // printf("State %d, %d", pr->state, pr->payloadCurSiz);
    switch (pr->state)
    {
        case SOF:
            if (datum == SB_SOF_CHAR)
            {
                pr->state = Length;
            }
            // skip other bytes
            break;

        case Length:
            if (datum == SB_EX_LEN_CHAR)
            {
                pr->exLen = true;
                Logger.writeLog(LOG_ERROR, "Platform: %s - Extended Frames are not supported for now!!", __FUNCTION__);
                result = -1;
            } else {
                pr->len = datum;
                pr->state = FID;
            }
            break;

        case FID:
            if (pr->type == SBLFrame && datum == SB_FRAME_ID_CHAR)
            {
                pr->state = CMD;
            }
            else if (pr->type == MTFrame && datum == (SB_SYS_RESP_MASK | MT_RPC_SYS_SYS))
            {
                pr->state = CMD;
            }
            else
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - Invalid Frame ID - %d!!", __FUNCTION__, pr->type);
                result = -1;
            }
            break;

        case CMD:
            pr->respId = datum;
            if (pr->len > 0)
            {
                pr->state = Payload;
            }
            else
            {
                pr->state = FCS;
            }
            break;

        case Payload:
            if ((pr->payload != NULL) && (pr->payloadCurSiz < pr->payloadMaxSiz))
            {
                pr->payload[pr->payloadCurSiz++] = datum;
                if (pr->payloadCurSiz >= pr->len)
                {
                    pr->state = FCS;
                }
            }
            else
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - Invalid payload buffer or buffer is too small!!", __FUNCTION__);
                result = -1;
            }
            break;

        case FCS:
            // check FCS, end parsing
            result = 0;
            break;

        default:
            Logger.writeLog(LOG_ERROR, "Platform: %s - Unknown State", __FUNCTION__);
            result = -1;
            break;
    }

    return result;
}

static void print_msg(const uint8_t* const pdata, int len)
{
#if 0
	for (int i = 0; i < len; i++) {
		printf("%02X:", pdata[i]);
	}
	printf("");
#endif
}

static int verify_response(uint8_t cmd, uint8_t* pbuff, size_t rlen, int timeout_ms)
{
    //usleep(RX_DELAY);
    uint8_t rx[2048] = { 0 };
    int rx_len = 0;
    // Setup response context
    uint8_t payload[512] = { 0 };
    s_resp_context r = { 0 };
    r.type = SBLFrame;
    r.payload = payload;
    r.payloadMaxSiz = sizeof(payload);

    int exit = TIMEOUT_ERROR; // timeout error

    struct timespec ts;
    get_timestamp(&ts);

    while (exit > 0) {
        if(is_time_expired(&ts, timeout_ms))
        {
            exit = TIMEOUT_ERROR;
            break;
        }
        if (zsbl_uart_read(rx, &rx_len, RX_DELAY) != 0)
        {
            exit = -1;
            break;
        }

        for (int i = 0; i < rx_len; i++)
        {
            int parse = parse_resp_frame(rx[i], &r);
            if (parse <= 0)
            {
                exit = parse;
                break;
            }
        }
    }

    if (exit < 0)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Error parsing - exit", __FUNCTION__);
        return -1;
    }

    if (exit == TIMEOUT_ERROR)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Timeout error reading - exit", __FUNCTION__);
        return -1;
    }

    if ((0x80 | cmd) != r.respId)
    {
        Logger.writeLog(LOG_ERROR, "Platform: %s - Unexpected response expected %02X, rxed %02X- exit", __FUNCTION__, 0x80 | cmd, r.respId);
        return -1;
    }

    switch (cmd)
    {
        case SB_HANDSHAKE_CMD:
            parse_handshake(r.payload, r.len);
            break;

        case SB_WRITE_CMD:
            if (r.payload[0] == 0)
            {
                //printf("Write - ACK OK");
            } else
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - Write - ACK ERROR", __FUNCTION__);
                return -1;
            }
            break;

        case SB_READ_CMD:
            {
                // Note: this status byte is not mentioned in the SBL documentation, but it exists!
                if (r.payload[0] == 0)
                {
                    //printf("Read - ACK OK");
                }
                else
                {
                    Logger.writeLog(LOG_ERROR, "Platform: %s - Read - ACK ERROR", __FUNCTION__);
                    return -1;
                }

                uint8_t* p;
                p = &r.payload[1];
                uint32_t start_addr = BUF_TO_UINT32_LITTLE_ENDIAN(p);
                p = &r.payload[5];
                uint32_t len = BUF_TO_UINT32_LITTLE_ENDIAN(p);
                //printf("op-len = %d ", len);
                if (rlen != len)
                {
                    Logger.writeLog(LOG_ERROR, "Platform: %s - Error: wrong amount of bytes read!", __FUNCTION__);
                    return -1;
                }

                if (len > sizeof(payload))
                {
                    Logger.writeLog(LOG_ERROR, "Platform: %s - Error: Payload buffer not big enough!", __FUNCTION__);
                    return -1;
                }
                /* If the "operation length" and "payload length" aren't matching
                    * fill the remaining bytes with 0xFF
                    */
                size_t padding = len - (r.len - 9);
                if ((r.len - 9) != 0)
                {
                    memcpy(pbuff, &r.payload[9], (r.len - 9));
                }
                memset(pbuff + (r.len - 9), 0xFF, padding);
                print_msg(pbuff, len);
            }
            break;

        case SB_ENABLE_CMD:
            if (r.payload[0] == 0)
            {
                Logger.writeLog(LOG_WARN, "Platform: %s - Enable - ACK OK", __FUNCTION__);
            }
            else
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - Enable - ACK ERROR", __FUNCTION__);
                return -1;
            }
            break;
    }

    return 0;
}

static void module_hard_reset(void)
{
    zsbl_uart_deinit();
    Logger.writeLog(LOG_WARN, "Platform: %s - Pin - LOW", __FUNCTION__);
    system(SBL_HARD_RESET_CMD_PIN_LOW);
    usleep(SBL_DELAY_1S_US/2);
    Logger.writeLog(LOG_WARN, "Platform: %s - Pin - HIGH", __FUNCTION__);
    system(SBL_HARD_RESET_CMD_PIN_HIGH);

    usleep(SBL_HARD_RESET_USB_RESTART_TIME);
    Logger.writeLog(LOG_WARN, "Platform: %s - Re-enum", __FUNCTION__);
    // Hard reset on the curent Zigbee module causes a re-enumeration of USB too, so we must re-init the UART
    zsbl_uart_init(g_zigbee_device, 115200);
}

static int send_handshake(void)
{
    // Send handshake and verify response
    int result;
    if ((result = zbSocSblSendMtFrame(SB_HANDSHAKE_CMD, NULL, 0)) == 0)
    {
        result = verify_response(SB_HANDSHAKE_CMD, NULL, 0,
        TIMEOUT_DEFAULT_MS);
    }

    return result;
}

static int send_enable(void)
{
    int result;
    if ((result = zbSocSblSendMtFrame(SB_ENABLE_CMD, NULL, 0)) == 0)
    {
        result = verify_response(SB_ENABLE_CMD, NULL, 0,
        TIMEOUT_DEFAULT_MS);
    }

    return result;
}

static int send_write(uint8_t* buf, uint8_t size, int delay_ms)
{
    int result;
    if ((result = zbSocSblSendMtFrame(SB_WRITE_CMD, buf, size)) == 0)
    {
        if (delay_ms)
            usleep(1000 * delay_ms); // some time to allow device to erase it's pages

        result = verify_response(SB_WRITE_CMD, NULL, 0, TIMEOUT_WRITE_FLASH_MS);
    }

    return result;
}

// buf - out - payload read
// plen - in/out - bytes to read / num of bytes in payload
// return 1 on reaching end or memory 0 on OK, -1 on errors
static int send_read(uint32_t address, uint8_t* buf, size_t* plen)
{
    uint8_t payload[256];
    uint8_t* p = &payload[0];
    // Adjust address offset
    address += SB_CC2538_FLASH_START_ADDR;

    if (address >= SB_CC2538_CODE_FLASH_END_ADDRESS)
    {
        Logger.writeLog(LOG_WARN, "Platform: %s - Reached end of memory - nothing more to read", __FUNCTION__);
        return 1;
    }

    UINT32_TO_BUF_LITTLE_ENDIAN(p, address);
    UINT32_TO_BUF_LITTLE_ENDIAN(p, DEFAULT_WRITE_SIZE);

    int result;
    if ((result = zbSocSblSendMtFrame(SB_READ_CMD, payload, 8)) == 0)
    {
        //usleep(RX_DELAY);
        result = verify_response(SB_READ_CMD, buf, *plen, TIMEOUT_DEFAULT_MS);
    }

    return result;
}

static int check_ff(uint8_t* start, size_t siz)
{
    for (size_t i = 0; i < siz; i++)
    {
        if (start[i] != 0xFF)
            return -1;
    }

    return 0;
}

static int build_next_write_payload(uint32_t address, uint8_t* pin, size_t remaining_bytes, uint8_t* pframe, uint8_t* pff_packet)
{
    address += SB_CC2538_FLASH_START_ADDR;
    UINT32_TO_BUF_LITTLE_ENDIAN(pframe, address);

    int op_length = DEFAULT_WRITE_SIZE;
    if (remaining_bytes < DEFAULT_WRITE_SIZE)
    {
        op_length = remaining_bytes;
    }

    #if 1
    if (memcmp(pin, g_all_ones_512, sizeof(g_all_ones_512)) == 0)
    {
        Logger.writeLog(LOG_WARN, "Platform: %s - Found an page thats 0xFF at %08X", __FUNCTION__, address);
        // Check if the rest of the entire image is also 0xFF
        if (check_ff(pin, remaining_bytes) == 0) {
            Logger.writeLog(LOG_WARN, "Platform: %s - Found all 0xFFs in the image .. we can skip writes now, remaining bytes = %d", __FUNCTION__, remaining_bytes);
            op_length = remaining_bytes;
            UINT32_TO_BUF_LITTLE_ENDIAN(pframe, op_length);
            *pff_packet = true;

            Logger.writeLog(LOG_WARN, "Platform: %s - Writing to %08X", __FUNCTION__, address);
            return op_length;
        }
    }
    #endif

    UINT32_TO_BUF_LITTLE_ENDIAN(pframe, op_length);

    memcpy(pframe, pin, op_length);
    #if DEBUG
    Logger.writeLog(LOG_WARN, "Platform: %s - Writing to %08X", __FUNCTION__, address);
    #endif
    return op_length;
}

static int execute_sbl(char* in_file, char* device)
{
    if(device == NULL)
    {
        device = DEFAULT_ZIGBEE_DEVICE;
    }
    g_zigbee_device = device;

    switch (g_sbl_state)
    {
        case Init:
            // read file into memory
            g_firmware_buffer_size = sizeof(g_firmware_buffer);
            if (read_file2buffer(in_file, (char*)g_firmware_buffer, &g_firmware_buffer_size) != 0)
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - File read error", __FUNCTION__);
                g_sbl_state = Error;
                g_last_state_bef_error = g_sbl_state;
                break;
            }

            zsbl_uart_init(device, 115200);
            g_sbl_state = ResetModule;
            memset(g_all_ones_512, 0xFF, sizeof(g_all_ones_512));
            break;

        case ResetModule:
            // Reset the module - try a soft reset first followed by a hard reset if it doesn't work
            if (mt_send_soft_reset() == 0)
            {
                usleep(SBL_DELAY_1S_US);
                if (send_handshake() != 0)
                {
                    // Try Hard reset and handshake
                    Logger.writeLog(LOG_ERROR, "Platform: %s - Soft Reset failed - trying hard reset!", __FUNCTION__);
                    module_hard_reset();
                    g_sbl_state = Handshake;
                }
                else
                {
                    Logger.writeLog(LOG_WARN, "Platform: %s - Soft Reset successful - entered SBL mode!", __FUNCTION__);
                    g_sbl_state = Handshake;
                }
            }
            else
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - Soft Reset failed - trying hard reset!", __FUNCTION__);
                module_hard_reset();
                g_sbl_state = Handshake;
            }
            break;

        case Handshake:
            // Handshake to verify everything is OK
            if (send_handshake() != 0)
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - Handshake error", __FUNCTION__);
                g_sbl_state = Error;
                g_last_state_bef_error = g_sbl_state;
            }
            else
            {
                Logger.writeLog(LOG_WARN, "Platform: %s - Handshake OK", __FUNCTION__);
                g_sbl_state = Write;
                g_start_address = 0;
            }
            break;

        case Write:
            {
                // Note: the firmware takes care of the erase
                uint8_t payload[256];
                uint8_t ff_packet = false;
                int copied = build_next_write_payload(g_start_address,
                        g_firmware_buffer + g_start_address,
                        (g_firmware_buffer_size - g_start_address), payload,
                        &ff_packet);

                if (ff_packet == true)
                {
                    if (send_write(payload, 8, 10000) != 0)
                    {
                        Logger.writeLog(LOG_ERROR, "Platform: %s - Write error %d, %d", __FUNCTION__, g_start_address, g_firmware_buffer_size);
                        g_sbl_state = Error;
                        g_last_state_bef_error = g_sbl_state;
                        break;
                    }
                }
                else
                {
                    if (send_write(payload, copied + 8, 0) != 0)
                    {
                        Logger.writeLog(LOG_ERROR, "Platform: %s - Write error %d, %d", __FUNCTION__, g_start_address, g_firmware_buffer_size);
                        g_sbl_state = Error;
                        g_last_state_bef_error = g_sbl_state;
                        break;
                    }
                }

                if (g_start_address >= g_firmware_buffer_size)
                {
                    Logger.writeLog(LOG_WARN, "Platform: %s - Done Writing! %d, %d", __FUNCTION__, g_start_address, g_firmware_buffer_size);
                    Logger.writeLog(LOG_WARN, "Platform: %s - Verifying image...This will take some time..", __FUNCTION__);
                    g_sbl_state = Verify;
                    g_start_address = 0;
                }
                else
                {
                    g_start_address += copied;
            #if DEBUG
                    Logger.writeLog(LOG_WARN, "Platform: %s - %d packets to go", __FUNCTION__, (g_firmware_buffer_size - g_start_address) / DEFAULT_WRITE_SIZE);
            #endif
                }
            }
            break;

        case Verify:
            {
                // read back the data
                size_t rlen = DEFAULT_WRITE_SIZE;
                int read_res = send_read(g_start_address + g_read_size, g_firmware_read_buffer + g_read_size, &rlen);

                if (read_res == 1)
                {
                    Logger.writeLog(LOG_WARN, "Platform: %s - Read Completed!", __FUNCTION__);
                    // Compare buffers
                    if (memcmp(g_firmware_buffer, g_firmware_read_buffer, g_firmware_buffer_size) == 0)
                    {
                        Logger.writeLog(LOG_WARN, "Platform: %s - Contents match, wonderful!", __FUNCTION__);
                        g_sbl_state = Enable;
                    }
                    else
                    {
                        g_last_state_bef_error = g_sbl_state;
                        g_sbl_state = Error;
                    }

                    // Write file for easy comparison
                    FILE* pFile = fopen("/tmp/SBL-ZNP-readback.bin", "w");
                    if (pFile != NULL)
                    {
                        fwrite(g_firmware_read_buffer, g_read_size, 1, pFile);
                        if (fclose(pFile) != 0)
                        {
                            Logger.writeLog(LOG_ERROR, "Platform: %s - Error closing file!", __FUNCTION__);
                            perror("");
                        }
                    }

                    break;
                }

                if (read_res == 0)
                {
                    g_read_size += DEFAULT_WRITE_SIZE;
                    // copy the data into the buffer
                }
                else
                {
                    Logger.writeLog(LOG_ERROR, "Platform: %s - Read error", __FUNCTION__);
                    g_sbl_state = Done;
                }
            }
            break;

        case Enable:
            // Since the image is verified, we may manually enable it so that application can restart without any extra delay due to CRC calculation
            if (send_enable() != 0)
            {
                Logger.writeLog(LOG_ERROR, "Platform: %s - ENABLE error", __FUNCTION__);
                g_sbl_state = Error;
                g_last_state_bef_error = g_sbl_state;
            }
            else
            {
                Logger.writeLog(LOG_WARN, "Platform: %s - ENABLE OK", __FUNCTION__);
                g_sbl_state = VerifyAppEntry;
            }
            break;

        case Error:
            Logger.writeLog(LOG_ERROR, "Platform: %s - Error at state %d", __FUNCTION__, g_last_state_bef_error);
            break;

        case VerifyAppEntry:
            // wait to enter application
            Logger.writeLog(LOG_WARN, "Platform: %s - Waiting %dus for system to enter application mode ", __FUNCTION__, SBL_BOOT_DELAY_US);
            usleep(SBL_BOOT_DELAY_US);
            mt_send_get_system_vsn();
            g_sbl_state = Done;
            break;

        case Done:
            break;
    }

    return g_sbl_state;

}

static void sbl_init(void)
{
    g_sbl_state = Init;
    memset(g_firmware_buffer, 0, sizeof(g_firmware_buffer));
    memset(g_firmware_read_buffer, 0, sizeof(g_firmware_read_buffer));
    g_firmware_buffer_size = 0;
    g_read_size = 0;
    memset(&g_current_revision.transportRev, 0, sizeof(g_current_revision));
}

int execute_fw_update(char* file, char* device)
{
    e_sbl_states result;
    sbl_init();
    while((result = execute_sbl(file, device)) < Error);

    zsbl_uart_deinit();
    return (result == Done) ? 0: -1;
}

#if 0
int main(int argc, char* argv[]) {
	if (argc < 2)
	{
		printf("Error, wrong number of argument. specify filename ");
		exit(-1);
	}

	printf("Updating %s ", argv[1]);
	execute_fw_update(argv[1], "/dev/zigbee");

	return 0;

	mt_revision rev;
	execute_read_sys_vsn(NULL, &rev);
	printf("==============================");
	printf("TransportRev = %d", rev.transportRev);
	printf("ProductRev = %d", rev.productRev);
	printf("MajorRev = %d", rev.majorRev);
	printf("MinorRev = %d", rev.minorRev);
	printf("HWRev = %d", rev.hwRev);

	execute_soft_reset(NULL);
}
#endif

