/*
 * HX2262_emul.c
 *
 *  Created on: Dec 9, 2019
 *      Author: niels
 */

#include <Arduino.h>

#include "HX2262_emul.h"

// Define the bit symbols used to build the data frame
// These are derived from the HX2262 data sheet.
#define SYMBOL_LOW   0x88
#define SYMBOL_HIGH  0xee
#define SYMBOL_FLOAT 0x8e
#define SYMBOL_SYNC  0x80
#define SYMBOL_ZERO  0x00

byte frame[16];
int frame_index = 0;
int byte_index = 0;
int repeat = 0;

void HX2262_init(void)
{
  pinMode(DATA_PIN, OUTPUT);

  // set timer2 interrupt at 2kHz
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2 = 0; //initialize counter value to 0
  // set compare match register
  OCR2A = 124;   // = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS20 and CS21 bits for 64 prescaler
  TCCR2B |= (1 << CS21) | (1 << CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}

bool HX2262_send(int dev_addr, int sub_addr, int state)
{
  int i;

  // validate input
  if ((dev_addr < 0) || (dev_addr > 31))
    return false;
  if ((sub_addr < 0) || (sub_addr > 4))
    return false;
  if ((state < 0) || (state > 1))
    return false;

  for (i = 0; i < 12; i++)
    frame[i] = SYMBOL_FLOAT;

  for (i = 0; i < 5; i++)
  {
    if (dev_addr & (1 << i))
      frame[i] = SYMBOL_LOW;
  }
  frame[5 + sub_addr] = SYMBOL_LOW;
  if (state)
  {
    frame[10] = SYMBOL_LOW;
    frame[11] = SYMBOL_HIGH;
  }
  else
  {
    frame[10] = SYMBOL_HIGH;
    frame[11] = SYMBOL_LOW;
  }
  frame[12] = SYMBOL_SYNC;
  frame[13] = SYMBOL_ZERO;
  frame[14] = SYMBOL_ZERO;
  frame[15] = SYMBOL_ZERO;

  frame_index = 0;
  byte_index = 0;
  repeat = FRAME_CNT; // trigger interrupt
  return true;
}

bool HX2262_is_busy(void)
{
  return (repeat != 0);
}

ISR(TIMER2_COMPA_vect)
{
  if (repeat)
  {
    if (frame[frame_index] & (1 << (7 - byte_index)))
    {
      digitalWrite(DATA_PIN, HIGH);
    }
    else
    {
      digitalWrite(DATA_PIN, LOW);
    }
    byte_index++;
    if (byte_index > 7)
    {
      byte_index = 0;
      frame_index++;
    }
    if (frame_index > 15)
    {
      frame_index = 0;
      repeat--;
    }
  }
}
