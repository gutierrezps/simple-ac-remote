#ifndef IRSender_hpp
#define IRSender_hpp

#include <Arduino.h>
#include "IRProtocols.hpp"
#include "IRData.hpp"


/**
 * Sends infrared data with given IRsend. Based on sendNEC
 * function from IRremote library.
 * 
 * @param   irSender    sender object from IRremote library
 * @param   irData      to be sent
 */
void sendIR(IRsend &irSender, IRData &irData)
{
    // Set IR carrier frequency
    irSender.enableIROut(38);

    if(!irData.isValid || irData.protocol == NULL) return;

    // Header
    irSender.mark(irData.protocol->HeaderMark());
    irSender.space(irData.protocol->HeaderSpace());

    for(uint8_t iBit = 0; iBit < irData.nBits; iBit++)
    {
        // Same for both bit values
        irSender.mark(irData.protocol->BitMark());

        // Sends MSb first
        if( irData.data[(int)iBit/8] & ( 1 << (7 - (iBit % 8)) ) )
        {
            irSender.space(irData.protocol->BitOneSpace());
        }
        else
        {
            irSender.space(irData.protocol->BitZeroSpace());
        }
    }

    // Last mark
    irSender.mark(irData.protocol->BitMark());
    if(irData.protocol->HasTrail())
    {
        irSender.space(irData.protocol->TrailSpace());
        irSender.mark(irData.protocol->TrailMark());
    }
    irSender.space(0);  // Always end with the LED off
}

#endif