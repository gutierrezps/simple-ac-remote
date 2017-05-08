#ifndef IRData_hpp
#define IRData_hpp

#include <Arduino.h>
#include <EEPROM.h>
#include "IRProtocols.hpp"

#define IRDATA_MAX_VALUE_SIZE     20

class IRData
{
    public:
        IRProtocol *protocol;
        uint8_t data[IRDATA_MAX_VALUE_SIZE];
        uint8_t nBits;
        bool isValid;

        IRData() {
            protocol = nullptr;
            isValid = false;
        }

        uint8_t MaxSize() { return IRDATA_MAX_VALUE_SIZE; }

        uint8_t Length()
        {
            return nBits/8 + ( nBits % 8 > 0 );
        }

        char WriteToEEPROM(uint16_t address)
        {
            if(Length() == 0) return 0;

            EEPROM.write(address, nBits);
            for(uint8_t i = 0; i < Length(); i++)
            {
                EEPROM.write(address + 1 + i, data[i]);
            }

            return 1;
        }

        char ReadFromEEPROM(uint16_t address)
        {
            nBits = EEPROM.read(address);
            uint8_t size = Length();

            if(size == 0 || size > MaxSize()) return 0;

            for(uint8_t i = 0; i < size; i++)
            {
                data[i] = EEPROM.read(address + 1 + i);
            }

            return 1;
        }
};

#endif