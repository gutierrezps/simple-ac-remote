#ifndef IRDecoder_hpp
#define IRDecoder_hpp

#include <Arduino.h>
#include "IRProtocols.hpp"
#include "IRData.hpp"

class IRDecoder
{
public:

    enum Error : char
    {
        None = 0,
        HeaderMismatch = 1,
        DataOverflow,
        MarkMismatch,
        SpaceMismatch,
        TrailMismatch,
        NoRepeat
    };

    void errorToString(Error error)
    {
        switch(error)
        {
            case None:              return "none";
            case HeaderMismatch:    return "header mismatch";
            case DataOverflow:      return "data overflow";
            case MarkMismatch:      return "mark mismatch";
            case SpaceMismatch:     return "space mismatch";
            case TrailMismatch:     return "trail mismatch";
            case NoRepeat:          return "no repeat";
            case Repeat:            return "no repeat";
        }
    }
};


/**
 * Tries to decode the raw data by ckecking its timings against
 * a certain protocol.
 *
 * @see     IRProtocol class
 * 
 * @param   results     obtained from IRremote library
 * @param   irData      destination data packet
 * @param   protocol
 * @param   offset      initial offset on results' raw data
 * @param   debug       if 1, prints debug info
 * 
 * @return  true if raw data match given protocol
 */
bool tryDecodeIR(
    decode_results *results, IRData &irData, IRProtocol *protocol,
    uint8_t offset, char debug
    )
{
    uint8_t nBits = 0;      // # of bits received (mark-space pairs)
    uint8_t rawLength = results->rawlen;
    unsigned int rawValue = 0;
    unsigned char iData = 0;

    if(offset == 0) offset = 1;

    if(rawLength <= 4) return false;    // not sure if this could happen
    
    // checks initial mark and space - please notice offset++
    if( !MATCH_MARK(results->rawbuf[offset++], protocol->HeaderMark())
        || !MATCH_SPACE(results->rawbuf[offset++], protocol->HeaderSpace())
        )
    {
        if(debug) Serial.println("header mismatch");
        return false;
    }

    // ignores start space, header mark and space, and last mark
    nBits = (results->rawlen - 4)/2;
    if(nBits > irData.MaxSize() * 8)
    {
        Serial.println("tryDecodeIR overflow");
        return false;
    }

    // tries to decode each bit
    for(uint8_t iBit = 0; iBit < nBits; iBit++)
    {
        iData = iBit / 8;
        rawValue = results->rawbuf[offset];

        // initialize data array
        if(iBit % 8 == 0) irData.data[iData] = 0;

        if(!MATCH_MARK(rawValue, protocol->BitMark()))
        {
            if(debug)
            {
                Serial.print("mark mismatch - ");
                Serial.println((unsigned long) rawValue*USECPERTICK, DEC);
            }
            return false;
        }

        offset++;
        rawValue = rawValue;

        if(MATCH_SPACE(rawValue, protocol->BitOneSpace()))
        {
            irData.data[iData] = (irData.data[iData] << 1) | 1;
        }
        else if(MATCH_SPACE(rawValue, protocol->BitZeroSpace()))
        {
            irData.data[iData] = (irData.data[iData] << 1);
        }
        else if(protocol->IsRepeated() && MATCH_SPACE(rawValue, protocol->RepeatSpace()))
        {
            // TODO
        }
        else if(protocol->HasTrail() && (offset == rawLength - 2 || offset == rawLength - 1))
        {
            if( ( offset == rawLength - 2 && !MATCH_SPACE(rawValue, protocol->TrailSpace()) )
                || (offset == rawLength - 1 && !MATCH_MARK(rawValue, protocol->BitMark()))
                )
            {
                if(debug)
                {
                    Serial.print("trail mismatch - [");
                    Serial.print(offset);
                    Serial.print("]");
                    Serial.println((unsigned long) rawValue*USECPERTICK, DEC);
                }
                return false;
            }
        }
        else
        {
            if(debug)
            {
                Serial.print("space mismatch - [");
                Serial.print(offset);
                Serial.print("]");
                Serial.println((unsigned long) rawValue*USECPERTICK, DEC);
            }
            return false;
        }

        offset++;
    }

    // aligns left last bits on last data byte
    if(nBits % 8 > 0)
    {
        irData.data[(int)nBits/8] <<= 8 - (nBits % 8);
    }

    irData.nBits = nBits;
    irData.protocol = protocol;
    irData.isValid = true;

    return true;
}


/**
 * Tries to decode the raw data by ckecking its timings against all
 * available protocols.
 *
 * @see     IRProtocols class
 * 
 * @param   results     raw data
 * @param   data        destination data packet
 * @param   debug       if 1, prints debug info
 * 
 * @return  true if a matching protocol was found
 */
bool decodeIR(decode_results *results, IRData &data, char debug)
{
    IRProtocol *protocol = nullptr;

    data.isValid = false;

    // iterates over all protocols
    g_irProtocols.First();

    while(!g_irProtocols.IsDone())
    {
        protocol = g_irProtocols.Current();

        if(debug)
        {
            Serial.print("Trying ");
            Serial.print(protocol->Name());
            Serial.print(": ");
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

/**
 * Prints raw data on Serial. Based on IRrevcDump example from
 * IRremote library.
 * 
 * @param   results     raw data
 * @param   skip_lines  if 1, prints each timing on a new line
 */
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