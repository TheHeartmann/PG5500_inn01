#include <LEDMatrixDriver.hpp>
#include <SimpleTimer.h>
#include "font.h"

/*
 * This sketch is based on the MarqueeText example from the LEDMatrixDriver library
 * but has been tweaked to a minor degree and extended with some additional functonality
 * to scroll the text based on analog input.
 *
 * Created by Thomas Hartmann
 * 2017.10.01
 */

// Define the ChipSelect pin for the led matrix (Dont use the SS or MISO pin of your Arduino!)
// Other pins are arduino specific SPI pins (MOSI=DIN of the LEDMatrix and CLK) see https://www.arduino.cc/en/Reference/SPI
const uint8_t LEDMATRIX_CS_PIN = 8;

// Define LED Matrix dimensions (0-n) - eg: 32x8 = 31x7
const int LEDMATRIX_WIDTH = 31;
const int LEDMATRIX_HEIGHT = 7;
const int LEDMATRIX_SEGMENTS = 1;

// The LEDMatrixDriver class instance
LEDMatrixDriver lmd(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);

// Joystick config
const int Y_PIN = A0;
// This appears to be the neutral position when on a flat surface.
const int NO_MOTION = 505;
const int INPUT_MIN = 0;
const int INPUT_MAX = 1023;
// To avoid jittery text when moving the joystick around
const int JITTER_OFFSET = 4;
int yRead;
int yReadPrev;

// timer
SimpleTimer timer;

int x = 0, y = 0; // start top left

// Marquee speed
int animDelay = 50;
int nextDelay;
const int MIN_DELAY = 10;
const int MAX_DELAY = 60;

// Marquee text
char text[] = "PG5500 // INN01: MATRIX";
int len = strlen(text);

// Direction
bool backwards = false;

void setup()
{
    // init the display
    lmd.setEnabled(true);
    lmd.setIntensity(2); // 0 = low, 10 = high

    updateDisplay();
}

void loop()
{
    timer.run();

    // read values from input
    yRead = analogRead(Y_PIN);
    if (yRead != yReadPrev)
    {
        // gives it enough of an offset to not be jittery
        backwards = yRead < NO_MOTION-JITTER_OFFSET;
        int min, max;
        if (backwards)
        {
            min = INPUT_MIN;
            max = NO_MOTION;
        }
        else
        {
            max = NO_MOTION;
            min = INPUT_MAX;
        }
        animDelay = mapToDelay(yRead, min, max);

        yReadPrev = yRead;
    }
}

void updateDisplay()
{
    // Draw the text to the current position
    drawString(text, len, x, 0);

    // Toggle display of the new framebuffer
    lmd.display();

    // Advance to next/previous coordinate
    x = backwards ? ++x : --x;
    if (x < len * -8)
    {
        x = LEDMATRIX_WIDTH;
    }
    else if (x > LEDMATRIX_WIDTH)
    {
        x = len * -8;
    }

    timer.setTimeout(animDelay, updateDisplay);
}

int mapToDelay(int value, int min, int max)
{
    return map(value, min, max, MIN_DELAY, MAX_DELAY);
}

/**
 * This function draws a string of the given length to the given position.
 */
void drawString(char *text, int len, int x, int y)
{
    for (int idx = 0; idx < len; idx++)
    {
        int c = text[idx] - 32;

        // stop if char is outside visible area
        if (x + idx * 8 > LEDMATRIX_WIDTH)
            return;

        // only draw if char is visible
        if (8 + x + idx * 8 > 0)
            drawSprite(font[c], x + idx * 8, y, 8, 8);
    }
}

/**
 * This draws a sprite to the given position using the width and height supplied (usually 8x8)
 */
void drawSprite(byte *sprite, int x, int y, int width, int height)
{
    // The mask is used to get the column bit from the sprite row
    byte mask = B10000000;

    for (int iy = 0; iy < height; iy++)
    {
        for (int ix = 0; ix < width; ix++)
        {
            lmd.setPixel(x + ix, y + iy, (bool)(sprite[iy] & mask));

            // shift the mask by one pixel to the right
            mask = mask >> 1;
        }

        // reset column mask
        mask = B10000000;
    }
}
