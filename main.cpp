#include "mbed.h"
#include "arm_book_lib.h"

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

AnalogIn potentiometer(A0);
AnalogIn lm35(A1); // 10 mV/\xB0 C
PwmOut buzzer(D9);
DigitalIn mq2(A3);

bool quit = false;

char receivedChar = '\0';

float lm35Reading = 0.0; // Raw ADC input A0 value
float lm35TempC = 0.0;   // Temperature in Celsius degrees [\xB0 C]
float lm35TempF = 0.0;   // Temperature in Fahrenheit degrees [\xB0 F]

float potentiometerReading = 0.0;   // Raw ADC input A1 value
float potentiometerScaledToC = 0.0; // Potentiometer value scaled to Celsius degrees [\xB0 C]
float potentiometerScaledToF = 0.0; // Potentiometer value scaled to Fahrenheit degrees [\xB0 F]

void availableCommands();
void uartTask();
void pcSerialComStringWrite(const char* str);
char pcSerialComCharRead();

float analogReadingScaledWithTheLM35Formula(float analogReading);;
float celsiusToFahrenheit(float tempInCelsiusDegrees);
float analogValueToFahrenheit(float analogValue);
float potentiometerScaledToCelsius(float analogValue);
float potentiometerScaledToFahrenheit(float analogValue);

int main()
{
    availableCommands();
    while( true ) {
        uartTask();
    }
}

void availableCommands()
{
    pcSerialComStringWrite( "\r\nPress the following keys to continuously " );
    pcSerialComStringWrite( "print the readings until 'q' is pressed:\r\n" );

    pcSerialComStringWrite(" - 'a' the reading at the analog pin A0 ");
    pcSerialComStringWrite("(connected to the potentiometer)\r\n");

    pcSerialComStringWrite(" - 'b' the reading at the analog pin A1 ");
    pcSerialComStringWrite("(connected to the LM35)\r\n");

    pcSerialComStringWrite(" - 'c' the reading of the temperature measured ");
    pcSerialComStringWrite("by the LM35 expressed in \xB0 C\r\n");

    pcSerialComStringWrite(" - 'd' the reading of the temperature measured ");
    pcSerialComStringWrite("by the LM35 expressed in \xB0 F\r\n");

    pcSerialComStringWrite(" - 'e' the reading of the temperature measured ");
    pcSerialComStringWrite("by the LM35 expressed in \xB0 C and the potentiometer ");
    pcSerialComStringWrite("reading scaled by the same factor\r\n");

    pcSerialComStringWrite(" - 'f' the reading of the temperature measured ");
    pcSerialComStringWrite("by the LM35 expressed in \xB0 F and the potentiometer ");
    pcSerialComStringWrite("reading scaled by the same factor\r\n");

    pcSerialComStringWrite(" - 'g' the reading of the DOUT signal of the ");
    pcSerialComStringWrite("MQ-2 gas sensor \r\n");

    pcSerialComStringWrite("\r\nWARNING: The readings are printed continuously ");
    pcSerialComStringWrite("until 'q' or 'Q' are pressed.\r\n\r\n");
}

void uartTask()
{
    char receivedChar = '\0';
    char str[100] = "";
    receivedChar = pcSerialComCharRead();
    if( receivedChar !=  '\0') {
        switch (receivedChar) {

        case 'a':
        case 'A':
            while( !(receivedChar == 'q' || receivedChar == 'Q') ) {
                potentiometerReading = potentiometer.read();
                str[0] = '\0';
                sprintf ( str, "Potentiometer reading: %.2f\r\n", potentiometerReading);
                pcSerialComStringWrite( str );
                delay( 200 );
                receivedChar = pcSerialComCharRead();
            }
            break;

        case 'b':
        case 'B':
            while( !(receivedChar == 'q' || receivedChar == 'Q') ) {
                lm35Reading = lm35.read();
                str[0] = '\0';
                sprintf ( str, "LM35 reading: %.2f\r\n", lm35Reading);
                pcSerialComStringWrite( str );
                delay( 200 );
                receivedChar = pcSerialComCharRead();
            }
            break;

        case 'c':
        case 'C':
            while( !(receivedChar == 'q' || receivedChar == 'Q') ) {             
                lm35Reading = lm35.read();
                lm35TempC = analogReadingScaledWithTheLM35Formula(lm35Reading);
                str[0] = '\0';
                sprintf ( str, "LM35: %.2f \xB0 C\r\n", lm35TempC);
                pcSerialComStringWrite( str );
                delay( 200 );
                receivedChar = pcSerialComCharRead();
            }
            break;

        case 'd':
        case 'D':
            while( !(receivedChar == 'q' || receivedChar == 'Q') ) {
                lm35Reading = lm35.read();
                lm35TempC = analogReadingScaledWithTheLM35Formula(lm35Reading);
                lm35TempF = celsiusToFahrenheit(lm35TempC);
                str[0] = '\0';
                sprintf ( str, "LM35: %.2f \xB0 F\r\n", lm35TempF);
                pcSerialComStringWrite( str );                
                delay( 200 );
                receivedChar = pcSerialComCharRead();
            }
            break;

        case 'e':
        case 'E':
            while( !(receivedChar == 'q' || receivedChar == 'Q') ) {
                potentiometerReading = potentiometer.read();
                potentiometerScaledToC = potentiometerScaledToCelsius(potentiometerReading);
                lm35Reading = lm35.read();
                lm35TempC = analogReadingScaledWithTheLM35Formula(lm35Reading);
                str[0] = '\0';
                sprintf ( str, "LM35: %.2f \xB0 C, Potentiometer scaled to \xB0 C: %.2f\r\n",
                               lm35TempC, potentiometerScaledToC);
                pcSerialComStringWrite( str );   
                delay( 200 );
                receivedChar = pcSerialComCharRead();
            }
            break;

        case 'f':
        case 'F':
            while( !(receivedChar == 'q' || receivedChar == 'Q') ) {
                potentiometerReading = potentiometer.read();
                potentiometerScaledToF = potentiometerScaledToFahrenheit(potentiometerReading);
                lm35Reading = lm35.read();
                lm35TempC = analogReadingScaledWithTheLM35Formula(lm35Reading);
                lm35TempF = celsiusToFahrenheit(lm35TempC);
                str[0] = '\0';
                sprintf ( str, "LM35: %.2f \xB0 F, Potentiometer scaled to \xB0 F: %.2f\r\n",
                               lm35TempF, potentiometerScaledToF);
                pcSerialComStringWrite( str );  
                delay( 200 );
                receivedChar = pcSerialComCharRead();
            }
            break;

        case 'g':
        case 'G':
            while( !(receivedChar == 'q' || receivedChar == 'Q') ) {
                if (!mq2) {
                    pcSerialComStringWrite("Gas is being detected\r\n");
                    buzzer.period(1.0/100.0);
                    buzzer = 10;
                    delay(1);
                    buzzer.period(1.0/105);
                    buzzer = 10;
                    delay(1);
                    buzzer = 0;                       
                } else {
                    pcSerialComStringWrite("Gas is not being detected\r\n");
                }
                delay(200);
                receivedChar = pcSerialComCharRead();
            }
            break;

        default:
            availableCommands();
            break;
        }
    }
}

float analogReadingScaledWithTheLM35Formula(float analogReading)
{
    return analogReading * 330.0;
}

float celsiusToFahrenheit(float tempInCelsiusDegrees)
{
    return 9.0/5.0 * tempInCelsiusDegrees + 32.0;
}

float potentiometerScaledToCelsius(float analogValue)
{
    return 148.0 * analogValue + 2.0;
}

float potentiometerScaledToFahrenheit(float analogValue)
{
    return celsiusToFahrenheit(potentiometerScaledToCelsius(analogValue));
}

void pcSerialComStringWrite(const char* str)
{
    uartUsb.write(str, strlen(str));
}

char pcSerialComCharRead()
{
    char receivedChar = '\0';
    if(uartUsb.readable()) {
        uartUsb.read(&receivedChar, 1);
    }
    return receivedChar;
}