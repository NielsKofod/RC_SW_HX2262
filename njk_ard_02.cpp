#include <Arduino.h>

extern HardwareSerial Serial;


void setup_int(int div)
{
    cli();//stop interrupts

    //set timer2 interrupt at 8kHz
    TCCR2A = 0;// set entire TCCR2A register to 0
    TCCR2B = 0;// same for TCCR2B
    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 8khz increments
    OCR2A = div;// = (16*10^6) / (20000*64) - 1 (must be <256)
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    // Set CS20 and CS21 bits for 64 prescaler
    TCCR2B |= (1 << CS21) | (1 << CS20);
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);

    sei();//allow interrupts

}

void setup() {
	pinMode(22, OUTPUT);
    Serial.begin(115200);
    Serial.setTimeout(1000*60*60*24);
    setup_int(120);
}


int group = 0;
int addr = 0;
int func = 0;
byte frame[16];
int frame_index = 0;
int byte_index = 0;
int repeat = 0;

void buildFrame(void)
{
	int i;
	for (i=0; i<12; i++)
		frame[i] = 0x8e; // Float

	for (i=0; i<5; i++)
	{
	  if (group & (1 << i))
		  frame[i] = 0x88; // '0'
	}
	frame[5+addr] = 0x88; // '0'
	if (func)
	{
		frame[10] = 0x88; // '0'
		frame[11] = 0xee; // '1'
	}
	else
	{
		frame[10] = 0xee; // '1'
		frame[11] = 0x88; // '0'
	}
	frame[12] = 0x80; // Sync
	frame[13] = 0x00;
	frame[14] = 0x00;
	frame[15] = 0x00;

	frame_index = 0;
	byte_index  = 0;
	repeat = 12; // trigger interrupt
}

bool decodeCmd(String cmd)
{
	String input = cmd;
	String sub;
	int pos = 0;
	if (input == "") return false;

	addr = 0;
	pos = input.indexOf(' ');
	if (pos == -1) return false;

	sub = input.substring(0, pos);
	group = sub.toInt();
	sub = input;
	input = sub.substring(pos+1);

	pos = input.indexOf(' ');
	if (pos == -1) return false;

	sub = input.substring(0, pos);
	addr = sub.toInt();
	sub = input;
	input = sub.substring(pos+1);

	func = input.toInt();
	if ((group < 0) || (group > 31)) return false;
	if ((addr  < 0) || (addr  > 4 )) return false;
	if ((func  < 0) || (func  > 1 )) return false;

	return true;
}

void loop() {

	if (decodeCmd(Serial.readStringUntil(0x0d)))
	{
		buildFrame();
		Serial.print("Group: ");
		Serial.print(group);
		Serial.print("   Addr: ");
		Serial.print(addr);
		Serial.print("   Func: ");
		Serial.println(func);
	}
}

ISR(TIMER2_COMPA_vect)
{
	if (repeat)
	{
		if (frame[frame_index] & (1 << (7-byte_index)))
		{
			digitalWrite(22,HIGH);
		}
		else
		{
		    digitalWrite(22,LOW);
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

