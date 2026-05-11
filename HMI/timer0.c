/*
 * timer0.c
 *
 *  Created on: Oct 17, 2025
 *      Author: Omar Mansour
 */

#include "timer0.h"


/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/
static volatile void(*g_Timer0CallBackPtr)(void) = NULL_PTR;
static volatile void(*g_Timer1CallBackPtr)(void) = NULL_PTR;
static volatile void(*g_Timer2CallBackPtr)(void) = NULL_PTR;

void Timer_init(const Timer_ConfigType * Config_Ptr)
{
	/*******************************************************************************
	 *								 TIMER0											*
	 ********************************************************************************/
	if(Config_Ptr->timer_ID==timer0)
	{
		TCCR0=0;
		TCCR0 |= (1<<FOC0);/* non-pwm mode*/
		TCNT0 = Config_Ptr->timer_InitialValue;


		switch(Config_Ptr->timer_mode)
		{
		case overflow :
			TIMSK|= (1<<TOIE0); //enable overflow interrupt
			break;

		case compare:
			TCCR0 |= (1<<WGM01); // adjusting timer mode
			OCR0 = Config_Ptr->timer_compare_MatchValue; // compare value
			TIMSK|= (1<<OCIE0); //enable compare interrupt

			break;
		}



		TCCR0|= Config_Ptr->timer_clock;

	}

	/*******************************************************************************
	 *								 TIMER1											*
	 ********************************************************************************/

	else if(Config_Ptr->timer_ID==timer1)
	{
		TCCR1A =0; //don't react to compare match
		TCCR1B = 0;
		TCNT1 = Config_Ptr->timer_InitialValue;

		TCCR1A |=(1<<FOC1A) | (1<<FOC1B); // non-pwm mode

		switch(Config_Ptr->timer_mode)
		{
		case overflow :
			TIMSK|=(1<<TOIE1); // enable overflow interrupt
			break;

		case compare:
			TCCR1B |= (1<<WGM12); // adjusting timer mode
			OCR1A = Config_Ptr->timer_compare_MatchValue; // compare value
			TIMSK|= (1<<OCIE1A); //enable compare interrupt

			break;
		}

		TCCR1B|=Config_Ptr->timer_clock;

	}
	/*******************************************************************************
	 *								 TIMER2											*
	 ********************************************************************************/
	else if(Config_Ptr->timer_ID==timer2)
	{
		TCCR2=0;
		TCCR2|= (1<<FOC2);
		TCNT2 = Config_Ptr->timer_InitialValue;

		switch(Config_Ptr->timer_mode)
		{
		case overflow :
			TIMSK|=(1<<TOIE2);//enable overflow interrupt
			break;

		case compare:
			TCCR2 |= (1<<WGM21); // adjusting timer mode
			OCR2 = Config_Ptr->timer_compare_MatchValue; // compare value
			TIMSK|= (1<<OCIE2); //enable compare interrupt

			break;
		}

		TCCR2 |= Config_Ptr->timer_clock;

	}

}



void Timer_deInit(Timer_ID_Type timer_type)
{
	/*******************************************************************************
	 *								 TIMER0											*
	 ********************************************************************************/
	if(timer_type==timer0)
	{
		TCCR0=0;
		TCNT0 = 0;
		TIMSK &= ~(1<<TOIE0); //disable overflow interrupt
		OCR0 = 0; // compare value
		TIMSK &= ~(1<<OCIE0); //disable compare interrupt
	}

	/*******************************************************************************
	 *								 TIMER1											*
	 ********************************************************************************/

	else if(timer_type==timer1)
	{
		TCCR1A =0; //don't react to compare match
		TCCR1B = 0;
		TCNT1 = 0; // timer initial value
		TIMSK &=~ (1<<TOIE1); // disable overflow interrupt
		OCR1A = 0; // compare value
		TIMSK &= ~(1<<OCIE1A); //disable compare interrupt




	}
	/*******************************************************************************
	 *								 TIMER2											*
	 ********************************************************************************/
	else if(timer_type==timer2)
	{
		TCCR2=0;
		TCNT2 = 0;
		TIMSK &= ~(1<<TOIE2);// disable overflow interrupt
		OCR2 = 0; // compare value
		TIMSK &= ~ (1<<OCIE2); //disable compare interrupt
	}

}

void Timer_setCallBack(void(*a_ptr)(void), Timer_ID_Type a_timer_ID )
{
	switch (a_timer_ID)
	{
	case timer0:
		g_Timer0CallBackPtr=a_ptr;
		break;

	case timer1:
		g_Timer1CallBackPtr=a_ptr;
		break;

	case timer2:
		g_Timer2CallBackPtr=a_ptr;
		break;
	}
}


ISR(TIMER0_OVF_vect)
{
	(*g_Timer0CallBackPtr)();
}
ISR(TIMER0_COMP_vect)
{
	(*g_Timer0CallBackPtr)();
}


ISR(TIMER1_OVF_vect)
{
	(*g_Timer1CallBackPtr)();
}
ISR(TIMER1_COMPA_vect)
{
	(*g_Timer1CallBackPtr)();
}


ISR(TIMER2_OVF_vect)
{
	(*g_Timer2CallBackPtr)();
}
ISR(TIMER2_COMP_vect)
{
	(*g_Timer2CallBackPtr)();
}




