#ifndef IRRawAnalyzer_hpp
#define IRRawAnalyzer_hpp

#include <Arduino.h>
#include "IRData.hpp"
#include "IRProtocols.hpp"

#define MAX_DIFFERENT_VALUES 20

void analyze(decode_results *results)
{
    uint8_t offset = 1;     // Skip initial space
    uint8_t nBits = 0;      // # of bits received (mark-space pairs)
    uint8_t rawLength = results->rawlen;
    unsigned char i;

    unsigned int values[MAX_DIFFERENT_VALUES] = {0};
    unsigned char count[MAX_DIFFERENT_VALUES] = {0};
    unsigned int value = 0;
    unsigned char occupied = 1;

    values[0] = results->rawbuf[offset];
    count[0] = 1;

    for(offset = 2; offset < rawLength; offset++)
    {
        value = results->rawbuf[offset];

        for(i = 0; i < occupied; i++)
        {
            if(value == values[i])
            {
                count[i]++;
                break;
            }
        }

        if(i == occupied)
        {
            values[i] = value;
            count[i] = 1;
            occupied++;
            if(occupied == MAX_DIFFERENT_VALUES)
            {
                Serial.println("overflow");
                return;
            }
        }
    }

    for(i = 0; i < occupied; i++)
    {
        Serial.print((unsigned long) values[i]*USECPERTICK, DEC);
        Serial.print(" ");
        Serial.println(count[i], DEC);
    }
}


#endif