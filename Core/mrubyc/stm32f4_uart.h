#ifndef STM32F4_UART_H
#define STM32F4_UART_H

#include <stdint.h>
#include "stm32f4_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UART_SIZE_RXFIFO
#define UART_SIZE_RXFIFO 128
#endif

/*! UART Handle
*/
typedef struct UART_HANDLE {
  PIN_HANDLE txd_pin;
  PIN_HANDLE rxd_pin;

  uint8_t unit_num;		// 1..
  uint8_t delimiter;		// '\n'
  uint16_t rx_rd;		// index of rxfifo for read.

  UART_HandleTypeDef *hal_uart;
  uint8_t rxfifo[UART_SIZE_RXFIFO];	// FIFO for received data.

} UART_HANDLE;

extern UART_HANDLE * const TBL_UART_HANDLES[];


/*
  function prototypes.
*/
void uart_init(void);
int uart_setmode(const UART_HANDLE *hndl, int baud, int parity, int stop_bits);
void uart_clear_rx_buffer(UART_HANDLE *hndl);
int uart_read(UART_HANDLE *hndl, void *buffer, int size);
int uart_write(UART_HANDLE *hndl, const void *buffer, int size);
int uart_is_readable(const UART_HANDLE *hndl);
int uart_bytes_available(const UART_HANDLE *hndl);
int uart_can_read_line(const UART_HANDLE *hndl);
void mrbc_init_class_uart(void);


#ifdef __cplusplus
}
#endif
#endif
