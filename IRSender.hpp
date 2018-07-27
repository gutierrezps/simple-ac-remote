#ifndef IRSender_hpp
#define IRSender_hpp

#include <Arduino.h>
#include "IRProtocols.hpp"
#include "IRData.hpp"

void sendIR(IRsend &irSender, IRData &irData);
void sendIRBlock(IRsend &irSender, IRData &irData);

/**
 * Sends infrared data with given IRsend. Based on sendNEC
 * function from IRremote library.
 * 
 * @param   irSender    sender object from IRremote library
 * @param   irData      to be sent
 */
void sendIR(IRsend &irSender, IRData &irData)
{
    // Set IR carrier frequency (38 kHz)
    irSender.enableIROut(38);

    if (!irData.isValid || irData.protocol == NULL)
        return;

    sendIRBlock(irSender, irData);

    if (irData.isRepeated)
    {
        irSender.space(irData.protocol->RepeatSpace());
        sendIRBlock(irSender, irData);
    }
}

/**
 * Sends a single header and data block
 * 
 * @param irSender
 * @param irData
 */
void sendIRBlock(IRsend &irSender, IRData &irData)
{
    // Header
    irSender.mark(irData.protocol->HeaderMark());
    irSender.space(irData.protocol->HeaderSpace());

    uint8_t currentBit = 0;     // bit to be sent
    uint8_t currentByte = 0;    // byte where the currentBit is located
    uint8_t mask = 0;           // binary mask to extract currentBit from currentByte

    for (uint8_t bitIndex = 0; bitIndex < irData.nBits; bitIndex++)
    {
        // Same for both bit values
        irSender.mark(irData.protocol->BitMark());

        currentByte = irData.data[(int) bitIndex/8];

        // bits are sent from MSB to LSB, i.e. given the same byte,
        // the first bit is selected using the mask 0b10000000,
        // and the last bit is selected using the mask 0b00000001
        mask = 1 << (7 - (bitIndex % 8));

        currentBit = currentByte & mask;

        if (currentBit)
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

    if (irData.protocol->HasTrail())
    {
        irSender.space(irData.protocol->TrailSpace());
        irSender.mark(irData.protocol->BitMark());
    }

    // turn LED off
    irSender.space(0);
}

#endif