#ifndef TI_LINUX_GATEWAY_SOURCE_PROJECTS_ZSTACK_LINUX_SBL_TOOL_SOURCE_ZIGBEE_SBL_UART_H_
#define TI_LINUX_GATEWAY_SOURCE_PROJECTS_ZSTACK_LINUX_SBL_TOOL_SOURCE_ZIGBEE_SBL_UART_H_

int zsbl_uart_init(char* port, uint32_t baud);
void zsbl_uart_deinit(void);
int zsbl_uart_write(uint8_t* pbuf, int len);
int zsbl_uart_flush(void);
int zsbl_uart_read(uint8_t* pbuf, int* plen, int timeout);


#endif /* TI_LINUX_GATEWAY_SOURCE_PROJECTS_ZSTACK_LINUX_SBL_TOOL_SOURCE_ZIGBEE_SBL_UART_H_ */
