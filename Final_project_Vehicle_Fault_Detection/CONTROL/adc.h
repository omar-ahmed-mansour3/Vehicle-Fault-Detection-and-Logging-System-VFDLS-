 /******************************************************************************
 *
 * Module: ADC
 *
 * File Name: adc.h
 *
 * Description: header file for the ATmega32 ADC driver
 *
 *
 *******************************************************************************/

#ifndef ADC_H_
#define ADC_H_

#include "std_types.h"
#include "common_macros.h"

#include<avr/io.h>

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define ADC_MAXIMUM_VALUE    1023
//#define ADC_REF_VOLT_VALUE   2.56

typedef enum{
    AREF_REF = 0,      // External reference on AREF pin
    AVCC_REF=1,          // AVCC with capacitor at AREF pin
    INTERNAL_2_56V_REF=3 // Internal 2.56V reference
}ADC_ReferenceVolatge;

typedef enum {
    ADC_PRESCALER_2   = 1,
    ADC_PRESCALER_4   = 2,
    ADC_PRESCALER_8   = 3,
    ADC_PRESCALER_16  = 4,
    ADC_PRESCALER_32  = 5,
    ADC_PRESCALER_64  = 6,
    ADC_PRESCALER_128 = 7
} ADC_Prescaler;


typedef struct{
 ADC_ReferenceVolatge ref_volt;
 ADC_Prescaler prescaler;
}ADC_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Function responsible for initialize the ADC driver.
 */
void ADC_init(const ADC_ConfigType * Config_Ptr);

/*
 * Description :
 * Function responsible for read analog data from a certain ADC channel
 * and convert it to digital using the ADC driver.
 */
uint16 ADC_readChannel(uint8 channel_num);

#endif /* ADC_H_ */
