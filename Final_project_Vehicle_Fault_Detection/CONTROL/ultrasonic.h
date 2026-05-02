
#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

/****************************************Includes****************************************/

#include "std_types.h"

/***************************************Definitions**************************************/

/* Configuration of port and pin, which the Ultrasonic sensor Trigger pin is connected to. */
#define TRIGGER_PORT                       PORTD_ID
#define TRIGGER_PIN                        PIN7_ID

/* Ultrasonic sensor Echo pin is connected to the input capture pin of the micro-controller. */
#define ECHO_PORT                          ICU_PORT
#define ECHO_PIN                           ICU_PIN

/* Constants needed by the distance calculations. */
#define SOUND_VELOCITY_CM                  ((uint32)34300)
#define SOUND_VELOCITY_CM_DIVIDETWO        (SOUND_VELOCITY_CM/2)
/* Calculating the time of one cycle/tick. */
#define PRESCLAER                          8
#define F_TIMER                            ((uint32)(F_CPU/PRESCLAER))
#define T_TICK                             (((double)1/F_TIMER))

/**********************************Functions Prototypes**********************************/

/*
 * Description:
 * Initialize the Ultrasonic sensor:
 * 1- Setup the Setup the direction of the trigger pin as output pin.
 * 2- Setup the ICU call back function.
 * 3- Initialize the ICU driver as required.
 */
void Ultrasonic_init(void);

/*
 * Description:
 * 1- Send the trigger pulse by using Ultrasonic_Trigger function.
 * 2- Wait till the Echo pin high time is calculated.
 * 3- Calculate the distance.
 */
uint16 Ultrasonic_readDistance(void);


#endif /* ULTRASONIC_H_ */
