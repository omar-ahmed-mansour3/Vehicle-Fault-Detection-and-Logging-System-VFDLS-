
#include "icu.h"
#include "gpio.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/******************************* Global variable Definitions ******************************/
static void (*g_callBackFunction_ptr)(void) = NULL_PTR;

/* Flag for timeout handling */
static volatile uint8 g_timer1OverflowCount = 0;

/******************************* Interrupt Service Routines ******************************/
ISR(TIMER1_CAPT_vect)
{
    if(g_callBackFunction_ptr != NULL_PTR)
    {
        (*g_callBackFunction_ptr)();
    }
}

/* Handle Timer1 overflow to avoid lockup */
ISR(TIMER1_OVF_vect)
{
    g_timer1OverflowCount++;
}

/********************************** Functions Definitions *********************************/

void ICU_init(const ICU_ConfigType *a_config_Ptr)
{
    GPIO_setupPinDirection(ICU_PORT, ICU_PIN, PIN_INPUT);

    uint8 sreg = SREG;
    cli();

    /* Normal mode */
    TCCR1A = (1<<FOC1A) | (1<<FOC1B);

    /* Clock + edge select */
    TCCR1B = ((a_config_Ptr->edge)<<ICES1) | (a_config_Ptr->clock);

    TCNT1 = 0;
    ICR1  = 0;

    SREG = sreg;

    /* Enable Input Capture + Overflow interrupts */
    TIMSK |= (1<<TICIE1) | (1<<TOIE1);
}

void ICU_setCallBack(void(*a_callBackFunction_ptr)(void))
{
    g_callBackFunction_ptr = a_callBackFunction_ptr;
}

void ICU_setEdgeDetectionType(const ICU_EdgeType a_edgeType)
{
    uint8 sreg = SREG;
    cli();
    TCCR1B = (TCCR1B & ~(1<<ICES1)) | (a_edgeType<<ICES1);
    SREG = sreg;
}

uint16 ICU_getInputCaptureValue(void)
{
    uint16 value;
    uint8 sreg = SREG;
    cli();
    value = ICR1;
    SREG = sreg;
    return value;
}

void ICU_clearTimerValue(void)
{
    uint8 sreg = SREG;
    cli();
    TCNT1 = 0;
    g_timer1OverflowCount = 0; // reset overflow counter
    SREG = sreg;
}

uint8 ICU_isTimeout(void)
{
    /* Example: if overflowed more than 5 times → timeout */
    return (g_timer1OverflowCount >= 5);
}

void ICU_deinit(void)
{
    uint8 sreg = SREG;
    cli();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    ICR1   = 0;

    SREG = sreg;

    TIMSK &= ~((1<<TICIE1) | (1<<TOIE1));
}
