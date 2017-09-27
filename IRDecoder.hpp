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
        NotEnoughData,
        HeaderMismatch,
        DataOverflow,
        MarkMismatch,
        SpaceMismatch,
        TrailMismatch
    };

    static uint16_t lastOffset;

    static String errorToString(Error error)
    {
        switch(error)
        {
            case None:              return "none";
            case NotEnoughData:     return "not enough data";
            case HeaderMismatch:    return "header mismatch";
            case DataOverflow:      return "data overflow";
            case MarkMismatch:      return "mark mismatch";
            case SpaceMismatch:     return "space mismatch";
            case TrailMismatch:     return "trail mismatch";
        }
    }

    static Error tryDecodeIR(decode_results *results, IRData &irData,
                        IRProtocol *protocol);
};

uint16_t IRDecoder::lastOffset = 1; // defines the static member variable


/**
 * Tries to decode the raw data by ckecking its timings against
 * a certain protocol.
 *
 * @see     IRProtocol class
 *
 * @param   results     obtained from IRremote library
 * @param   irData      destination data packet
 * @param   protocol
 *
 * @return  true if raw data match given protocol
 */
IRDecoder::Error IRDecoder::tryDecodeIR(
    decode_results *results, IRData &irData, IRProtocol *protocol)
{
    uint8_t nBits = 0;      // # of bits received (mark-space pairs)
    uint16_t rawLength = results->rawlen;
    unsigned int rawValue = 0;
    uint8_t iData = 0;
    uint8_t iBit = 0;
    char repeatReached = 0;         // used on protocols that repeat its packet

    lastOffset = 1;

    // not sure if this could happen
    if(rawLength <= 4) return NotEnoughData;

    // checks initial mark and space - please notice lastOffset++
    if( !MATCH_MARK(results->rawbuf[lastOffset++], protocol->HeaderMark())
        || !MATCH_SPACE(results->rawbuf[lastOffset++], protocol->HeaderSpace())
        )
    {
        return HeaderMismatch;
    }

    // ignores start space, header mark and space, and last mark
    nBits = (results->rawlen - 4)/2;
    if(nBits > irData.MaxSize() * 8)
    {
        return DataOverflow;
    }

    // tries to decode each bit
    for(iBit = 0; iBit < nBits; iBit++)
    {
        iData = iBit / 8;
        rawValue = results->rawbuf[lastOffset];

        // initialize data array
        if(iBit % 8 == 0) irData.data[iData] = 0;

        if(!MATCH_MARK(rawValue, protocol->BitMark()))
        {
            return MarkMismatch;
        }

        lastOffset++;
        rawValue = results->rawbuf[lastOffset];

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
            repeatReached = 1;
            irData.isRepeated = true;
            break;
        }
        else if(protocol->HasTrail() && (lastOffset == rawLength - 2 || lastOffset == rawLength - 1))
        {
            if( ( lastOffset == rawLength - 2 && !MATCH_SPACE(rawValue, protocol->TrailSpace()) )
                || (lastOffset == rawLength - 1 && !MATCH_MARK(rawValue, protocol->BitMark()))
                )
            {
                return TrailMismatch;
            }
        }
        else
        {
            return SpaceMismatch;
        }

        lastOffset++;
    }

    // If there is a repeat, nBits calculated previously is wrong.
    // Here we update it to the real value, i.e. the number of
    // bits processed so far, before reaching repeat space.
    // Therefore, data is made of those first bits decoded.
    if(repeatReached) nBits = iBit;
    else irData.isRepeated = false;

    // Align left last bits on last data byte
    if(nBits % 8 > 0)
    {
        irData.data[(int)nBits/8] <<= 8 - (nBits % 8);
    }

    irData.nBits = nBits;
    irData.protocol = protocol;
    irData.isValid = true;

    return None;
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
    IRDecoder::Error error = IRDecoder::None;

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

        error = IRDecoder::tryDecodeIR(results, data, protocol);

        if(error == IRDecoder::None)
        {
            if(debug) Serial.println("MATCH");
            break;
        }
        else
        {
            if(debug)
            {
                Serial.print(IRDecoder::errorToString(error));
                Serial.print(" - [");
                Serial.print(IRDecoder::lastOffset);
                Serial.print("] ");
                Serial.println((unsigned long) results->rawbuf[IRDecoder::lastOffset]*USECPERTICK, DEC);
            }
        }

        g_irProtocols.Next();
    }

    return data.isValid;
}

#endif
