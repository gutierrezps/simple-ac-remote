#ifndef IRDecoder_hpp
#define IRDecoder_hpp

#include <Arduino.h>
#include "IRProtocols.hpp"
#include "IRData.hpp"


bool tryDecodeIR(decode_results *results, IRData &irData, IRProtocol *protocol, char debug)
{
    uint8_t offset = 1;     // Skip initial space
    uint8_t nBits = 0;     // # of bits received (mark-space pairs)
    uint8_t rawLength = results->rawlen;
    unsigned char iData = 0;

    if(rawLength <= 4) return false;    // not sure if this could happen
    
    // Initial mark and space - please notice offset++
    if( !MATCH_MARK(results->rawbuf[offset++], protocol->HeaderMark())
        || !MATCH_SPACE(results->rawbuf[offset++], protocol->HeaderSpace())
        )
    {
        if(debug) Serial.println("header mismatch");
        return false;
    }

    // Ignoring start space, header mark and space, and last mark
    nBits = (results->rawlen - 4)/2;
    if(nBits > irData.MaxSize() * 8)
    {
        Serial.println("tryDecode overflow");
        return false;
    }

    for(uint8_t iBit = 0; iBit < nBits; iBit++)
    {
        iData = iBit / 8;

        // initialize data array
        if(iBit % 8 == 0) irData.data[iData] = 0;

        if(!MATCH_MARK(results->rawbuf[offset], protocol->BitMark()))
        {
            if(debug)
            {
                Serial.print("mark mismatch - ");
                Serial.println((unsigned long) results->rawbuf[offset]*USECPERTICK, DEC);
            }
            return false;
        }
        offset++;

        if(MATCH_SPACE(results->rawbuf[offset], protocol->BitOneSpace()))
        {
            irData.data[iData] = (irData.data[iData] << 1) | 1;
        }
        else if(MATCH_SPACE(results->rawbuf[offset], protocol->BitZeroSpace()))
        {
            irData.data[iData] = (irData.data[iData] << 1);
        }
        else
        {
            if(debug)
            {
                Serial.print("space mismatch - [");
                Serial.print(offset);
                Serial.print("]");
                Serial.println((unsigned long) results->rawbuf[offset]*USECPERTICK, DEC);
            }
            return false;
        }

        offset++;
    }

    // align left last bits on last byte
    if(nBits % 8 > 0)
    {
        irData.data[(int)nBits/8] <<= 8 - (nBits % 8);
    }

    irData.nBits = nBits;
    irData.protocol = protocol;
    irData.isValid = true;

    return true;
}


bool decodeIR(decode_results *results, IRData &data, char debug)
{
    data.isValid = false;

    g_irProtocols.First();
    IRProtocol *protocol = nullptr;

    while(!g_irProtocols.IsDone())
    {
        protocol = g_irProtocols.Current();

        if(debug)
        {
            Serial.print("\nTRYING ");
            Serial.println(protocol->Name());
        }

        tryDecodeIR(results, data, protocol, debug);

        if(data.isValid)
        {
            if(debug) Serial.println("MATCH");
            break;
        }

        g_irProtocols.Next();
    }

    return data.isValid;
}


void dumpRaw(decode_results *results, char skip_lines)
{
    for(int i = 1; i < results->rawlen; i++) {
        //Serial.print("[");
        //Serial.print(i);
        //Serial.print("]");
        if (i & 1) {
            Serial.print((unsigned long) results->rawbuf[i]*USECPERTICK, DEC);
        }
        else {
            Serial.write('-');
            Serial.print((unsigned long) results->rawbuf[i]*USECPERTICK, DEC);
        }
        Serial.print(" ");
        if(skip_lines) Serial.println("");
    }
    if(results->overflow) Serial.print("\noverflow");
    Serial.println("");

}


#endif