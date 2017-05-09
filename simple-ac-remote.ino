//******************************************************************************
// Simple AC Remote
// Version 0.2 May 2017
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
g_pins = {12, 11, 4, 5, 6, 7, 2};

IRrecv g_irRecv(g_pins.irSensor);
IRsend g_irSender;      // IR LED connected on pin 3

char g_ACLevel = 0;      // 0: off, 1-3: cooling level
char g_sendCode = 0;
char g_remoteQty = 1;

IRData * g_codes[2][4];

void program();
void load();

void setup()
{
    pinMode(g_pins.irSensor, INPUT);
    pinMode(g_pins.led1, OUTPUT);
    pinMode(g_pins.led2, OUTPUT);
    pinMode(g_pins.led3, OUTPUT);
    pinMode(g_pins.ledBlink, OUTPUT);
    pinMode(g_pins.buttonOff, INPUT_PULLUP);
    pinMode(g_pins.buttonLevel, INPUT_PULLUP);

    Serial.begin(115200);

    if(digitalRead(g_pins.buttonOff) == LOW)
    {
        EEPROM.write(0, 0);
        delay(100);
        while(digitalRead(g_pins.buttonOff) == LOW) delay(10);
        delay(100);
    }

    if(EEPROM.read(0) != 'p') program();

    load();
}

void loop()
{
    if(digitalRead(g_pins.buttonLevel) == LOW)
    {
        delay(100);
        while(digitalRead(g_pins.buttonLevel) == LOW);

        if(++g_ACLevel == 4) g_ACLevel = 1;
        g_sendCode = 1;
    }

    if(digitalRead(g_pins.buttonOff) == LOW)
    {
        delay(100);
        while(digitalRead(g_pins.buttonOff) == LOW);
        g_ACLevel = 0;
        g_sendCode = 1;
    }

    if(g_sendCode)
    {
        Serial.print("sending ");
        Serial.println(g_ACLevel, DEC);

        digitalWrite(g_pins.led1, LOW);
        digitalWrite(g_pins.led2, LOW);
        digitalWrite(g_pins.led3, LOW);

        switch(g_ACLevel)
        {
            case 3:
                digitalWrite(g_pins.led3, HIGH);
            case 2:
                digitalWrite(g_pins.led2, HIGH);
            case 1:
                digitalWrite(g_pins.led1, HIGH);
                break;
        }
        
        for(char remote = 0; remote < g_remoteQty; ++remote)
        {
            digitalWrite(g_pins.ledBlink, HIGH);
            sendIR(g_irSender, (*g_codes[remote][g_ACLevel]));
            delay(100);
            digitalWrite(g_pins.ledBlink, LOW);
            delay(50);
        }
        
        delay(400);
        g_sendCode = 0;
    }
}



void program()
{
    char currentCode = 0, blinkStatus = 1, received = 0, saveOk = 0;
    uint16_t eepromAddr = 1;
    unsigned long blinkTimer = 0;
    decode_results irRawData;

    Serial.println("\nProgramming mode");

    digitalWrite(g_pins.led1, HIGH);

    do
    {
        digitalWrite(g_pins.led2, (g_remoteQty == 2) ? HIGH : LOW);
        if(digitalRead(g_pins.buttonOff) == LOW)
        {
            delay(100);
            while(digitalRead(g_pins.buttonOff) == LOW) delay(10);
            delay(100);
            g_remoteQty = (g_remoteQty == 1) ? 2 : 1;
        }
    } while(digitalRead(g_pins.buttonLevel) == HIGH);

    EEPROM.write(eepromAddr++, g_remoteQty);

    g_irRecv.enableIRIn();

    for(char remote = 0; remote < g_remoteQty; ++remote)
    {
        currentCode = 0;
        IRData data;

        while(currentCode < 4)
        {
            if(g_irRecv.decode(&irRawData)) received = 1;

            switch(currentCode)
            {
                case 0: 
                    digitalWrite(g_pins.led1, blinkStatus ? LOW : HIGH);
                    digitalWrite(g_pins.led2, blinkStatus ? HIGH : LOW);
                    digitalWrite(g_pins.led3, blinkStatus ? LOW : HIGH);
                    break;

                case 3:
                    digitalWrite(g_pins.led3, blinkStatus);
                case 2:
                    digitalWrite(g_pins.led2, blinkStatus);
                case 1:
                    digitalWrite(g_pins.led1, blinkStatus);
                    break;
            }

            if(!received)
            {
                if(++blinkTimer > 500)
                {
                    blinkStatus = blinkStatus ? 0 : 1;
                    blinkTimer = 0;
                }

                delay(1);
                continue;   // return to loop beginning
            }

            // IR data received
            
            Serial.print("\ncode ");
            Serial.print(currentCode, DEC);
            Serial.print(": ");

            decodeIR(&irRawData, data, 1);

            if(data.isValid)
            {
                //digitalWrite(g_pins.ledBlink, HIGH);
                //sendIR(g_irSender, data);
                //digitalWrite(g_pins.ledBlink, LOW);
                digitalWrite(g_pins.ledBlink, HIGH);
                delay(50);
                digitalWrite(g_pins.ledBlink, LOW);
                delay(50);
                digitalWrite(g_pins.ledBlink, HIGH);
                delay(50);
                digitalWrite(g_pins.ledBlink, LOW);

                saveOk = data.WriteToEEPROM(eepromAddr);

                if(!saveOk) break;

                eepromAddr += data.SizeOnEEPROM();

                currentCode++;
            }
            else
            {
                Serial.println("UNKNOWN");
                dumpRaw(&irRawData, 0);

                digitalWrite(g_pins.ledBlink, HIGH);
                delay(500);
                digitalWrite(g_pins.ledBlink, LOW);
            }

            g_irRecv.enableIRIn();
            g_irRecv.resume(); // Receive the next value
            received = 0;
            blinkTimer = 0;
            blinkStatus = 1;

            digitalWrite(g_pins.led1, LOW);
            digitalWrite(g_pins.led2, LOW);
            digitalWrite(g_pins.led3, LOW);
        }

        if(!saveOk) break;
    }

    
    // blink level leds twice
    for(char i = 0; i <= 4; i++)
    {
        digitalWrite(g_pins.led1, i % 2);
        digitalWrite(g_pins.led2, i % 2);
        digitalWrite(g_pins.led3, i % 2);
        delay(100);
    }

    if(saveOk) EEPROM.write(0, 'p');
    else Serial.println("eeprom save error");
}

void load()
{
    IRData data;
    uint16_t eepromAddr = 2;
    g_remoteQty = EEPROM.read(1);

    for(char remote = 0; remote < g_remoteQty; ++remote)
    {
        for(char code = 0; code < 4; ++code)
        {
            data.ReadFromEEPROM(eepromAddr);

            if(!data.isValid)
            {
                Serial.println("eeprom load error");
                return;
            }

            g_codes[remote][code] = new IRData();
            (*g_codes[remote][code]) = data;

            eepromAddr += data.SizeOnEEPROM();
        }
    }

    Serial.println("load done");
}


