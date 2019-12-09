#include <Arduino.h>

#include "HX2262_emul.h"

extern HardwareSerial Serial;

void setup()
{
  cli(); // Disable interrupts
  Serial.begin(115200);
  Serial.setTimeout(1000 * 60 * 60 * 24);

  HX2262_init();
  sei(); // Enable interruts
}

int addr;
int group;
int func;
bool decodeCmd(String cmd)
{
  String input = cmd;
  String sub;
  int pos = 0;
  if (input == "")
    return false;

  addr = 0;
  pos = input.indexOf(' ');
  if (pos == -1)
    return false;

  sub = input.substring(0, pos);
  group = sub.toInt();
  sub = input;
  input = sub.substring(pos + 1);

  pos = input.indexOf(' ');
  if (pos == -1)
    return false;

  sub = input.substring(0, pos);
  addr = sub.toInt();
  sub = input;
  input = sub.substring(pos + 1);

  func = input.toInt();
  if ((group < 0) || (group > 31))
    return false;
  if ((addr < 0) || (addr > 4))
    return false;
  if ((func < 0) || (func > 1))
    return false;

  return true;
}

void loop()
{

  if (decodeCmd(Serial.readStringUntil(0x0d)))
  {
    HX2262_send(group, addr, func);
    Serial.print("Group: ");
    Serial.print(group);
    Serial.print("   Addr: ");
    Serial.print(addr);
    Serial.print("   Func: ");
    Serial.println(func);
  }
}

