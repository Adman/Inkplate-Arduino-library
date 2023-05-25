/*
   Inkplate7_SD_TXT_Read example for Soldered Inkplate 7
   For this example you will need a USB-C cable, Inkplate 7, and a micro SD card
   loaded with text.txt file that can be found inside folder of this example.
   Select "Soldered Inkplate7" from Tools -> Board menu.
   Don't have "Soldered Inkplate7" option? Follow our tutorial and add it:
   https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

   This example will show you how to open .txt files, limit the number of characters
   to read, and display the content of that file on the Inkplate e-paper display.
   You can open any other .txt file, just make sure that if there are more than 500 chars,
   increase the MAX_LENGTH to your needs. You can also change the file name.
   You can see on the Serial Monitor at 115200 baud rate what's happening.

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: https://forum.soldered.com/
   26 April 2023 by Soldered
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE7
#error "Wrong board selection for this example, please select Soldered Inkplate7 in the boards menu."
#endif

#include "Inkplate.h" // Include Inkplate library to the sketch
Inkplate display;     // Create an object on Inkplate library
SdFile file;          // Create SdFile object used for accessing files on SD card

// Define how many characters will be read from the .txt file. Change if you want to read larger files
#define MAX_LENGTH 500

// Here is the specified name of the file
const char fileName[] = "/text.txt";

void setup()
{
    Serial.begin(115200);   // Init serial communication at 115200 baud rate
    display.begin();        // Init Inkplate library (you should call this function ONLY ONCE)
    display.clearDisplay(); // Clear frame buffer of display
    display.setTextSize(3); // Set text to be 3 times bigger than original (5x7 px)

    // Init SD card. Display if SD card is init propery or not.
    if (display.sdCardInit())
    {
        display.println("SD Card ok! Reading data...");
        Serial.println("SD Card ok! Reading data...");

        // Try to load text with a max length of 500 chars. You can change this by changing MAX_LENGTH define
        if (!file.open(fileName, O_RDONLY))
        {
            // If it fails to open, send error message to display and Serial, otherwise read the
            // file.
            display.println("File open error");
            Serial.println("File open error");
        }
        else
        {
            display.clearDisplay();    // Clear everything that is stored in frame buffer of epaper
            display.setCursor(0, 0);   // Set print position at the begining of the screen
            char text[MAX_LENGTH + 1]; // An array where data from an SD card is stored (max 500 chars here plus one for
                                       // the null terminating char)
            int len = file.fileSize(); // Read how big is file that we are opening
            if (len > MAX_LENGTH)
                len = MAX_LENGTH;  // If it's more than 500 bytes (500 chars), limit to max 500 bytes
            file.read(text, len);  // Read 500 chars from the file and save it in the text array
            text[len] = 0;         // Put the null terminating char at the end of data
            Serial.print(text);    // Print data on the serial
            display.print(text);   // Print data/text
        }
    }
    else
    {
        // If card init was not successful, display error on screen.
        // It is a good practice to stop the program here because of SD failure (for example 
        // using an infinite empty loop) but here nothing happens afterward, so the loop() has that function.
        Serial.println("SD Card error!");
        display.println("SD Card error!");
    }

    // Show everything on the screen
    display.display();

    // Put the SD card in sleep mode (turn off the MOSFET which turns it on)
    display.sdCardSleep();
}

void loop()
{
    // Nothing because we want to run this sketch only once
}
