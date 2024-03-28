#ifndef EngineSensors_h
#define EngineSensors_h

#include <Arduino.h>
#include <Console.h>

class CANEngineSensors
{
private:
    Console *m_console;

public:
    CANEngineSensors(Console *console)
    {
        m_console = console;
    }

public:
    uint8_t EngineNumber;
    uint16_t OilPressure;
    uint16_t CoolantTemp;
    uint16_t BatteryVoltage;
    uint8_t TrimPosition;

    void handle(uint16_t pgn, uint8_t *data, uint8_t len)
    {
        switch ((data[0] & 0x03))
        {
        case 0x00:
            OilPressure = (data[3] << 8) | data[5];
            CoolantTemp = (data[5] << 6) | data[7];
            break;

        case 0x01:
            BatteryVoltage = (data[1] << 8) | data[2];
            break;

        default:
            TrimPosition = data[4];
            break;
        }
    }

    void print()
    {
        m_console->print("EngineNumber: ");
        m_console->println(String(EngineNumber));
        m_console->print("OilPressure: ");
        m_console->println(String(OilPressure));
        m_console->print("CoolantTemp: ");
        m_console->println(String(CoolantTemp));
        m_console->print("BatteryVoltage: ");
        m_console->println(String(BatteryVoltage));
        m_console->print("TrimPosition: ");
        m_console->println(String(TrimPosition));
    }
};

#endif