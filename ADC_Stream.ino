/*
#### change log####
version 1.0.6
.changed datastrings to strings with constant length

version 1.0.5
+own analog read function

version 1.0.4
.changed: readVcc just once before the other ADC reads start -> saves some conversion time


version 1.0.3
.changed ADC Prescaler to 64 (=>250kHz ADC Clock) -> ,ore stable readings
.commented out unused stuff and commands

version 1.0.2
.changed back to continuous timestamp


version 1.0.1
.change to delta time instead of continuous time for timestamp
.on restart Help massage is displayed
.minor cosmetic changes

version 1.0
code freeze from development

*/

/*
#include <JsonGenerator.h>
#include <JsonParser.h>

using namespace ArduinoJson::Generator;
*/
boolean DEBUG=false;
String version ="1.0.6";

//
#define FASTADC 1
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

long readA0;
long readA1;
long readA2;
int readA3;
int readA4;
int readA5;

int readAnalog;

String msg = "HH";

unsigned long lastTime = 0;
unsigned long currTime= 0;
long Power =0;
unsigned long currTime1 = 0;
unsigned long currTime2 = 0;
unsigned long meassureTime = 0;

static long buad = 115200;




long readVcc() {
	/* Source: https://code.google.com/p/tinkerit/wiki/SecretVoltmeter */
	long result;
	//! Read 1.1V reference against AVcc
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delay(2); //! Wait for Vref to settle
	ADCSRA |= _BV(ADSC); //! Convert
	while (bit_is_set(ADCSRA,ADSC));
	result = ADCL;
	result |= ADCH<<8;
	/* Back-calculate AVcc in mV.
	* V_Bandgap is approx. 1.119505859375V on my Arduino (labeled: MAIK)
	* V_Bandgap for Loadcontrollerboard: 1.08609275 => 1112159L
	* V_Bandgap for Measurementboard: 1.111953125 => 1138640L
	*/
	result = 1138640L/result;
	if(DEBUG)
	{
		Serial.print("Vcc: ");
		Serial.println(result);
	}
	
	/*
	/result returns is in mV (Millivolt)
	*/
	
	return result;
}

int adcReadA0()
{
	int result;
	ADMUX = (1<<REFS0) ;
	//ADCSRA = (1<<ADEN) | (1<<ADSC);
	ADCSRA |= (1<<ADEN) |(1<<ADSC);
	delayMicroseconds(1500);
	while(bit_is_set(ADCSRA,ADSC));
	result = ADCL;
	result |=ADCH<<8;
	//ADCSRA |= (0<<ADEN)
	return result;
}

int adcReadA1()
{
	int result;
	ADMUX = (1<<REFS0) | (1 << MUX0);; 
	ADCSRA |= (1<<ADEN) | (1<<ADSC);
	delayMicroseconds(1500);
	while(bit_is_set(ADCSRA,ADSC));
	result =ADCL;
	result |=ADCH<<8;
	return result;
}

int adcReadA2()
{
	int result;
	ADMUX = (1<<REFS0) | (1 << MUX1);
	ADCSRA |= (1<<ADEN) | (1<<ADSC);
	delayMicroseconds(1500);
	while(bit_is_set(ADCSRA,ADSC));
	result =ADCL;
	result |=ADCH<<8;
	return result;
}


long toVolt(int reading, long Vcc)
{
	//! Converts the ADC reading to Voltage.
	//! Consider the actual Supply voltage and the Voltage divider factor of 5
	///return returns milliVolts
	if (DEBUG)
	{
		Serial.print("V_raw: ");
		Serial.println(reading);
	}
	return long(Vcc*reading*5/1024);
	
}

long toCurrent(int reading, long Vcc)
{
	//! Converts the ADC reading to Current.
	//! Consider the actual Supply voltage
	///return returns milliAmps
	if (DEBUG)
	{
	Serial.print("I_raw: ");
	Serial.println(reading);
	}
	return long(Vcc*reading/1024);
	
}

long toRPM(int reading, long Vcc)
{
	//! Converts the ADC reading to RPM.
	//! Consider the actual Supply voltage and the Voltage divider factor of 5
	///return returns rpm
	
	long Voltage=Vcc*reading*5/1024; //! Voltage in milliVolts
	if (DEBUG)
	{
		
		Serial.print("N_raw: ");
		Serial.println(reading);
		Serial.print("Volts_RPM: ");
		Serial.println(Voltage);
	}
	long rpm = round(Voltage/1.5);		//! Conversion from Volts to rpm
	
	return rpm;
}



String constTimeStringLengthOf10(unsigned long timestamp)
{
	char charBuffer[11+1];
	
	sprintf(charBuffer,"%10lu",timestamp);
	return String(charBuffer);
}

String constTimeStringLengthOf5(long value)
{
	char charBuffer[6+1];
	
	sprintf(charBuffer,"%5ld",value);
	return String(charBuffer);
}

String constTimeStringLengthOf4(long value)
{
	char charBuffer[5+1];
	
	sprintf(charBuffer,"%4ld",value);
	return String(charBuffer);
}

void setup()
{
	#if FASTADC
	//! set prescale to 64
	sbi(ADCSRA,ADPS2);
	sbi(ADCSRA,ADPS1);
	cbi(ADCSRA,ADPS0);
	#endif

	//! Initialize Serial Communication
	Serial.begin(buad);
	Serial.println("Ready");

}

void loop()
{


	//! Read the Serial Data in for the commands
	if(Serial.available()>0)
	{
		msg="";
		while(Serial.available()>0)
		{
			msg+=char(Serial.read());
			delay(10);
	}
	msg = msg.substring(0,2);
	//serial.print("msg: ");
	//serial.println(msg);
	}
	

	//! Definition of the Serial Commands and the send back Data
	if (msg.equals("DD"))
	{
		
		DEBUG=!DEBUG;		
		msg= "AA";
		if (DEBUG)
		{
			Serial.println("Debug: On");
			
		}
		else
		{
			Serial.println("Debug: Off");
			
		}
		
	}
	//
	//else if (msg.equals("A0"))
	//{
		//currTime1 = micros();
		//readAnalog=analogRead(A0);
		//currTime2 = micros();
		//Serial.print("A0: ");
		//Serial.print(readAnalog,DEC);
		//Serial.print(", ");
		//Serial.print(currTime2-currTime1);
		//Serial.print(", ");
		//Serial.println(micros());
	//}
	//else if (msg.equals("A1"))
	//{
		//currTime1 = micros();
		//readAnalog=analogRead(A1);
		//currTime2 = micros();
		//Serial.print("A1: ");
		//Serial.print(readAnalog,DEC);
		//Serial.print(", ");
		//Serial.print(currTime2-currTime1);
		//Serial.print(", ");
		//Serial.println(micros());
	//}
	//else if (msg.equals("A2"))
	//{
		//currTime1 = micros();
		//readAnalog=analogRead(A2);
		//currTime2 = micros();
		//Serial.print("A2: ");
		//Serial.print(readAnalog,DEC);
		//Serial.print(", ");
		//Serial.print(currTime2-currTime1);
		//Serial.print(", ");
		//Serial.println(micros());
	//}
	//else if (msg.equals("A3"))
	//{
		//currTime1 = micros();
		//readAnalog=analogRead(A3);
		//currTime2 = micros();
		//Serial.print("A3: ");
		//Serial.print(readAnalog,DEC);
		//Serial.print(", ");
		//Serial.print(currTime2-currTime1);
		//Serial.print(", ");
		//Serial.println(micros());
	//}
//
	//else if (msg.equals("A4"))
	//{
		//currTime1 = micros();
		//readAnalog=analogRead(A4);
		//currTime2 = micros();
		//Serial.print("A4: ");
		//Serial.print(readAnalog,DEC);
		//Serial.print(", ");
		//Serial.print(currTime2-currTime1);
		//Serial.print(", ");
		//Serial.println(micros());
	//}
//
	//else if (msg.equals("A5"))
	//{
		//currTime1 = micros();
		//readAnalog=analogRead(A5);
		//currTime2 = micros();
		//Serial.print("A5: ");
		//Serial.print(readAnalog,DEC);
		//Serial.print(", ");
		//Serial.print(currTime2-currTime1);
		//Serial.print(", ");
		//Serial.println(micros());
	//}

	else if (msg.equals("AA"))
	{
		currTime2 = micros();
		//! Main Serial.
		//! Reads the first three Analog Inputs and converts them into their corresponding Unit
        //long VCC = readVcc();
		//readA0=toVolt(analogRead(A0),VCC);
		////delay(2);
		//readA1=toCurrent(analogRead(A1),VCC);
		////delay(2);
		//readA2=toRPM(analogRead(A2),VCC);
		long VCC = readVcc();
		readA0=toVolt(adcReadA0(),VCC);
		//delay(2);
		readA1=toCurrent(adcReadA1(),VCC);
		//delay(2);
		readA2=toRPM(adcReadA2(),VCC);
		//delay(2);
		Power =readA0*readA1/1000; //>in milliWatt
		//currTime = currTime2;
		//! Start Time measurement for measuring the loop length
		
		
		if(DEBUG)
		{			
			Serial.print("V: ");
			Serial.print(readA0);
			Serial.print("; ");
			Serial.print("I: ");
			Serial.print(readA1);
			Serial.print("; ");
			Serial.print("N: ");
			Serial.print(readA2);
			Serial.print("; ");
			Serial.print("P: ");
			Serial.print(Power);
			Serial.print("; ");
			Serial.print("t: ");
			Serial.println(currTime2);
		}
		else
		{
		/*
		JsonObject<5> dataItem;
		dataItem["voltage"] = readA0;
		dataItem["current"] = readA1;
		dataItem["rpm"] = readA2;
		dataItem["power"] = Power;
		dataItem["timestamp"] = currTime;
		dataItem.prettyPrintTo(Serial);
		Serial.println();
		Serial.println("EOL");
                */
                
                Serial.println("{");
                Serial.print("\t\"voltage\": ");
                Serial.print(constTimeStringLengthOf5(readA0));
                Serial.println(",");
                Serial.print("\t\"current\": ");
                Serial.print(constTimeStringLengthOf4(readA1));
                Serial.println(",");
                Serial.print("\t\"rpm\": ");
                Serial.print(constTimeStringLengthOf4(readA2));
                Serial.println(",");
                Serial.print("\t\"power\": ");
                Serial.print(constTimeStringLengthOf4(Power));
                Serial.println(",");
                Serial.print("\t\"timestamp\": ");
                Serial.println(constTimeStringLengthOf10(currTime2));
                Serial.println("}");
                Serial.println("EOL");

		}

		
	}
	else if (msg.equals("HH"))
	{Serial.println("Measurement Board");
		Serial.print("Firmware  Version:");
		Serial.println(version);
		Serial.println("Commands:");
		Serial.println("HH : prints this massage");
		Serial.println("AA : print Volt, Current, RPM, Power and a Timestamp (since last restart) in actual Units ([mV],[mA],[1/min],[mW],[us])");
		Serial.println("     Output is in JSON Style!!!");
		//Serial.println("Ax : prints only Analog Pin x (bit value (x: 0-5))");
		Serial.println("DD : toggle debug mode on and off");
		msg="";
		
		
		
	}
	else
	{
		msg="";
	}

}
