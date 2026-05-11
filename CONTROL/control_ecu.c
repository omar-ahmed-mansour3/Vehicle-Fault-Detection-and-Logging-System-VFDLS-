#include "uart.h"
#include "gpio.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ultrasonic.h"
#include "lm35_sensor.h"
#include "adc.h"
#include "external_eeprom.h"
#include "twi.h"
#include "dc_motor.h"

/* Defines */
#define RX_BUFFER_SIZE 30
#define ADC_REF_VOLT 2.56

/* Error Logging Defines */
#define ERROR_DISTANCE 'a'              // P001 equivalent
#define ERROR_TEMPERATURE 'b'           // P002 equivalent
#define EEPROM_ERROR_COUNT_ADDR 0x0000
#define EEPROM_ERRORS_START_ADDR 0x0001
#define MAX_ERRORS 20

/* Window Control Defines */
#define MAX_TICKS 40  // 40 ticks * 50ms = 2000ms = 2 seconds

#define WINDOW1_OPEN_BUTTON_PORT    PORTD_ID
#define WINDOW1_OPEN_BUTTON_PIN     PIN2_ID
#define WINDOW1_CLOSE_BUTTON_PORT   PORTD_ID
#define WINDOW1_CLOSE_BUTTON_PIN    PIN3_ID
#define WINDOW2_OPEN_BUTTON_PORT    PORTD_ID
#define WINDOW2_OPEN_BUTTON_PIN     PIN4_ID
#define WINDOW2_CLOSE_BUTTON_PORT   PORTD_ID
#define WINDOW2_CLOSE_BUTTON_PIN    PIN5_ID

/* Window States */
typedef enum {
	WINDOW_CLOSED = 0,
	WINDOW_OPENING,
	WINDOW_OPEN,
	WINDOW_CLOSING
} WindowState;

/* Global Variables */
volatile uint8 g_rxBuffer[RX_BUFFER_SIZE];
volatile uint8 g_newMessageReceived = 0;
volatile uint8 g_currentMode = 0;

/* Error Logging Variables */
volatile uint8 g_errorCount = 0;
volatile uint8 g_lastDistanceError = 0;
volatile uint8 g_lastTempError = 0;
volatile uint8 g_prevMode = 0;

/* Window Control Variables */
volatile uint8 g_window1_position = 0;  // 0 = closed, MAX_TICKS = open
volatile uint8 g_window2_position = 0;
volatile WindowState g_window1_state = WINDOW_CLOSED;
volatile WindowState g_window2_state = WINDOW_CLOSED;

static uint8 g_canSendUART = 1;

/* Function Prototypes */
void rxCompleteCallback(void);
void intToString(uint16 num, uint8 *str);
void logError(uint8 errorCode);
void initWindowButtons(void);
void handleWindowControl(void);
uint8 readButton(uint8 port, uint8 pin);

int main(void)
{
	UART_ConfigType uartConfig = {9600, 8, 1, UART_PARITY_NONE};
	ADC_ConfigType adcConfig = {INTERNAL_2_56V_REF, ADC_PRESCALER_8};
	TWI_ConfigType twiConfig = {0x01, 400000}; // address=1, 400kHz

	uint8 temprature;
	uint16 distance;

	/* Initialize peripherals */
	UART_init(&uartConfig);
	Ultrasonic_init();
	ADC_init(&adcConfig);
	TWI_init(&twiConfig);
	DcMotor_init();
	initWindowButtons();
	_delay_ms(10);

	/* Read error count from EEPROM on startup */
	EEPROM_readByte(EEPROM_ERROR_COUNT_ADDR, (uint8*)&g_errorCount);
	_delay_ms(10);
	/* If EEPROM is uninitialized (0xFF), reset to 0 */
	if(g_errorCount == 0xFF)
	{
		g_errorCount = 0;
		EEPROM_writeByte(EEPROM_ERROR_COUNT_ADDR, 0);
		_delay_ms(10);
	}

	UART_setRxStringCallback(rxCompleteCallback);
	UART_receiveString((uint8*)g_rxBuffer, '#');

	sei();

	while(1)
	{
		/* Read sensors */


		/* Error logging only in Mode 1 */
		if(g_currentMode == 1)
		{
			/* Check distance fault (only log once per occurrence) */
			if(distance < 10 && !g_lastDistanceError)
			{
				logError(ERROR_DISTANCE);
				g_lastDistanceError = 1;
			}
			else if(distance >= 10)
			{
				g_lastDistanceError = 0;
			}

			/* Check temperature fault (only log once per occurrence) */
			if(temprature > 90 && !g_lastTempError)
			{
				logError(ERROR_TEMPERATURE);
				g_lastTempError = 1;
			}
			else if(temprature <= 90)
			{
				g_lastTempError = 0;
			}
		}

		/* Handle window control */
		handleWindowControl();

		/* Handle mode changes from HMI */
		if (g_newMessageReceived)
		{
			g_newMessageReceived = 0;

			/* Save old mode, then update current mode */
			uint8 newMode = g_rxBuffer[0] - '0';
			if (newMode > 4) newMode = 0;

			/* Handle Mode transitions */
			if (newMode != g_currentMode)
			{
				/* Leaving Mode 1 → reset flags */
				if (g_currentMode == 1 && newMode != 1)
				{
					g_lastDistanceError = 0;
					g_lastTempError = 0;
				}

				/* Entering Mode 1 → immediately log active faults */
				if (newMode == 1)
				{
					if (distance < 10 && !g_lastDistanceError)
					{
						logError(ERROR_DISTANCE);
						g_lastDistanceError = 1;
					}
					if (temprature > 90 && !g_lastTempError)
					{
						logError(ERROR_TEMPERATURE);
						g_lastTempError = 1;
					}
				}

				g_prevMode = g_currentMode;
				g_currentMode = newMode;
			}

			/* Prepare for next UART message */
			UART_receiveString((uint8*)g_rxBuffer, '#');
		}

		if ((g_currentMode == 1 || g_currentMode == 2) && g_canSendUART)
		{
			g_canSendUART = 0;  // Block new transmissions until this one completes
			distance = Ultrasonic_readDistance();
			temprature = LM35_getTemperature(ADC_REF_VOLT);
		    uint8 temp_snapshot = temprature;
		    uint16 dist_snapshot = distance;
		    WindowState win1_snapshot = g_window1_state;
		    WindowState win2_snapshot = g_window2_state;
			/* Mode 1 or 2: send temperature, distance, and window states */
			uint8 txBuffer[25];
			uint8 i = 0;

			/* Convert temperature to string manually */
			uint16 temp = temp_snapshot;
			uint8 tempDigits[5];
			uint8 tIndex = 0;

			if (temp == 0)
			{
				txBuffer[i++] = '0';
			}
			else
			{
				while (temp > 0)
				{
					tempDigits[tIndex++] = (temp % 10) + '0';
					temp /= 10;
				}
				while (tIndex > 0)
				{
					txBuffer[i++] = tempDigits[--tIndex];
				}
			}

			txBuffer[i++] = ' ';

			/* Convert distance to string manually */
			uint16 dist = dist_snapshot;
			uint8 distDigits[5];
			uint8 dIndex = 0;

			if (dist == 0)
			{
				txBuffer[i++] = '0';
			}
			else
			{
				while (dist > 0)
				{
					distDigits[dIndex++] = (dist % 10) + '0';
					dist /= 10;
				}
				while (dIndex > 0)
				{
					txBuffer[i++] = distDigits[--dIndex];
				}
			}

			txBuffer[i++] = ' ';

			/* Window 1 state: 1 = Open/Opening, 0 = Closed/Closing */
			if (win1_snapshot == WINDOW_OPEN || win1_snapshot == WINDOW_OPENING)
			{
				txBuffer[i++] = '1';
			}
			else
			{
				txBuffer[i++] = '0';
			}

			txBuffer[i++] = ' ';

			/* Window 2 state: 1 = Open/Opening, 0 = Closed/Closing */
			if (win2_snapshot == WINDOW_OPEN || win2_snapshot == WINDOW_OPENING)
			{
				txBuffer[i++] = '1';
			}
			else
			{
				txBuffer[i++] = '0';
			}

			txBuffer[i++] = '#';
			txBuffer[i] = '\0';

			UART_sendString(txBuffer);
		}

		/* Check if transmission completed, allow next send */
		if (!g_canSendUART && UART_isTxComplete())
		{
			g_canSendUART = 1;
		}
		if (g_currentMode == 3)
		{
			/* Mode 3: send ALL logged fault info from EEPROM */
			uint8 txBuffer[50];
			uint8 i = 0;
			uint8 errorData;

			txBuffer[i++] = 'E';
			txBuffer[i++] = 'r';
			txBuffer[i++] = 'r';
			txBuffer[i++] = ' ';

			if (g_errorCount == 0)
			{
				txBuffer[i++] = 'n';
				txBuffer[i++] = 'o';
				txBuffer[i++] = 'n';
				txBuffer[i++] = 'e';
			}
			else
			{
				/* Send ALL logged errors from EEPROM */
				for(uint8 j = 0; j < g_errorCount && j < 20; j++)
				{
					EEPROM_readByte(EEPROM_ERRORS_START_ADDR + j, &errorData);
					_delay_ms(10);

					if(errorData == 'a' || errorData == ERROR_DISTANCE)
					{
						txBuffer[i++] = 'A';
						txBuffer[i++] = ' ';
					}
					else if(errorData == 'b' || errorData == ERROR_TEMPERATURE)
					{
						txBuffer[i++] = 'B';
						txBuffer[i++] = ' ';
					}
				}
			}

			txBuffer[i++] = '#';
			txBuffer[i] = '\0';

			UART_sendString(txBuffer);
		}

		_delay_ms(50);
	}

	return 0;
}

/* Initialize window button pins as inputs with internal pull-up */
void initWindowButtons(void)
{
	GPIO_setupPinDirection(WINDOW1_OPEN_BUTTON_PORT, WINDOW1_OPEN_BUTTON_PIN, PIN_INPUT);
	GPIO_setupPinDirection(WINDOW1_CLOSE_BUTTON_PORT, WINDOW1_CLOSE_BUTTON_PIN, PIN_INPUT);
	GPIO_setupPinDirection(WINDOW2_OPEN_BUTTON_PORT, WINDOW2_OPEN_BUTTON_PIN, PIN_INPUT);
	GPIO_setupPinDirection(WINDOW2_CLOSE_BUTTON_PORT, WINDOW2_CLOSE_BUTTON_PIN, PIN_INPUT);

	/* Enable internal pull-up resistors */
	GPIO_writePin(WINDOW1_OPEN_BUTTON_PORT, WINDOW1_OPEN_BUTTON_PIN, LOGIC_HIGH);
	GPIO_writePin(WINDOW1_CLOSE_BUTTON_PORT, WINDOW1_CLOSE_BUTTON_PIN, LOGIC_HIGH);
	GPIO_writePin(WINDOW2_OPEN_BUTTON_PORT, WINDOW2_OPEN_BUTTON_PIN, LOGIC_HIGH);
	GPIO_writePin(WINDOW2_CLOSE_BUTTON_PORT, WINDOW2_CLOSE_BUTTON_PIN, LOGIC_HIGH);
}

/* Read button state (returns 1 if pressed, 0 if not pressed) */
uint8 readButton(uint8 port, uint8 pin)
{
	return (GPIO_readPin(port, pin) == LOGIC_LOW) ? 1 : 0;
}

/* Handle window control logic with travel counter */
void handleWindowControl(void)
{
	/* ===== WINDOW 1 CONTROL ===== */
	switch(g_window1_state)
	{
	case WINDOW_CLOSED:
		/* Ensure position is at 0 */
		g_window1_position = 0;
		/* Only respond to OPEN button */
		if(readButton(WINDOW1_OPEN_BUTTON_PORT, WINDOW1_OPEN_BUTTON_PIN))
		{
			g_window1_state = WINDOW_OPENING;
			DcMotor_Rotate(MOTOR1, CW);  // Start opening
		}
		break;

	case WINDOW_OPENING:
		/* Increment position counter */
		g_window1_position++;

		if(g_window1_position >= MAX_TICKS)
		{
			/* Reached fully open */
			g_window1_position = MAX_TICKS;
			g_window1_state = WINDOW_OPEN;
			DcMotor_Rotate(MOTOR1, STOP);
		}
		/* Ignore all button presses during transition */
		break;

	case WINDOW_OPEN:
		/* Ensure position is at MAX */
		g_window1_position = MAX_TICKS;
		/* Only respond to CLOSE button */
		if(readButton(WINDOW1_CLOSE_BUTTON_PORT, WINDOW1_CLOSE_BUTTON_PIN))
		{
			g_window1_state = WINDOW_CLOSING;
			DcMotor_Rotate(MOTOR1, Anti_CW);  // Start closing
		}
		break;

	case WINDOW_CLOSING:
		/* Decrement position counter */
		if(g_window1_position > 0)
		{
			g_window1_position--;

			/* Check if reached fully closed */
			if(g_window1_position == 0)
			{
				g_window1_state = WINDOW_CLOSED;
				DcMotor_Rotate(MOTOR1, STOP);
			}
		}
		/* Ignore all button presses during transition */
		break;
	}

	/* ===== WINDOW 2 CONTROL ===== */
	switch(g_window2_state)
	{
	case WINDOW_CLOSED:
		/* Ensure position is at 0 */
		g_window2_position = 0;
		/* Only respond to OPEN button */
		if(readButton(WINDOW2_OPEN_BUTTON_PORT, WINDOW2_OPEN_BUTTON_PIN))
		{
			g_window2_state = WINDOW_OPENING;
			DcMotor_Rotate(MOTOR2, CW);  // Start opening
		}
		break;

	case WINDOW_OPENING:
		/* Increment position counter */
		g_window2_position++;

		if(g_window2_position >= MAX_TICKS)
		{
			/* Reached fully open */
			g_window2_position = MAX_TICKS;
			g_window2_state = WINDOW_OPEN;
			DcMotor_Rotate(MOTOR2, STOP);
		}
		/* Ignore all button presses during transition */
		break;

	case WINDOW_OPEN:
		/* Ensure position is at MAX */
		g_window2_position = MAX_TICKS;
		/* Only respond to CLOSE button */
		if(readButton(WINDOW2_CLOSE_BUTTON_PORT, WINDOW2_CLOSE_BUTTON_PIN))
		{
			g_window2_state = WINDOW_CLOSING;
			DcMotor_Rotate(MOTOR2, Anti_CW);  // Start closing
		}
		break;

	case WINDOW_CLOSING:
		/* Decrement position counter */
		if(g_window2_position > 0)
		{
			g_window2_position--;

			/* Check if reached fully closed */
			if(g_window2_position == 0)
			{
				g_window2_state = WINDOW_CLOSED;
				DcMotor_Rotate(MOTOR2, STOP);
			}
		}
		/* Ignore all button presses during transition */
		break;
	}
}

/* UART receive callback */
void rxCompleteCallback(void)
{
	g_newMessageReceived = 1;
}

/* Convert integer to ASCII string manually */
void intToString(uint16 num, uint8 *str)
{
	uint8 i = 0;
	uint8 temp[6];

	if(num == 0)
	{
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	while(num > 0)
	{
		temp[i++] = (num % 10) + '0';
		num /= 10;
	}

	/* Reverse digits into str[] */
	uint8 j;
	for(j = 0; j < i; j++)
	{
		str[j] = temp[i - j - 1];
	}
	str[i] = '\0';
}

/* Log error to EEPROM */
void logError(uint8 errorCode)
{
	if(g_errorCount >= MAX_ERRORS) return;

	uint16 addr = EEPROM_ERRORS_START_ADDR + g_errorCount;
	EEPROM_writeByte(addr, errorCode);
	_delay_ms(10);

	g_errorCount++;
	EEPROM_writeByte(EEPROM_ERROR_COUNT_ADDR, g_errorCount);
	_delay_ms(10);
}
