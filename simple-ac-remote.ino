//******************************************************************************
// Simple AC Remote
// Version 0.1 April 2017
// Copyright 2017 Gutierrez PS
// For details, see http://github.com/gutierrezps/simple-ac-remote
// 
// 
//******************************************************************************

#include <IRremote.h>
#include <IRremoteInt.h>

#include "IRData.hpp"
#include "IRDecoder.hpp"
#include "IRSender.hpp"

const struct
{
    char buttonLevel;
    char buttonOff;
    char led1;
    char led2;
    char led3;
    char ledBlink;
    char irSensor;
}
gPins = {12, 11, 4, 5, 6, 7, 2};

IRrecv gIRRecv(gPins.irSensor);
IRsend gIRSender;      // IR LED connected on pin 3

decode_results gIrData;

char gACLevel = 0;      // 0: off, 1-3: cooling level
char gSendCode = 0;

void program();
void load();

void setup()
{
    pinMode(gPins.irSensor, INPUT);
    pinMode(gPins.led1, OUTPUT);
    pinMode(gPins.led2, OUTPUT);
    pinMode(gPins.led3, OUTPUT);
    pinMode(gPins.ledBlink, OUTPUT);
    pinMode(gPins.buttonOff, INPUT_PULLUP);
    pinMode(gPins.buttonLevel, INPUT_PULLUP);

    Serial.begin(115200);

    //if(digitalRead(gPins.buttonLevel) == LOW)
    {
        Serial.println("Programming mode");
        program();
    }
}

void loop() {}



void program()
{
    char currentCode = 0, blinkStatus = 1, received = 0;
    unsigned long blinkTimer = 0;

    gIRRecv.enableIRIn();

    while(currentCode < 4)
    {
        if(gIRRecv.decode(&gIrData)) received = 1;

        switch(currentCode)
        {
            case 0: 
                digitalWrite(gPins.led1, blinkStatus ? LOW : HIGH);
                digitalWrite(gPins.led2, blinkStatus ? HIGH : LOW);
                digitalWrite(gPins.led3, blinkStatus ? LOW : HIGH);
                break;

            case 3:
                digitalWrite(gPins.led3, blinkStatus);
            case 2:
                digitalWrite(gPins.led2, blinkStatus);
            case 1:
                digitalWrite(gPins.led1, blinkStatus);
                break;
        }

        if(received)
        {
            Serial.print("\ncode ");
            Serial.print(currentCode, DEC);
            Serial.print(": ");

            IRData data;

            decodeIR(&gIrData, data, 1);

            if(data.isValid)
            {
                digitalWrite(gPins.ledBlink, HIGH);
                sendIR(gIRSender, data);
                digitalWrite(gPins.ledBlink, LOW);
                currentCode++;
            }
            else
            {
                Serial.println("UNKNOWN");
                dumpRaw(&gIrData, 0);
            }

            gIRRecv.enableIRIn();
            gIRRecv.resume(); // Receive the next value
            received = 0;
            blinkTimer = 0;
            blinkStatus = 1;

            digitalWrite(gPins.led1, LOW);
            digitalWrite(gPins.led2, LOW);
            digitalWrite(gPins.led3, LOW);
        }
        else
        {
            if(++blinkTimer > 500)
            {
                blinkStatus = blinkStatus ? 0 : 1;
                blinkTimer = 0;
            }
        }

        delay(1);
    }

    for(char i = 0; i <= 4; i++)
    {
        digitalWrite(gPins.led1, i % 2);
        digitalWrite(gPins.led2, i % 2);
        digitalWrite(gPins.led3, i % 2);
        delay(100);
    }

    //if(!save_error)
    {
        //EEPROM.write(0, 'p');
    }
}

void load()
{

}


/*
void load()
{
    unsigned int addr = 1;
    
    for(char code = 0; code < 4; code++)
    {
        if(g_decode_ac.readFromEEPROM(addr))
        {
            g_decode_ac.dump();
            g_decode_ac.copyValueTo(g_codes[code], 10);
            g_codes_bits[code] = g_decode_ac.bits;

            addr += g_decode_ac.valueLength() + 1;
        }
        else
        {
            Serial.println("load error");
            return;
        }
    }

    Serial.println("codes loaded");
}

void setup()
{
    pinMode(gPins.irSensor, INPUT);
    pinMode(gPins.led1, OUTPUT);
    pinMode(gPins.led2, OUTPUT);
    pinMode(gPins.led3, OUTPUT);
    pinMode(gPins.ledBlink, OUTPUT);
    pinMode(gPins.buttonOff, INPUT_PULLUP);
    pinMode(gPins.buttonLevel, INPUT_PULLUP);

    Serial.begin(115200);

    if(digitalRead(gPins.buttonLevel) == LOW)
    {
        Serial.println("Programming mode");
        program();
    }
}

void loop()
{
    if(digitalRead(gPins.buttonLevel) == LOW)
    {
        delay(100);
        while(digitalRead(gPins.buttonLevel) == LOW);

        if(++gACLevel == 4) gACLevel = 1;
        gSendCode = 1;
    }

    if(digitalRead(gPins.buttonOff) == LOW)
    {
        delay(100);
        while(digitalRead(gPins.buttonOff) == LOW);
        gACLevel = 0;
        gSendCode = 1;
    }

    if(gSendCode)
    {
        Serial.print("sending ");
        Serial.println(gACLevel, DEC);

        digitalWrite(gPins.led1, LOW);
        digitalWrite(gPins.led2, LOW);
        digitalWrite(gPins.led3, LOW);

        switch(gACLevel)
        {
            case 3:
                digitalWrite(gPins.led3, HIGH);
            case 2:
                digitalWrite(gPins.led2, HIGH);
            case 1:
                digitalWrite(gPins.led1, HIGH);
                break;
        }

        digitalWrite(gPins.ledBlink, HIGH);
        gIRSender.sendRaw(g_raw[gACLevel], g_raw_size[gACLevel], 38);
        delay(100);
        digitalWrite(gPins.ledBlink, LOW);
        delay(400);
        gSendCode = 0;
    }
}
*/