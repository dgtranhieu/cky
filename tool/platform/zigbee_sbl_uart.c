#include <errno.h>
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
#include "Logger.h"
#include "zigbee_sbl_uart.h"



int BAUD = 115200u;
int Zigbee_Handle = -1;

static struct termios oldtio;
static struct serial_struct oldserial;
static int speed = B115200;

static void print_msg(const uint8_t* const pdata, int len)
{
#if 0
	for (int i = 0; i < len; i++)
	{
		printf("%02X:", pdata[i]);
	}
	printf("");
#endif
}

static bool Zigbee_Set_Baud_Rate(uint32_t baud)
{
    bool valid = true;
    switch (baud) {
        case 0:
            speed = B0;
            break;
        case 50:
            speed = B50;
            break;
        case 75:
            speed = B75;
            break;
        case 110:
            speed = B110;
            break;
        case 134:
            speed = B134;
            break;
        case 150:
            speed = B150;
            break;
        case 200:
            speed = B200;
            break;
        case 300:
            speed = B300;
            break;
        case 600:
            speed = B600;
            break;
        case 1200:
            speed = B1200;
            break;
        case 1800:
            speed = B1800;
            break;
        case 2400:
            speed = B2400;
            break;
        case 4800:
            speed = B4800;
            break;
        case 9600:
            speed = B9600;
            break;
        case 19200:
            speed = B19200;
            break;
        case 38400:
            speed = B38400;
            break;
        case 57600:
            speed = B57600;
            break;
        case 115200:
            speed = B115200;
            break;
        case 230400:
            speed = B230400;
            break;
        case 460800:
            speed = B460800;
            break;
        case 921600:
        	speed = B921600;
        	break;
        case 1000000:
        	speed = B1000000;
        	break;
        case 2000000:
        	speed = B2000000;
        	break;
        default:
            valid = false;
            break;
    }

    if (valid)
    { /* FIXME: store the baud rate */ }

    return valid;
}

void zsbl_uart_deinit(void)
{
	// Guard against multiple calls
	if(Zigbee_Handle)
    {
		tcflush(Zigbee_Handle, TCIOFLUSH ); // flush data
	    /* restore the old port settings */
	    tcsetattr(Zigbee_Handle, TCSANOW, &oldtio);
	    ioctl(Zigbee_Handle, TIOCSSERIAL, &oldserial);
	    close(Zigbee_Handle);
	    Zigbee_Handle = 0;
	    Logger.writeLog(LOG_WARN, "UART: %s - Closed UART PORT", __FUNCTION__);
	}
}

int zsbl_uart_init(char* port, uint32_t baud)
{
    struct termios newtio;
    struct serial_struct newserial;
    float baud_error = 0.0;

    printf("RS485: Initializing %s with baud: %u\n", port, baud);
    Zigbee_Set_Baud_Rate(baud);
    /*
       Open device for reading and writing.
       Blocking mode - more CPU effecient
     */
    Zigbee_Handle = open(port, O_RDWR | O_NOCTTY /*| O_NDELAY */ );
    if (Zigbee_Handle < 0)
    {
        perror(port);
       return -1;
    }
#if 0
    /* non blocking for the read */
    fcntl(Zigbee_Handle, F_SETFL, FNDELAY);
#else
    /* efficient blocking for the read */
    fcntl(Zigbee_Handle, F_SETFL, 0);
#endif
    /* save current serial port settings */
    tcgetattr(Zigbee_Handle, &oldtio);
    /* we read the old serial setup */
    ioctl(Zigbee_Handle, TIOCGSERIAL, &oldserial);
    /* we need a copy of existing settings */
    memcpy(&newserial, &oldserial, sizeof(struct serial_struct));
    /* clear struct for new port settings */
    memset(&newtio, 0, sizeof(newtio));
    /*
       BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
       CRTSCTS : output hardware flow control (only used if the cable has
       all necessary lines. See sect. 7 of Serial-HOWTO)
       CS8     : 8n1 (8bit,no parity,1 stopbit)
       CLOCAL  : local connection, no modem contol
       CREAD   : enable receiving characters
     */
    newtio.c_cflag = speed | CS8 | CLOCAL | CREAD;
    /* Raw input */
    newtio.c_iflag = 0;
    /* Raw output */
    newtio.c_oflag = 0;
    /* no processing */
    newtio.c_lflag = 0;
    /* activate the settings for the port after flushing I/O */
    tcsetattr(Zigbee_Handle, TCSAFLUSH, &newtio);

    /* destructor */
    atexit(zsbl_uart_deinit);
    /* flush any data waiting */
    usleep(200000);
    tcflush(Zigbee_Handle, TCIOFLUSH);
    /* ringbuffer */

    Logger.writeLog(LOG_WARN, "UART: %s - success!", __FUNCTION__);
    return 0;

}

int ReadData(void)
{
    fd_set input;
    struct timeval waiter;
    uint8_t buf[2048];
    int n;

    FD_ZERO(&input);
    FD_SET(Zigbee_Handle, &input);
    n = select(Zigbee_Handle + 1, &input, NULL, NULL, &waiter);
    if (n < 0)
    {
        //return; //because of JENKINS_CLANG_BUILD
        return -1;
    }
    if (FD_ISSET(Zigbee_Handle, &input))
    {
        n = read(Zigbee_Handle, buf, sizeof(buf));
        write(1,buf,n);
    }

    return 0;
}

int zsbl_uart_write(uint8_t* pbuf, int len)
{
	int result = 0;
	int written = write(Zigbee_Handle,pbuf,len);
	if (written <= 0)
    {
		Logger.writeLog(LOG_ERROR, "UART: %s - Write error: %s", __FUNCTION__, strerror(errno));
		result = -1;
	}
	else
    {
		if(written != len)
        {
			Logger.writeLog(LOG_ERROR, "UART: %s - Only %d bytes written instead of %d", __FUNCTION__, written, len);
			result = -1;
		}
		tcdrain(Zigbee_Handle);
	}
	return result;
}

int zsbl_uart_flush(void)
{
	return tcflush(Zigbee_Handle, TCIOFLUSH );
}

int zsbl_uart_read(uint8_t* pbuf, int* plen, int timeout)
{
    fd_set input;
    struct timeval waiter;
    int n;
    int result = -1;

    waiter.tv_sec = timeout/1000;
    waiter.tv_usec = timeout%1000;
    FD_ZERO(&input);
    FD_SET(Zigbee_Handle, &input);
    *plen = 0;

    n = select(Zigbee_Handle + 1, &input, NULL, NULL, &waiter);
    if (n < 0)
    {
    	Logger.writeLog(LOG_ERROR, "UART: %s select error", __FUNCTION__);
        perror("");
        return result;
    }
    if (FD_ISSET(Zigbee_Handle, &input))
    {
        n = read(Zigbee_Handle, pbuf, 2048);
        if (n < 0)
        {
            Logger.writeLog(LOG_ERROR, "UART: %s - Read error", __FUNCTION__);
            perror("");
            return result;
        }
        *plen = n;
    }
    else
    {
    	printf("RX + %d - ", n);
    }

    return 0;
}