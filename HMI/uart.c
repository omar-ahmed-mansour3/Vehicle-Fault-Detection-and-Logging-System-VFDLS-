#include "uart.h"
#include "common_macros.h"   // for SET_BIT / CLEAR_BIT macros

/* ==========================
   Global / Static Variables
   ========================== */

/* Transmission state */
static volatile const uint8 *g_txString = NULL_PTR;
static volatile uint8 g_txIndex = 0;
static volatile uint8 g_txInProgress = 0;

/* Reception state */
static volatile uint8 *g_rxStringBuffer = NULL_PTR;
static volatile uint8 g_rxIndex = 0;
static volatile uint8 g_rxEndChar = '#';
static volatile uint8 g_rxReceiving = 0;

/* Callback function pointer (called when a full string is received) */
static void (*g_rxStringCallback)(void) = NULL_PTR;


/*******************************************************************************
 *                           Function Definitions                              *
 *******************************************************************************/

void UART_init(const UART_ConfigType *Config_Ptr)
{
    uint16 ubrr_value = 0;
    uint8 ucsrc_value = 0;

    /* Step 1: Enable double transmission speed */
    UCSRA = (1 << U2X);
    // Setting U2X makes UART sample at 8x clock rate instead of 16x,
    // reducing baud rate error.

    /* Step 2: Enable transmitter, receiver, and RX complete interrupt */
    UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
    // RXEN: enable receiver hardware
    // TXEN: enable transmitter hardware
    // RXCIE: trigger interrupt when a byte is received

    /* Step 3: Build UCSRC frame format byte */
    ucsrc_value = (1 << URSEL); // URSEL=1 means we’re writing to UCSRC (not UBRRH)

    /* Set parity bits according to config */
    if(Config_Ptr->parity == UART_PARITY_EVEN)
        ucsrc_value |= (1 << UPM1);
    else if(Config_Ptr->parity == UART_PARITY_ODD)
        ucsrc_value |= (1 << UPM1) | (1 << UPM0);

    /* Set number of stop bits */
    if(Config_Ptr->stop_bits == 2)
        ucsrc_value |= (1 << USBS);

    /* Set data frame size */
    switch(Config_Ptr->data_bits)
    {
        case 6: ucsrc_value |= (1 << UCSZ0); break;
        case 7: ucsrc_value |= (1 << UCSZ1); break;
        case 8: ucsrc_value |= (1 << UCSZ0) | (1 << UCSZ1); break;
        default: break; // 5 bits default (00)
    }

    UCSRC = ucsrc_value; // Write final frame format

    /* Step 4: Set baud rate registers */
    ubrr_value = (uint16)(((F_CPU / ((Config_Ptr->baud_rate) * 8UL))) - 1);
    UBRRH = ubrr_value >> 8;
    UBRRL = ubrr_value;
}



uint8 UART_isTxComplete(void)
{
    return !g_txInProgress;
}
/* ============================ TRANSMISSION ============================ */

void UART_sendString(const uint8 *Str)
{
    if(Str == NULL_PTR || g_txInProgress) return; // ignore if busy

    g_txString = Str;       // point to string to send
    g_txIndex = 0;
    g_txInProgress = 1;

    SET_BIT(UCSRB, UDRIE);  // enable Data Register Empty interrupt
    // This causes USART_UDRE_vect ISR to trigger immediately,
    // which begins sending characters one by one.
}

/* ISR triggered when UDR (transmit buffer) becomes empty */
ISR(USART_UDRE_vect)
{
    if(g_txString[g_txIndex] != '\0')
    {
        UDR = g_txString[g_txIndex++];
        // Writing to UDR starts transmission of that byte.
    }
    else
    {
        g_txString = NULL_PTR;
        g_txInProgress = 0;
        CLEAR_BIT(UCSRB, UDRIE); // disable interrupt once done
    }
}

/* ============================= RECEPTION ============================= */

void UART_receiveString(uint8 *StrBuffer, uint8 endChar)
{
    if(StrBuffer == NULL_PTR) return;

    g_rxStringBuffer = StrBuffer;
    g_rxEndChar = endChar;
    g_rxIndex = 0;
    g_rxReceiving = 1;
}

/* Register a callback that will run when the full string is received */
void UART_setRxStringCallback(void (*a_ptr)(void))
{
    g_rxStringCallback = a_ptr;
}

/* ISR triggered when a byte is received and available in UDR */
ISR(USART_RXC_vect)
{
    uint8 received = UDR; // reading clears RXC flag automatically

    if(g_rxReceiving && g_rxStringBuffer != NULL_PTR)
    {
        if(received != g_rxEndChar)
        {
            g_rxStringBuffer[g_rxIndex++] = received;
        }
        else
        {
            g_rxStringBuffer[g_rxIndex] = '\0'; // terminate string
            g_rxReceiving = 0;

            if(g_rxStringCallback != NULL_PTR)
                g_rxStringCallback(); // notify upper layer
        }
    }
}
