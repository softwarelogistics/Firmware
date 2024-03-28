#include <Arduino.h>
#include <NeoPixelBus.h>
#include <NuvIoT.h>

const uint8_t arrow[8] = {
    0b00010000,
    0b00111000,
    0b01111100,
    0b11010110,
    0b00010000,
    0b00010000,
    0b00000000,
    0b00000000,
};

const uint8_t right_slight[8] = {
    0b00001111,
    0b00010011,
    0b00000101,
    0b00001001,
    0b00010010,
    0b00100000,
    0b00100000,
    0b00000000};

const uint8_t left_slight[8] = {
    0b11110000,
    0b11001000,
    0b10100000,
    0b10010000,
    0b01001000,
    0b00000100,
    0b00000100,
    0b00000000};

const uint8_t left_hard[8] = {
    0b00011000,
    0b00001100,
    0b00000110,
    0b11111111,
    0b11001100,
    0b11011000,
    0b11000000,
    0b00000000};

const uint8_t right_hard[8] = {
    0b00010000,
    0b00110000,
    0b01100000,
    0b11111110,
    0b01100011,
    0b00110011,
    0b00000011,
    0b00000000};

const uint8_t shelter[8] = {
    0b00011000,
    0b00111100,
    0b01111110,
    0b01100110,
    0b01100110,
    0b01111110,
    0b00000000,
    0b00000000,
};

void resetShape(NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1X8Ws2812xMethod> *strip)
{
    for (int idx = 0; idx < 8 * 8; ++idx)
        strip->SetPixelColor(idx, RgbColor(0, 0, 0));
}

void drawShape(const uint8_t *buffer, NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1X8Ws2812xMethod> *strip, uint8_t r, uint8_t g, uint8_t b)
{
    for (int idx = 0; idx < 8; ++idx)
    {
        uint8_t row = buffer[idx];
        for (int col = 0; col < 8; ++col)
        {
            uint16_t pixel = (col * 8) + ((col % 2 == 0) ? idx : (7 - idx));
            uint8_t mask = 0b00000001 << col;

            if ((row & mask) == mask)
            {
                strip->SetPixelColor(pixel, RgbColor(r, g, b));
            }
        }
    }
}

void setCell(NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1X8Ws2812xMethod> *strip, uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t pixel = (col * 8) + ((col % 2 == 0) ? row : (7 - row));
    strip->SetPixelColor(pixel, RgbColor(r, g, b));
}

void setFlags(NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1X8Ws2812xMethod> *strip, bool cellOnline, bool gpsOnline, int battLevel)
{
    if (cellOnline)
    {
        setCell(strip, 7, 1, 0, 255, 0);
        setCell(strip, 7, 0, 0, 255, 0);
    }
    else
    {
        setCell(strip, 7, 1, 255, 0, 0);
        setCell(strip, 7, 0, 255, 0, 0);
    }

    if (gpsOnline)
    {
        setCell(strip, 7, 4, 0, 255, 0);
        setCell(strip, 7, 3, 0, 255, 0);
    }
    else
    {
        setCell(strip, 7, 4, 255, 0, 0);
        setCell(strip, 7, 3, 255, 0, 0);
    }
}