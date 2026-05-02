/******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for interrupt-based UART driver (ATmega32)
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "std_types.h"
#include "avr/io.h"
#include "avr/interrupt.h"

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/

/* Enumeration for parity type configuration */
typedef enum {
    UART_PARITY_NONE = 0,
    UART_PARITY_EVEN = 2,
    UART_PARITY_ODD  = 3
} UART_ParityType;

/* Configuration structure to hold UART setup parameters */
typedef struct {
    uint32 baud_rate;        /* Transmission speed (bits/sec) */
    uint8  data_bits;        /* Frame size: usually 8 bits */
    uint8  stop_bits;        /* 1 or 2 stop bits */
    UART_ParityType parity;  /* Parity mode: none, even, odd */
} UART_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

void UART_init(const UART_ConfigType *Config_Ptr);
/* Initialize UART peripheral according to configuration struct */

void UART_sendString(const uint8 *Str);
/* Start transmitting a string; non-blocking (interrupt-based) */

void UART_receiveString(uint8 *StrBuffer, uint8 endChar);
/* Start receiving a string until endChar is found */

void UART_setRxStringCallback(void (*a_ptr)(void));
/* Register function to call automatically when string reception completes */


uint8 UART_isTxComplete(void);
/* Check if UART transmission is complete - returns 1 if done, 0 if busy */

#endif /* UART_H_ */
