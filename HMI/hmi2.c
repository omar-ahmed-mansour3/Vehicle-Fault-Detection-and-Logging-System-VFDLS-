/******************************************************************************
 * MCU2 Main Application - WITH MODE 5
 *
 * Description: Second ATmega32 - Mode 5 is confirmation screen for Mode 2/3
 ******************************************************************************/

#include "uart.h"
#include "lcd.h"
#include "gpio.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include "keypad.h"
#include "timer0.h"

/* Global buffer to receive messages */
#define RX_BUFFER_SIZE 50
#define MAX_ERRORS 100

volatile uint8 g_rxBuffer[RX_BUFFER_SIZE];
volatile uint8 g_newMessageReceived = 0;
volatile uint8 g_timeoutFlag = 0;
volatile uint16 g_timerTicks = 0;
volatile uint8 g_forceFirstDisplay = 0;

/* Error scrolling variables */
uint8 g_errorList[MAX_ERRORS];
uint8 g_errorCount = 0;
uint8 g_scrollIndex = 0;
uint16 g_scrollTimerTicks = 0;
uint8 g_displayState = 0;
uint8 previous_mode = 0;
uint8 scrollPage = 0;
uint8 endDisplayed = 0;
uint8 prevErrorCount = 0;
uint8 g_errorsReady = 0;

/* Mode 5 variables */
uint8 g_previousMode = 0;  // Store which mode (2 or 3) we came from

void UART_parseData(uint8 *str, uint8 *tempPtr, uint16 *distPtr, uint8 *win1Ptr, uint8 *win2Ptr);
void UART_parseErrors(uint8 *str, uint8 *errorList, uint8 *count);
void displayErrorScroll(void);
void rxCompleteCallback(void);
void timerCallback(void);

int main(void)
{
	UART_ConfigType uartConfig = {9600, 8, 1, UART_PARITY_NONE};
	uint8 temprature = 0;
	uint16 distance = 0;
	uint8 window1_state = 0;
	uint8 window2_state = 0;
	uint8 key;
	uint8 current_mode = 0;  // 0=menu, 1-4=normal modes, 5=confirmation
	uint8 mode[3];
	uint8 screen_updated = 0;

	LCD_init();
	UART_init(&uartConfig);
	UART_setRxStringCallback(rxCompleteCallback);
	UART_receiveString((uint8*)g_rxBuffer, '#');
	sei();

	Timer_ConfigType timerConfig = {
			.timer_ID = timer1,
			.timer_mode = compare,
			.timer_InitialValue = 0,
			.timer_compare_MatchValue = 7811,
			.timer_clock = F_CPU_1024
	};

	Timer_init(&timerConfig);
	Timer_setCallBack(timerCallback, timer1);

	LCD_clearScreen();
	LCD_moveCursor(0,0);
	LCD_displayString("1.Start System ");
	LCD_moveCursor(1,0);
	LCD_displayString("2.Show Readings ");
	LCD_moveCursor(2,0);
	LCD_displayString("3.View Faults ");
	LCD_moveCursor(3,0);
	LCD_displayString("4.Stop System ");

	while(1)
	{
		key = KEYPAD_getPressedKey();

		// ========== KEY HANDLING ==========
		if(key != 0xFF)
		{
			// Special handling for Mode 5 (confirmation screen)
			if(current_mode == 5)
			{
				// If user presses 2 or 3 (the mode they came from)
				if(key == g_previousMode)
				{
					// Re-enter that mode fresh
					current_mode = key;
					screen_updated = 0;
					g_timerTicks = 0;
					g_timeoutFlag = 0;

					// Special reset for Mode 3
					if(current_mode == 3)
					{
						g_errorsReady = 0;
						g_forceFirstDisplay = 1;
						g_scrollIndex = 0;
						g_scrollTimerTicks = 0;
						g_displayState = 0;
						scrollPage = 0;
						endDisplayed = 0;
						prevErrorCount = 0xFF;
					}

					// Send mode to Control ECU
					mode[0] = key + '0';
					mode[1] = '#';
					mode[2] = '\0';
					UART_sendString(mode);
				}
				else  // Any other key -> back to main menu
				{
					current_mode = 0;
					screen_updated = 0;
					g_timerTicks = 0;
					g_timeoutFlag = 0;

					LCD_clearScreen();
					LCD_moveCursor(0,0);
					LCD_displayString("1.Start System ");
					LCD_moveCursor(1,0);
					LCD_displayString("2.Show Readings ");
					LCD_moveCursor(2,0);
					LCD_displayString("3.View Faults ");
					LCD_moveCursor(3,0);
					LCD_displayString("4.Stop System ");

					mode[0] = '0';
					mode[1] = '#';
					mode[2] = '\0';
					UART_sendString(mode);
				}
				_delay_ms(300);  // Debounce
				continue;
			}

			// Normal key handling (not in Mode 5)
			if(key >= 1 && key <= 4)
			{
				// Different mode pressed -> return to menu
				if(current_mode != 0 && key != current_mode)
				{
					current_mode = 0;
					screen_updated = 0;
					g_timerTicks = 0;
					g_timeoutFlag = 0;
					g_scrollIndex = 0;
					g_scrollTimerTicks = 0;
					g_displayState = 0;

					LCD_clearScreen();
					LCD_moveCursor(0,0);
					LCD_displayString("1.Start System ");
					LCD_moveCursor(1,0);
					LCD_displayString("2.Show Readings ");
					LCD_moveCursor(2,0);
					LCD_displayString("3.View Faults ");
					LCD_moveCursor(3,0);
					LCD_displayString("4.Stop System ");

					mode[0] = '0';
					mode[1] = '#';
					mode[2] = '\0';
					UART_sendString(mode);
				}
				else  // Enter mode from menu or same key
				{
					previous_mode = current_mode;
					current_mode = key;

					// Special setup for Mode 3
					if(previous_mode != 3 && current_mode == 3)
					{
						g_errorsReady = 0;
						g_forceFirstDisplay = 1;
						g_scrollIndex = 0;
						g_scrollTimerTicks = 0;
						g_displayState = 0;
						scrollPage = 0;
						endDisplayed = 0;
						prevErrorCount = 0xFF;
					}

					screen_updated = 0;
					g_timerTicks = 0;
					g_timeoutFlag = 0;
					g_scrollIndex = 0;
					g_scrollTimerTicks = 0;
					g_displayState = 0;

					// Send mode to MCU1
					mode[0] = key + '0';
					mode[1] = '#';
					mode[2] = '\0';
					UART_sendString(mode);
				}
			}
			else  // Invalid key -> return to menu
			{
				if(current_mode != 0)
				{
					current_mode = 0;
					screen_updated = 0;
					g_timerTicks = 0;
					g_timeoutFlag = 0;

					LCD_clearScreen();
					LCD_moveCursor(0,0);
					LCD_displayString("1.Start System ");
					LCD_moveCursor(1,0);
					LCD_displayString("2.Show Readings ");
					LCD_moveCursor(2,0);
					LCD_displayString("3.View Faults ");
					LCD_moveCursor(3,0);
					LCD_displayString("4.Stop System ");

					mode[0] = '0';
					mode[1] = '#';
					mode[2] = '\0';
					UART_sendString(mode);
				}
			}
			_delay_ms(300);  // Debounce
		}

		// ========== TIMEOUT HANDLING ==========
		if(g_timeoutFlag)
		{
			g_timeoutFlag = 0;
			g_timerTicks = 0;

			// Modes 2 and 3 -> Go to Mode 5
			if(current_mode == 2 || current_mode == 3)
			{
				g_previousMode = current_mode;  // Remember where we came from
				current_mode = 5;                // Enter Mode 5
				screen_updated = 0;              // Force screen redraw
			}
			// Modes 1, 4, and 5 -> Go to main menu
			else if(current_mode == 1 || current_mode == 4 || current_mode == 5)
			{
				current_mode = 0;
				screen_updated = 0;

				LCD_clearScreen();
				LCD_moveCursor(0,0);
				LCD_displayString("1.Start System ");
				LCD_moveCursor(1,0);
				LCD_displayString("2.Show Readings ");
				LCD_moveCursor(2,0);
				LCD_displayString("3.View Faults ");
				LCD_moveCursor(3,0);
				LCD_displayString("4.Stop System ");

				mode[0] = '0';
				mode[1] = '#';
				mode[2] = '\0';
				UART_sendString(mode);
			}
		}

		// ========== UART MESSAGE HANDLING ==========
		if(g_newMessageReceived)
		{
			g_newMessageReceived = 0;
			UART_receiveString((uint8*)g_rxBuffer, '#');

			if(current_mode == 1 || current_mode == 2)
			{
				UART_parseData((uint8*)g_rxBuffer, &temprature, &distance, &window1_state, &window2_state);

				LCD_moveCursor(0,6);
				LCD_intgerToString(temprature);
				LCD_displayString(" C   ");

				LCD_moveCursor(1,6);
				LCD_intgerToString(distance);
				LCD_displayString(" cm  ");

				LCD_moveCursor(2,0);
				LCD_displayString("Win1: ");
				if(window1_state)
				{
					LCD_displayString("Open   ");
				}
				else
				{
					LCD_displayString("Closed ");
				}

				LCD_moveCursor(3,0);
				LCD_displayString("Win2: ");
				if(window2_state)
				{
					LCD_displayString("Open   ");
				}
				else
				{
					LCD_displayString("Closed ");
				}
			}
			else if(current_mode == 3 && !g_errorsReady)  // Only process ONCE
			{
				UART_parseErrors((uint8*)g_rxBuffer, g_errorList, &g_errorCount);
				g_scrollIndex = 0;
				g_scrollTimerTicks = 0;
				g_displayState = 0;
				screen_updated = 0;
				g_forceFirstDisplay = 1;
				g_errorsReady = 1;  // Mark as ready, ignore future messages
			}
			// If g_errorsReady == 1, ignore all future Mode 3 UART messages
		}

		// ========== INITIAL SCREEN SETUP ==========
		if(!screen_updated)
		{
			switch(current_mode)
			{
			case 1:
			case 2:
				LCD_clearScreen();
				LCD_moveCursor(0,0);
				LCD_displayString("Temp: ");
				LCD_moveCursor(1,0);
				LCD_displayString("Dist: ");
				LCD_moveCursor(2,0);
				LCD_displayString("Win1: ");
				LCD_moveCursor(3,0);
				LCD_displayString("Win2: ");
				screen_updated = 1;
				break;

			case 3:
				LCD_clearScreen();
				LCD_moveCursor(0,0);
				LCD_displayString("Logged Errors:");
				g_scrollIndex = 0;
				g_scrollTimerTicks = 0;
				g_displayState = 0;
				screen_updated = 1;
				break;

			case 4:
				LCD_clearScreen();
				LCD_moveCursor(0,0);
				LCD_displayString("System Monitoring");
				LCD_moveCursor(1,0);
				LCD_displayString("Stopped!        ");
				LCD_moveCursor(2,0);
				LCD_displayString("Returning to    ");
				LCD_moveCursor(3,0);
				LCD_displayString("Menu...         ");
				screen_updated = 1;
				break;

			case 5:  // Mode 5: Display Again confirmation
				LCD_clearScreen();
				LCD_moveCursor(0,0);
				LCD_displayString("Display again?");
				LCD_moveCursor(1,0);
				if(g_previousMode == 2)
				{
					LCD_displayString("Press 2 = YES");
				}
				else  // g_previousMode == 3
				{
					LCD_displayString("Press 3 = YES");
				}
				LCD_moveCursor(2,0);
				LCD_displayString("                ");
				LCD_moveCursor(3,0);
				LCD_displayString("Other key = MENU");
				screen_updated = 1;
				break;

			case 0:
			default:
				screen_updated = 1;
				break;
			}
		}

		// ========== MODE 3 ERROR SCROLLING ==========
		if(current_mode == 3 && g_errorsReady)
		{
			displayErrorScroll();
		}

		_delay_ms(20);
	}

	return 0;
}

void rxCompleteCallback(void)
{
	g_newMessageReceived = 1;
}

void UART_parseData(uint8 *str, uint8 *tempPtr, uint16 *distPtr, uint8 *win1Ptr, uint8 *win2Ptr)
{
	uint16 num = 0;
	uint8 i = 0;

	while(str[i] >= '0' && str[i] <= '9')
	{
		num = num * 10 + (str[i] - '0');
		i++;
	}
	*tempPtr = (uint8)num;

	while(str[i] == ' ') i++;

	num = 0;
	while(str[i] >= '0' && str[i] <= '9')
	{
		num = num * 10 + (str[i] - '0');
		i++;
	}
	*distPtr = num;

	while(str[i] == ' ') i++;

	if(str[i] == '1')
	{
		*win1Ptr = 1;
	}
	else
	{
		*win1Ptr = 0;
	}
	i++;

	while(str[i] == ' ') i++;

	if(str[i] == '1')
	{
		*win2Ptr = 1;
	}
	else
	{
		*win2Ptr = 0;
	}
}

void UART_parseErrors(uint8 *str, uint8 *errorList, uint8 *count)
{
	uint8 i = 0;
	*count = 0;

	while(str[i] != '\0' && str[i] != ' ') i++;
	if(str[i] == ' ') i++;

	if(str[i] == 'n' && str[i+1] == 'o' && str[i+2] == 'n' && str[i+3] == 'e')
	{
		*count = 0;
		return;
	}

	while(str[i] != '\0' && str[i] != '#' && *count < MAX_ERRORS)
	{
		if((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z'))
		{
			errorList[*count] = str[i];
			(*count)++;
		}
		i++;
	}
}

void displayErrorScroll(void)
{
	static uint16 lastScrollTick = 0;

	if(prevErrorCount != g_errorCount)
	{
		prevErrorCount = g_errorCount;
		lastScrollTick = 0;
		scrollPage = 0;
		endDisplayed = 0;
	}

	if(g_errorCount == 0)
	{
		LCD_moveCursor(1,0);
		LCD_displayString("No Logged Errors");
		LCD_moveCursor(2,0);
		LCD_displayString("                ");
		LCD_moveCursor(3,0);
		LCD_displayString("                ");
		return;
	}

	if(endDisplayed)
		return;

	uint16 t;
	cli();
	t = g_timerTicks;
	sei();

	if(g_forceFirstDisplay)
	{
		g_forceFirstDisplay = 0;
		lastScrollTick = t;
	}
	else
	{
		if((uint16)(t - lastScrollTick) < 1)
			return;
		lastScrollTick = t;
	}

	LCD_clearScreen();
	LCD_moveCursor(0, 0);
	LCD_displayString("Logged Errors:");

	uint8 errorsPerPage = 2;
	uint8 startIndex = scrollPage * errorsPerPage;

	if(startIndex >= g_errorCount)
	{
		LCD_moveCursor(1,0);
		LCD_displayString("-- End of List --");
		LCD_moveCursor(2,0);
		LCD_displayString("                ");
		LCD_moveCursor(3,0);
		LCD_displayString("                ");
		endDisplayed = 1;
		return;
	}

	for(uint8 i = 0; i < errorsPerPage; i++)
	{
		uint8 idx = startIndex + i;
		if(idx >= g_errorCount)
		{
			LCD_moveCursor(i + 1, 0);
			LCD_displayString("                ");
			continue;
		}

		LCD_moveCursor(i + 1, 0);
		LCD_intgerToString(idx + 1);
		LCD_displayCharacter('.');
		LCD_displayString("P00");

		uint8 code = g_errorList[idx];
		uint8 suffix = (code >= 'a') ? (code - 'a' + 1) : (code - 'A' + 1);
		LCD_displayCharacter(suffix + '0');
		LCD_displayString("                ");
	}

	scrollPage++;
}

void timerCallback(void)
{
	g_timerTicks++;
	if(g_timerTicks >= 10)
	{
		g_timeoutFlag = 1;
		g_timerTicks = 0;
	}
}
