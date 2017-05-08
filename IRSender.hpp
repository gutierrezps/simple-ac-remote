#ifndef IRSender_hpp
#define IRSender_hpp

#include <Arduino.h>
#include "IRProtocols.hpp"
#include "IRData.hpp"

void sendIR(IRsend &irSender, IRData &irData)
{
    // Set IR carrier frequency
    irSender.enableIROut(38);

    if(!irData.isValid || irData.protocol == nullptr) return;

    // Header
    irSender.mark(irData.protocol->HeaderMark());
    irSender.space(irData.protocol->HeaderSpace());

    for(uint8_t iBit = 0; iBit < irData.nBits; iBit++)
    {
        // MSb in data was received first, see decode_NEC_AC
        //         
        if( irData.data[(int)iBit/8] & ( 1 << (7 - (iBit % 8)) ) )
        {
            irSender.mark(irData.protocol->BitMark());
            irSender.space(irData.protocol->BitOneSpace());
        }
        else
        {
            irSender.mark(irData.protocol->BitMark());
            irSender.space(irData.protocol->BitZeroSpace());
        }
    }

    // Last mark
    irSender.mark(irData.protocol->BitMark());
    irSender.space(0);  // Always end with the LED off
}

#endif