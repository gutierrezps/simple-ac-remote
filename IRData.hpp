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

        IRData()
        {
            protocol = NULL;
            isValid = false;
        }

        uint8_t MaxSize() { return IRDATA_MAX_VALUE_SIZE; }

        uint8_t Length()
        {
            return nBits/8 + ( nBits % 8 > 0 );
        }

        char WriteToEEPROM(uint16_t address)
        {
            if(Length() == 0 || !isValid) return 0;

            EEPROM.write(address, nBits);
            EEPROM.write(address + 1, protocol->GetId());

            for(uint8_t i = 0; i < Length(); i++)
            {
                EEPROM.write(address + 2 + i, data[i]);
            }

            return 1;
        }

        char ReadFromEEPROM(uint16_t address)
        {
            isValid = false;

            nBits = EEPROM.read(address);
            uint8_t size = Length();

            if(size == 0 || size > MaxSize()) return 0;

            protocol = g_irProtocols.GetProtocol(EEPROM.read(address + 1));

            if(protocol == NULL) return 0;

            for(uint8_t i = 0; i < size; i++)
            {
                data[i] = EEPROM.read(address + 2 + i);
            }

            isValid = true;

            return 1;
        }

        char SizeOnEEPROM()
        {
            // one for nBits, another for protocol id
            return Length() + 2;
        }

        void operator = (const IRData &copyFrom)
        {
            protocol = copyFrom.protocol;
            for(char i = 0; i < IRDATA_MAX_VALUE_SIZE; ++i)
            {
                data[i] = copyFrom.data[i];
            }
            nBits = copyFrom.nBits;
            isValid = copyFrom.isValid;
        }
};

#endif