/*
   Inkplate5_Partial_Update_With_Deep_Sleep example for Soldered Inkplate 5
   For this example you will need only a USB-C cable and Inkplate 5
   Select "Soldered Inkplate5" from Tools -> Board menu.
   Don't have "Soldered Inkplate5" option? Follow our tutorial and add it:
   https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

   In this example we will show how to use partial update of epaper screen with deep sleep functionality of ESP32.
   This example is not same as Inkplate-basic_partial_update! Do not use Inkplate5_Partial_Update with deep sleep,
   IT WON'T WORK! Reason why you have to use this method with deep sleep is down how partial update works. It saves
   content from screen in RAM. By calling partialUpdate() function, code finds difference between what is currently on
   screen and what will be written and sends only that. When deep sleep is used, all content form the RAM has been
   deleted, so ESP32 doesn't know what is currently on the screen, so you have to "rewrite" what is currently on the
   screen, preload that and then create new screen that will be updated with partial update. NOTE: Partial update is
   only available on 1 Bit mode (BW) and it is not recommended to use it on first refresh after power up. It is
   recommended to do a full refresh every 5-10 partial refresh to maintain good picture quality.

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: https://forum.soldered.com/
   21 March 2023 by Soldered
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE5
#error "Wrong board selection for this example, please select Soldered Inkplate5 in the boards menu."
#endif

#include "Inkplate.h" // Include Inkplate library to the sketch
#include <rom/rtc.h>  // Include ESP32 library for RTC (needed for rtc_get_reset_reason() function)

#define uS_TO_S_FACTOR 1000000 // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  10      // Time how long ESP32 will be in deep sleep (in seconds). In this case 10 seconds.

Inkplate display(INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1-bit mode (BW)

// Counter variable (stored in RTC RAM that stores variable even if deep sleep is used)
// Variables that are changed after each partial update has to be stored in RTC RAM in order to recreate screen before
// deep sleep
RTC_DATA_ATTR int counter = 0;
RTC_DATA_ATTR float decimal = PI;

void setup()
{
    display.begin(); // Init Inkplate library (you should call this function ONLY ONCE)
    createScreen();  // Function that contains everything that has to be written on screen

    if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET) // Check if ESP32 is reseted by deep sleep or power up / user manual
                                                    // reset (or some other reason)
    {
        display
            .preloadScreen(); // If is woken up by deep sleep, recreate whole screen to be same as was before deep sleep
        counter++;            // Update variable / variables
        decimal *= 1.23;
        display.clearDisplay();      // Clear everything in buffer
        createScreen();              // Create new screen with new variables
        display.partialUpdate(true); // Partial update of screen. (Use this only in this
                                     // scenario, otherwise YOU CAN DAMAGE YOUR SCRREN)
    }
    else // If is not deep sleep reset, that must be some thing else, so use normal update procerude (full screen
         // update)
    {
        display.display();
    }
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP *
                                  uS_TO_S_FACTOR); // Set EPS32 to be woken up in 10 seconds (in this case)

    esp_deep_sleep_start(); // Put ESP32 into deep sleep (low power mode)
}

void loop()
{
    // Nothing should be here while using deep sleep!
}

void createScreen()
{
    display.setTextSize(3);           // Set font to be scaled up three times
    display.setCursor(35, 100);
    display.print("Inkplate 7 partial update with deep sleep example");
    display.setCursor(200, 250);      // Set text cursor @ X = 200, Y = 250
    display.print("First variable:"); // Write first variable to buffer
    display.print(counter, DEC);
    display.setCursor(200, 290);       // Set text cursor @ X = 200, Y = 290
    display.print("Second variable:"); // Write second variable to buffer (use two decimals places)
    display.print(decimal, 2);
}
