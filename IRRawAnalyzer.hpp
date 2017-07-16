#ifndef IRRawAnalyzer_hpp
#define IRRawAnalyzer_hpp

#include <Arduino.h>
#include "IRData.hpp"
#include "IRProtocols.hpp"

#define MAX_DIFFERENT_VALUES 30

char arrayInsert(unsigned int *target, unsigned int value, unsigned char lastFree,
    unsigned char *related1, unsigned char *related2
    )
{
    unsigned int temp = 0;

    target[lastFree] = value;

    if(lastFree == 0) return 0;

    for(char i = lastFree - 1; i >= 0; i--)
    {
        if(target[i] <= target[i + 1]) return i + 1;

        temp = target[i];
        target[i] = target[i+1];
        target[i+1] = temp;

        temp = related1[i];
        related1[i] = related1[i+1];
        related1[i+1] = temp;

        temp = related2[i];
        related2[i] = related2[i+1];
        related2[i+1] = temp;
    }

    return 0;
}


void analyze(decode_results *results)
{
    uint8_t offset = 1;     // Skip initial space
    uint8_t nBits = 0;      // # of bits received (mark-space pairs)
    uint8_t rawLength = results->rawlen;
    unsigned char i;

    unsigned int values[MAX_DIFFERENT_VALUES] = {0};    // time width
    unsigned char count[MAX_DIFFERENT_VALUES] = {0};    // number of occurrences
    unsigned char polarity[MAX_DIFFERENT_VALUES] = {0}; // if it's a mark or space
    unsigned int value = 0;
    unsigned char occupied = 1;
    unsigned char pol = 1;      // 1 for mark, 0 for space

    values[0] = results->rawbuf[offset];
    count[0] = 1;
    polarity[0] = pol;  // first is a mark

    for(offset = 2; offset < rawLength; offset++)
    {
        value = results->rawbuf[offset];
        pol = pol ? 0 : 1;      // flip polarity

        for(i = 0; i < occupied; i++)
        {
            if(value == values[i] && pol == polarity[i])
            {
                count[i]++;
                break;
            }
        }

        if(i == occupied)
        {

            //values[i] = value;
            i = arrayInsert(values, value, i, count, polarity);
            count[i] = 1;
            polarity[i] = pol;
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
        Serial.print( polarity[i] ? '+' : '-' );
        Serial.print((unsigned long) values[i]*USECPERTICK, DEC);
        Serial.print(" ");
        Serial.println(count[i], DEC);
    }
}


#endif
