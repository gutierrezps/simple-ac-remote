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

        /**
         * @return  data array length in bytes
         */
        uint8_t Length()
        {
            return nBits/8 + ( nBits % 8 > 0 );
        }

        /**
         * Writes the data packet on Arduino EEPROM if it's valid.
         *
         * First byte is nBits
         * Second byte is protocol ID
         * Next n bytes are data
         * 
         * @param   address     starting address
         * @return  0 on failure
         */
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

        /**
         * Reads the data packet from EEPROM.
         * 
         * @param   address     starting position
         * @return  0 if incorrect size or unknown protocol
         */
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

        /**
         * @see     WriteToEEPROM
         * @return  number of bytes occupied on EEPROM
         */
        char SizeOnEEPROM()
        {
            return Length() + 2;
        }

        /**
         * Copies data from other packet
         */
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