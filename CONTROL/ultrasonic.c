#include "Ultrasonic.h"
#include "icu.h"
#include "gpio.h"
#include <util/delay.h>
#include <avr/interrupt.h>
volatile uint8 g_edgeDetectedCount = 0;
volatile uint16 g_echoHighTime = 0;

static void Ultrasonic_edgeProcessing(void);
static void Ultrasonic_Trigger(void);

void Ultrasonic_init(void)
{
    GPIO_setupPinDirection(TRIGGER_PORT, TRIGGER_PIN, PIN_OUTPUT);

    ICU_setCallBack(Ultrasonic_edgeProcessing);

    ICU_ConfigType config_ptr = {FCPU_8_CLOCK, RISING_EDGE};
    ICU_init(&config_ptr);
    sei();
}

static void Ultrasonic_Trigger(void)
{
    GPIO_writePin(TRIGGER_PORT, TRIGGER_PIN, LOGIC_HIGH);
    _delay_us(15);
    GPIO_writePin(TRIGGER_PORT, TRIGGER_PIN, LOGIC_LOW);
}

uint16 Ultrasonic_readDistance(void)
{
    uint16 distance = 0;
    uint32 timeout = 0;

    Ultrasonic_Trigger();

    // Wait for two edges (rising + falling) OR timeout
    while ((g_edgeDetectedCount < 2) && (timeout < 80000UL))
    {
        _delay_us(1);   // avoid tight empty loop
        timeout++;
    }

    if (timeout >= 80000UL)
    {
        // No echo detected → reset and return 0
        g_edgeDetectedCount = 0;
        g_echoHighTime = 0;
        return 0;
    }

    // Use the captured high time to compute distance
    distance = (uint16)((uint32)g_echoHighTime * SOUND_VELOCITY_CM_DIVIDETWO * T_TICK);

    g_edgeDetectedCount = 0;
    g_echoHighTime = 0;

    return distance+1;
}

static void Ultrasonic_edgeProcessing(void)
{
    g_edgeDetectedCount++;

    if (g_edgeDetectedCount == 1)
    {
        ICU_clearTimerValue();
        ICU_setEdgeDetectionType(FALLING_EDGE);
    }
    else if (g_edgeDetectedCount == 2)
    {
        g_echoHighTime = ICU_getInputCaptureValue();
        ICU_setEdgeDetectionType(RISING_EDGE);
    }
}
