/*
   Inkplate7_Internal_IO_Expander example for Soldered Inkplate 7
   For this example you will need a USB-C cable, Inkplate7, 330 Ohm resistor and LED diode.
   Select "Soldered Inkplate7" from Tools -> Board menu.
   Don't have "Soldered Inkplate7" option? Follow our tutorial and add it:
   https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

   Only pins P1-1 and P1-2 will not work without cutting the jumpers JP2 and JP3 and soldering
   them on the other side because it's used for charging LED and controlling MOSFET for an SD card.

   Connect resistor to P1-7 on IO expander header at the top on the backside
   (component side) of Inkplate. You will have to connect one side of 330 Ohm resistor to P1-7, then other side
   to anode of LED and finally, cathode pin of LED to GND.

   This example will show you how you can manipulate with I/Os of internal IO Expander.
   If everything is connected ok, after you upload code, LED should blink. Nothing will 
   be displayed on the screen, the content will remain unchanged.

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: https://forum.soldered.com/
   26 April 2023 by Soldered.
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE7
#error "Wrong board selection for this example, please select Soldered Inkplate7 in the boards menu."
#endif

// Include Inkplate library to the sketch
#include "Inkplate.h" 

// We are going to use pin P1-7 
// Remember! P0-0 = 0, P0-1 = 1, ..., P0-7 = 7, P1-0 = 8, P1-1 = 9, ..., P1-7 = 15
#define LED_PIN IO_PIN_B7 // 7 pin on port B on the expander is P1-7. You can just type 15 here

// Create an object on Inkplate library
Inkplate display; 

void setup()
{
    // Init Inkplate library (you should call this function ONLY ONCE)
    display.begin(); 

    // Set P1-7 to output. On that pin, we sholud connect LED with current 
    // limiting resistor and specify that we want use internal IO expander
    display.pinModeIO(LED_PIN, OUTPUT, IO_INT_ADDR); 
}

void loop()
{
    display.digitalWriteIO(LED_PIN, LOW, IO_INT_ADDR);  // Set output to low (LED does not light up)
    delay(1000);                                        // Wait for one second
    display.digitalWriteIO(LED_PIN, HIGH, IO_INT_ADDR); // Set output to high (LED lights up)
    delay(1000);                                        // Wait for one second
}
