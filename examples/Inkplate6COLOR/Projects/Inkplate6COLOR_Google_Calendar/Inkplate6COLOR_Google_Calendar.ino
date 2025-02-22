/*
   Inkplate6COLOR_Google_Calendar example for Soldered Inkplate 6COLOR
   For this example you will need only USB cable and Inkplate 6COLOR.
   Select "Soldered Inkplate 6COLOR" from Tools -> Board menu.
   Don't have "Soldered Inkplate 6COLOR" option? Follow our tutorial and add it:
   https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

   This project shows you how Inkplate 6COLOR can be used to display
   events in your Google Calendar using their provided API

   For this to work you need to change your timezone, wifi credentials and your private calendar url
   which you can find following these steps:

    1. Open your google calendar
    2. Click the 3 menu dots of the calendar you want to access at the bottom of left hand side
    3. Click 'Settings and sharing'
    4. Navigate to 'Integrate Calendar'
    5. Take the 'Secret address in iCal format'

   (https://support.google.com/calendar/thread/2408874?hl=en)

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: https://forum.soldered.com/
   3 August 2020 by Soldered
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATECOLOR
#error "Wrong board selection for this example, please select Soldered Inkplate 6COLOR in the boards menu."
#endif

// Include Inkplate library to the sketch
#include "Inkplate.h"

// Including fonts
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

// Includes
#include "Network.h"
#include <algorithm>
#include <ctime>

// CHANGE HERE ---------------

char ssid[] = "";
char pass[] = "";
char calendarURL[] = "";

int timeZone = 2;

// Set to 3 to flip the screen 180 degrees
#define ROTATION 1

//---------------------------

// Delay between API calls
#define DELAY_MS 15 * 60 * 1000

// Initiate out Inkplate object
Inkplate display;

// Our networking functions, see Network.cpp for info
Network network;

// Variables for time and raw event info
char date[64];
char *data;

// Set background of first task to color green
int currentColor = 2;

// Struct for storing calender event info
struct entry
{
    char name[128];
    char time[128];
    char location[128];
    int day = -1;
    int timeStamp;
};

// Here we store calendar entries
int entriesNum = 0;
entry entries[100];

// All our functions declared below setup and loop
void drawInfo();
void drawGrid();
void getToFrom(char *dst, char *from, char *to, int *day, int *timeStamp);
bool drawEvent(entry *event, int day, int beginY, int maxHeigth, int *heigthNeeded);
int cmp(const void *a, const void *b);
void drawData();

void setup()
{
    Serial.begin(115200);

    data = (char *)ps_malloc(2000000LL);

    // Initial display settings
    display.begin();

    display.clearDisplay();
    display.setRotation(ROTATION);
    display.setTextWrap(false);
    display.setTextColor(0, 7);

    // Connect Inkplate to the WiFi network
    network.begin(ssid, pass);

    // Get the data from Google Calendar
    // Repeat attempts until data is fully downloaded
    Serial.println("Getting data... ");
    while (!network.getData(calendarURL, data))
    {
        delay(1000);
    }

    // Initial screen clearing
    display.clearDisplay();

    // Drawing all data, functions for that are below
    drawInfo();
    drawGrid();
    drawData();

    // Display the data on the screen
    display.display();

    // Go to sleep before checking again
    esp_sleep_enable_timer_wakeup(1000L * DELAY_MS);
    
     

    // Start deep sleep (this function does not return). Program stops here.
    esp_deep_sleep_start();
}

void loop()
{
    // Never here! If you are using deep sleep, the whole program should be in setup() because the board restarts each
    // time. loop() must be empty!
}

// Function for drawing calendar info
void drawInfo()
{
    // Setting font and color
    display.setTextColor(0, 7);
    display.setFont(&FreeSans12pt7b);
    display.setTextSize(1);

    display.setCursor(20, 20);

    // Find email in raw data
    char temp[64];
    char *start = strstr(data, "X-WR-CALNAME:");

    // If not found return
    if (!start)
        return;

    // Find where it ends
    start += 13;
    char *end = strchr(start, '\n');

    strncpy(temp, start, end - start - 1);
    temp[end - start - 1] = 0;

    // Print it
    display.println(temp);
}

// Draw lines in which to put events
void drawGrid()
{
    // upper left and low right coordinates
    int x1 = 3, y1 = 30;
    int x2 = 448 - 3, y2 = 598;

    // header size, for day info
    int header = 30;

    // Columns and rows
    int n = 1, m = 3;

    // Line drawing
    display.drawThickLine(x1, y1 + header, x2, y1 + header, 0, 2.0);
    for (int i = 0; i < n + 1; ++i)
    {
        display.drawThickLine(x1, (int)((float)y1 + (float)i * (float)(y2 - y1) / (float)n), x2,
                              (int)((float)y1 + (float)i * (float)(y2 - y1) / (float)n), 0, 2.0);
    }
    for (int i = 0; i < m + 1; ++i)
    {
        display.drawThickLine((int)((float)x1 + (float)i * (float)(x2 - x1) / (float)m), y1,
                              (int)((float)x1 + (float)i * (float)(x2 - x1) / (float)m), y2, 0, 2.0);
        display.setFont(&FreeSans9pt7b);

        // Display day info using time offset
        char temp[64];
        network.getTime(temp, i * 3600L * 24);
        temp[10] = 0;

        // calculate where to put text and print it
        display.setCursor(17 + (int)((float)x1 + (float)i * (float)(x2 - x1) / (float)m) + 15, y1 + header - 9);
        display.println(temp);
    }
}

// Format event times, example 13:00 to 14:00
void getToFrom(char *dst, char *from, char *to, int *day, int *timeStamp)
{
    // ANSI C time struct
    struct tm ltm = {0}, ltm2 = {0};
    char temp[128], temp2[128];
    strncpy(temp, from, 16);
    temp[16] = 0;

    // https://github.com/esp8266/Arduino/issues/5141, quickfix
    memmove(temp + 5, temp + 4, 16);
    memmove(temp + 8, temp + 7, 16);
    memmove(temp + 14, temp + 13, 16);
    memmove(temp + 16, temp + 15, 16);
    temp[4] = temp[7] = temp[13] = temp[16] = '-';

    // time.h function
    strptime(temp, "%Y-%m-%dT%H-%M-%SZ", &ltm);

    // create start and end event structs
    struct tm event, event2;
    time_t epoch = mktime(&ltm) + (time_t)timeZone * 3600L;
    gmtime_r(&epoch, &event);
    strncpy(dst, asctime(&event) + 11, 5);

    dst[5] = '-';

    strncpy(temp2, to, 16);
    temp2[16] = 0;

    // Same as above

    // https://github.com/esp8266/Arduino/issues/5141, quickfix
    memmove(temp2 + 5, temp2 + 4, 16);
    memmove(temp2 + 8, temp2 + 7, 16);
    memmove(temp2 + 14, temp2 + 13, 16);
    memmove(temp2 + 16, temp2 + 15, 16);
    temp2[4] = temp2[7] = temp2[13] = temp2[16] = '-';

    strptime(temp2, "%Y-%m-%dT%H-%M-%SZ", &ltm2);

    time_t epoch2 = mktime(&ltm2) + (time_t)timeZone * 3600L;
    gmtime_r(&epoch2, &event2);
    strncpy(dst + 6, asctime(&event2) + 11, 5);

    dst[11] = 0;

    char day0[64], day1[64], day2[64];

    // Find UNIX timestamps for next days to see where to put event
    network.getTime(day0, 0);
    network.getTime(day1, 24 * 3600);
    network.getTime(day2, 48 * 3600);

    *timeStamp = epoch;

    // Getting the time from our function in Network.cpp
    network.getTime(temp);
    if (strncmp(day0, asctime(&event), 10) == 0)
        *day = 0;
    else if (strncmp(day1, asctime(&event), 10) == 0)
        *day = 1;
    else if (strncmp(day2, asctime(&event), 10) == 0)
        *day = 2;
    else // event not in next 3 days, don't display
        *day = -1;
}

// Function to draw event
bool drawEvent(entry *event, int day, int beginY, int maxHeigth, int *heigthNeeded)
{
    // Upper left coordintes
    int x1 = 3 + 4 + (440 / 3) * day;
    int y1 = beginY + 3;

    // Setting text font
    display.setFont(&FreeSans12pt7b);

    // Some temporary variables
    int n = 0;
    char line[128];

    // Insert line brakes into setTextColor
    int lastSpace = -100;
    display.setCursor(x1 + 5, beginY + 26);
    for (int i = 0; i < min((size_t)64, strlen(event->name)); ++i)
    {
        // Copy name letter by letter and check if it overflows space given
        line[n] = event->name[i];
        if (line[n] == ' ')
            lastSpace = n;
        line[++n] = 0;

        int16_t xt1, yt1;
        uint16_t w, h;

        // Gets text bounds
        display.getTextBounds(line, 0, 0, &xt1, &yt1, &w, &h);

        // Char out of bounds, put in next line
        if (w > 420 / 3 - 30)
        {
            // if there was a space 5 chars before, break line there
            if (n - lastSpace < 5)
            {
                i -= n - lastSpace - 1;
                line[lastSpace] = 0;
            }

            // Print text line
            display.setCursor(x1 + 5, display.getCursorY());
            display.println(line);

            // Clears line (null termination on first charachter)
            line[0] = 0;
            n = 0;
        }
    }

    // display last line
    display.setCursor(x1 + 5, display.getCursorY());
    display.println(line);

    // Set cursor on same y but change x
    display.setCursor(x1 + 3, display.getCursorY());
    display.setFont(&FreeSans9pt7b);

    // Print time
    // also, if theres a location print it
    if (strlen(event->location) != 1)
    {
        display.println(event->time);

        display.setCursor(x1 + 5, display.getCursorY());

        char line[128] = {0};

        for (int i = 0; i < strlen(event->location); ++i)
        {
            line[i] = event->location[i];
            line[i + 1] = 0;

            int16_t xt1, yt1;
            uint16_t w, h;

            // Gets text bounds
            display.getTextBounds(line, 0, 0, &xt1, &yt1, &w, &h);

            if (w > (442 / 3))
            {
                for (int j = i - 1; j > max(-1, i - 4); --j)
                    line[j] = '.';
                line[i] = 0;
            }
        }

        display.print(line);
    }
    else
    {
        display.print(event->time);
    }

    // Calculating coordinates of text box
    int bx1 = x1 + 2;
    int by1 = y1;
    int bx2 = x1 + 440 / 3 - 7;
    int by2 = display.getCursorY() + 7;

    // Now we know the full height of the text box
    // From here, we print the background of according height and print the text again

    // Fill with color and cycle to next color
    display.fillRect(bx1, by1, 440 / 3 - 7, by2 - by1, currentColor);

    // If the color selected is yellow, print the text in black
    // Otherwise, print it in white
    if (currentColor == 5)
        display.setTextColor(0);
    else
        display.setTextColor(1);

    // Cycle to the next color
    currentColor++;
    if (currentColor == 7)
        currentColor = 2;

    // Upper left coordintes
    x1 = 3 + 4 + (440 / 3) * day;
    y1 = beginY + 3;

    // Setting text font
    display.setFont(&FreeSans12pt7b);

    // Some temporary variables
    n = 0;
    line[128];

    // Insert line brakes into setTextColor
    lastSpace = -100;
    display.setCursor(x1 + 5, beginY + 26);
    for (int i = 0; i < min((size_t)64, strlen(event->name)); ++i)
    {
        // Copy name letter by letter and check if it overflows space given
        line[n] = event->name[i];
        if (line[n] == ' ')
            lastSpace = n;
        line[++n] = 0;

        int16_t xt1, yt1;
        uint16_t w, h;

        // Gets text bounds
        display.getTextBounds(line, 0, 0, &xt1, &yt1, &w, &h);

        // Char out of bounds, put in next line
        if (w > 420 / 3 - 30)
        {
            // if there was a space 5 chars before, break line there
            if (n - lastSpace < 5)
            {
                i -= n - lastSpace - 1;
                line[lastSpace] = 0;
            }

            // Print text line
            display.setCursor(x1 + 5, display.getCursorY());
            display.println(line);

            // Clears line (null termination on first charachter)
            line[0] = 0;
            n = 0;
        }
    }

    // display last line
    display.setCursor(x1 + 5, display.getCursorY());
    display.println(line);

    // Set cursor on same y but change x
    display.setCursor(x1 + 3, display.getCursorY());
    display.setFont(&FreeSans9pt7b);

    // Print time
    // also, if theres a location print it
    if (strlen(event->location) != 1)
    {
        display.println(event->time);

        display.setCursor(x1 + 5, display.getCursorY());

        char line[128] = {0};

        for (int i = 0; i < strlen(event->location); ++i)
        {
            line[i] = event->location[i];
            line[i + 1] = 0;

            int16_t xt1, yt1;
            uint16_t w, h;

            // Gets text bounds
            display.getTextBounds(line, 0, 0, &xt1, &yt1, &w, &h);

            if (w > (442 / 3))
            {
                for (int j = i - 1; j > max(-1, i - 4); --j)
                    line[j] = '.';
                line[i] = 0;
            }
        }

        display.print(line);
    }
    else
    {
        display.print(event->time);
    }


    // Draw event rect bounds
    display.drawThickLine(bx1, by1, bx1, by2, 0, 2.0);
    display.drawThickLine(bx1, by2, bx2, by2, 0, 2.0);
    display.drawThickLine(bx2, by2, bx2, by1, 0, 2.0);
    display.drawThickLine(bx2, by1, bx1, by1, 0, 2.0);

    // Set how high is the event
    *heigthNeeded = display.getCursorY() + 12 - y1;

    // Return is it overflowing
    return display.getCursorY() < maxHeigth - 5;
}

// Struct event comparison function, by timestamp, used for qsort later on
int cmp(const void *a, const void *b)
{
    entry *entryA = (entry *)a;
    entry *entryB = (entry *)b;

    return (entryA->timeStamp - entryB->timeStamp);
}

// Main data drawing data
void drawData()
{
    long i = 0;
    long n = strlen(data);

    // reset count
    entriesNum = 0;

    // Search raw data for events
    while (i < n && strstr(data + i, "BEGIN:VEVENT"))
    {
        // Find next event start and end
        i = strstr(data + i, "BEGIN:VEVENT") - data + 12;
        char *end = strstr(data + i, "END:VEVENT");

        // Find all relevant event data
        char *summary = strstr(data + i, "SUMMARY:") + 8;
        char *location = strstr(data + i, "LOCATION:") + 9;
        char *timeStart = strstr(data + i, "DTSTART:") + 8;
        char *timeEnd = strstr(data + i, "DTEND:") + 6;

        if (summary && summary < end && (summary - data) > 0)
        {
            strncpy(entries[entriesNum].name, summary, strchr(summary, '\n') - summary);
            entries[entriesNum].name[strchr(summary, '\n') - summary] = 0;
        }
        if (location && location < end && (location - data) > 0)
        {
            strncpy(entries[entriesNum].location, location, strchr(location, '\n') - location);
            entries[entriesNum].location[strchr(location, '\n') - location] = 0;
        }
        if (timeStart && timeStart < end && timeEnd < end)
        {
            getToFrom(entries[entriesNum].time, timeStart, timeEnd, &entries[entriesNum].day,
                      &entries[entriesNum].timeStamp);
        }
        ++entriesNum;
    }

    // Sort entries by time
    qsort(entries, entriesNum, sizeof(entry), cmp);

    // Events displayed and overflown counters
    int columns[3] = {0};
    bool clogged[3] = {0};
    int cloggedCount[3] = {0};

    // Displaying events one by one
    for (int i = 0; i < entriesNum; ++i)
    {
        // If column overflowed just add event to not shown
        if (entries[i].day != -1 && clogged[entries[i].day])
            ++cloggedCount[entries[i].day];
        if (entries[i].day == -1 || clogged[entries[i].day])
            continue;

        // We store how much height did one event take up
        int shift = 0;
        bool s = drawEvent(&entries[i], entries[i].day, columns[entries[i].day] + 64, 600 - 4, &shift);

        columns[entries[i].day] += shift;

        // If it overflowed, set column to clogged and add one event as not shown
        if (!s)
        {
            ++cloggedCount[entries[i].day];
            clogged[entries[i].day] = 1;
        }
    }

    // Display not shown events info
    for (int i = 0; i < 3; ++i)
    {
        if (clogged[i])
        {
            // Draw notification showing that there are more events than drawn ones
            display.fillRoundRect(6 + i * (442 / 3), 600 - 24, (442 / 3) - 5, 20, 10, 0);
            display.setCursor(15 + i * (442 / 3), 600 - 10);
            display.setTextColor(INKPLATE_WHITE);
            display.setFont(&FreeSans9pt7b);
            display.print(cloggedCount[i]);
            display.print(" more events");
        }
    }
}
