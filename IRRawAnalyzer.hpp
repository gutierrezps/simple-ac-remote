#ifndef IRRawAnalyzer_hpp
#define IRRawAnalyzer_hpp

#include <Arduino.h>
#include "IRData.hpp"
#include "IRProtocols.hpp"

#define MAX_DIFFERENT_VALUES 30

/**
 * Register a new timing found, updating two other related arrays
 * (in our case, 'count' and 'polarity').
 *
 * Insertion sort used.
 * 
 * @param  target       the timing array
 * @param  value        new timing to be inserted
 * @param  lastFree     last free position
 * @param  related1     array whose data is related to target
 * @param  related2     idem
 * 
 * @return              index where 'value' was inserted
 */
char addTiming(unsigned int *target, unsigned int value, unsigned char lastFree,
    unsigned char *related1, unsigned char *related2
    )
{
    unsigned int temp = 0;

    target[lastFree] = value;

    // returns if it's the first value being inserted
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

/**
 * Counts the occurrences of each timing, and prints
 * in ascending order.
 * 
 * @param results   raw data
 */
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
    unsigned char occupied = 1;     // number of different timings found
    unsigned char pol = 1;          // 1 for mark, 0 for space

    values[0] = results->rawbuf[offset];
    count[0] = 1;
    polarity[0] = pol;  // first timing is a mark

    for(offset = 2; offset < rawLength; offset++)
    {
        value = results->rawbuf[offset];

        pol = pol ? 0 : 1;      // flip polarity i.e. if the last was a mark,
                                // this is a space

        // see if the timing was found before
        for(i = 0; i < occupied; i++)
        {
            // if found before, add to count
            if(value == values[i] && pol == polarity[i])
            {
                count[i]++;
                break;
            }
        }

        // otherwise, insert a new timing
        if(i == occupied)
        {
            // values[i] = value
            i = addTiming(values, value, i, count, polarity);
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

    // prints the timing frequency
    for(i = 0; i < occupied; i++)
    {
        Serial.print( polarity[i] ? '+' : '-' );
        Serial.print((unsigned long) values[i]*USECPERTICK, DEC);
        Serial.print(" ");
        Serial.println(count[i], DEC);
    }
}


/**
 * Prints raw data on Serial. Based on IRrevcDumpV2
 * example from IRremote library.
 *
 * @param   results     raw data
 */
void dumpRaw(decode_results *results)
{
    // Print Raw data
    Serial.print("Timing[");
    Serial.print(results->rawlen-1, DEC);
    Serial.println("]: ");

    for (int i = 1;  i < results->rawlen;  i++)
    {
        unsigned long  x = results->rawbuf[i] * USECPERTICK;

        if (!(i & 1))   // Even
        {
            Serial.print("-");
            if (x < 1000)  Serial.print(" ") ;
            if (x < 100)   Serial.print(" ") ;
            Serial.print(x, DEC);
        }
        else            // Odd
        {
            Serial.print("     ");
            Serial.print("+");
            if (x < 1000)  Serial.print(" ") ;
            if (x < 100)   Serial.print(" ") ;
            Serial.print(x, DEC);
            if (i < results->rawlen-1) Serial.print(", "); //',' not needed for last one
        }

        if (!(i % 8))  Serial.println("");
    }

    Serial.println("");                    // Newline
    if(results->overflow) Serial.println("Overflow occurred");
}


#endif
