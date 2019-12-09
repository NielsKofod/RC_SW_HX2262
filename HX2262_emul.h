/*
 * HX2262_emul.h
 *
 *  Created on: Dec 9, 2019
 *      Author: Niels Kofod
 */

#ifndef HX2262_EMUL_H_
#define HX2262_EMUL_H_

#include <Arduino.h>

// This module emulates the chip HX2262, which is used for cheap remote control devices.
// It is developed and tested with a 433.92MHz radio module and a set of 220V remote switches.
//
// Resurces used:
//  - Timer_2
//  - One GPIO pin

// Define the GPIO pin to use for the data stream.
#define DATA_PIN 22

// Set the number of frames send by the send function. Must be bigger than 4 according to the HX2262 data sheet
#define FRAME_CNT 12

// Init and setup interrupt
void HX2262_init (void);

// Send the data frame defined by the input parameters
// The number of frame repetitions is  defined by FRAME_CNT
// dev_addr(0..31)  Device address - Typically set by DIP switch on the transmitter remote
// sub_addr(0..4)   Sub address - Typically button A, B, C ..
// state(0..1)      State - '0' is OFF and '1' is ON
//
// Return: false if input validation faile
bool HX2262_send (int dev_addr, int sub_addr, int state);

// Return: true if the send interrupt routine is active
bool HX2262_is_busy (void);

#endif /* HX2262_EMUL_H_ */
