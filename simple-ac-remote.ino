//******************************************************************************
// Simple AC Remote
//
// Copyright 2017 Gutierrez PS
//
// http://github.com/gutierrezps/simple-ac-remote
//
//******************************************************************************

#include <IRremote.h>
#include <IRremoteInt.h>

#include <avr/eeprom.h>

#include "IRData.hpp"
#include "IRDecoder.hpp"
#include "IRSender.hpp"
#include "IRRawAnalyzer.hpp"

#define DUMPER_ENABLED 1

const struct
{
    char buttonLevel;
    char buttonOff;
    char buttonProjPower;
    char buttonProjMute;
    char led1;
    char led2;
    char led3;
    char ledBlink;
    char irSensor;
}
g_pins = {10, 11, 12, 9, 4, 5, 6, 7, 2};

IRrecv g_irRecv(g_pins.irSensor);
IRsend g_irSender;                  // IR LED connected on pin 3

uint8_t g_ACLevel = 0;              // 0: off, 1-3: cooling level
bool g_sendCode = false;
uint8_t g_remoteQty = 1;
uint8_t g_hasProjector = 0;
uint8_t g_projectorStatus = 0;      // 0: normal, 1: freeze, 2: mute

#define MAX_REMOTE_QTY  10

void program();
void dumper();
void sendCode(char code);
void sendProjector(char code);

void setup()
{
    pinMode(g_pins.irSensor, INPUT);
    pinMode(g_pins.led1, OUTPUT);
    pinMode(g_pins.led2, OUTPUT);
    pinMode(g_pins.led3, OUTPUT);
    pinMode(g_pins.ledBlink, OUTPUT);
    pinMode(g_pins.buttonOff, INPUT_PULLUP);
    pinMode(g_pins.buttonLevel, INPUT_PULLUP);
    pinMode(g_pins.buttonProjPower, INPUT_PULLUP);
    pinMode(g_pins.buttonProjMute, INPUT_PULLUP);

    Serial.begin(115200);

    #if DUMPER_ENABLED
    // Enter dumper mode if level button is held on startup
    if(digitalRead(g_pins.buttonLevel) == LOW && digitalRead(g_pins.buttonOff) == HIGH)
    {
        digitalWrite(g_pins.ledBlink, HIGH);
        delay(100);
        while(digitalRead(g_pins.buttonLevel) == LOW) delay(10);
        digitalWrite(g_pins.ledBlink, LOW);
        delay(100);
        dumper();
    }
    #endif

    // Erase programming if both buttons are held on startup
    // Or if the remote is not programmed
    if( ( digitalRead(g_pins.buttonOff) == LOW
            && digitalRead(g_pins.buttonLevel) == LOW
        ) || EEPROM.read(0) != 'p')
    {
        digitalWrite(g_pins.ledBlink, HIGH);
        delay(100);
        while(digitalRead(g_pins.buttonOff) == LOW
                || digitalRead(g_pins.buttonLevel) == LOW)
        {
            delay(10);
        }
        digitalWrite(g_pins.ledBlink, LOW);
        delay(100);

        program();
    }

    digitalWrite(g_pins.ledBlink, HIGH);

    // Second byte of EEPROM tells how many remotes are programmed
    g_remoteQty = EEPROM.read(1);

    Serial.print("remoteQty ");
    Serial.println(g_remoteQty);

    if(g_remoteQty == 0 || g_remoteQty > MAX_REMOTE_QTY)
    {
        Serial.println("error");
    }
    Serial.println("ready");
}

void loop()
{
    if(digitalRead(g_pins.buttonLevel) == LOW)
    {
        delay(100);
        while(digitalRead(g_pins.buttonLevel) == LOW) delay(1);

        if(++g_ACLevel == 4) g_ACLevel = 1;
        g_sendCode = 1;
    }

    if(digitalRead(g_pins.buttonOff) == LOW)
    {
        delay(100);
        while(digitalRead(g_pins.buttonOff) == LOW) delay(1);
        g_ACLevel = 0;
        g_sendCode = 1;
    }

    if(digitalRead(g_pins.buttonProjPower) == LOW)
    {
        delay(100);
        while(digitalRead(g_pins.buttonProjPower) == LOW) delay(1);
        Serial.println(F("sending proj power"));
        sendProjector(0);
        delay(400);
    }

    if(digitalRead(g_pins.buttonProjMute) == LOW)
    {
        delay(100);
        while(digitalRead(g_pins.buttonProjMute) == LOW) delay(1);
        Serial.println(F("sending proj mute/freeze"));
        sendProjector(1);
        delay(400);
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

        sendCode(g_ACLevel);

        delay(400);
        g_sendCode = 0;
    }
}

/**
 * Read line from Serial, terminated by '\n'
 * @param  ret  String to write to
 * @return      false if there is nothing on Serial
 */
bool readLine(String &ret)
{
    if(!Serial.available()) return false;

    ret = Serial.readStringUntil('\n');
    ret.replace("\r", "");
    ret.trim();
    Serial.println(ret);

    return true;
}

/**
 * Read a line from Serial and converts it to uint. If the conversion
 * was not successful, zero is returned if acceptZero is true,
 * otherwise prints an error message and waits a valid int
 * 
 * @param  acceptZero
 * @return  value read (zero also if not valid input)
 */
uint8_t readInt(bool acceptZero)
{
    uint8_t val = 0;
    String inputStr;

    while(1)
    {
        while(!readLine(inputStr)) delay(1);
        val = inputStr.toInt();
        if(acceptZero || val > 0) break;
        else Serial.println("invalid input");
    }

    return val;
}

/**
 * Converts an hex string to byte array.
 * 
 * @param  str          length must be double of capacity
 * @param  dest         data destination, byte array
 * @param  capacity     size of data (less than or equals to dest size)
 * @return              true if successful
 */
bool hexStringToArray(String str, uint8_t *dest, uint8_t capacity)
{
    uint8_t val = 0, i = 0;
    char ch = 0;

    if( (str.length() % 2 != 0) || (str.length() / 2 != capacity) )
    {
        Serial.println("capacity mismatch");
        return false;
    }

    for(i = 0; i < capacity; i++)
    {
        val = 0;
        ch = str.charAt(i*2);
        if(ch >= '0' && ch <= '9') val += ch - '0';
        else if(ch >= 'a' && ch <= 'f') val += ch - 'a' + 10;
        else if(ch >= 'A' && ch <= 'F') val += ch - 'A' + 10;
        else { Serial.println("invalid char") ; return false; }

        val *= 16;

        ch = str.charAt(i*2+1);
        if(ch >= '0' && ch <= '9') val += ch - '0';
        else if(ch >= 'a' && ch <= 'f') val += ch - 'a' + 10;
        else if(ch >= 'A' && ch <= 'F') val += ch - 'A' + 10;
        else { Serial.println("invalid char") ; return false; }

        dest[i] = val;
    }

    return true;
}

/**
 * Retrieves and removes the next word (arg) from a string (args),
 * separated by spaces. For example:
 * args = "set hour 12 24";
 * next = nextArg(args);    // next = "set", args = "hour 12 24"
 * 
 * @param  args     string separated by spaces
 * @return String   word retrieved, empty if none found
 */
String nextArg(String &args)
{
    String next("");
    char length = args.length();
    char init = -1, i;

    if(length == 0) return next;

    // find first occurrence of non-space character
    for(i = 0; i < length && args[i] == ' '; ++i);

    if(i == length)     // if it's all spaces, there are no args
    {
        next = "";
        args = "";
        return next;
    }

    init = i;

    // find first space after first argument
    for(++i; i < length && args[i] != ' '; ++i);

    next = args.substring(init, i);

    if(i != length)     // if there are characters left...
    {
        // find start of next argument
        for( ; i < length && args[i] == ' '; ++i);
    }

    // if it's all spaces, or if we've reached the end,
    // there are no arguments left
    args = (i == length) ? "" : args.substring(i);

    return next;
}

/**
 * Program remote control.
 *
 * First input is the number of remote controls, written on EEPROM[1].
 * 
 * Next is a flag for the presence of a projector remote control, written on EEPROM[2].
 *
 * For each remote, 4 codes are read:
 *     - 0 is to turn off
 *     - 1-3 are the cooling levels (1 being the highest temp)
 *
 * Each code is made of 4 parameters, separated by space:
 *     - number of bits (int)
 *     - hex string of length equals to double the byte length (rounded up number of bits)
 *     - protocol id (int)
 *     - repeat: 1 if code is sent twice, and if the protocol supports it
 *
 * Each code is packed on an IRData object and written to EEPROM.
 *
 * Storage on EEPROM is made of two parts: the IRData address on EEPROM (dataAddr),
 * and the IRData itself. pointerAddr points to the EEPROM addr where dataAddr
 * value is stored. dataAddr's of each code are stored after the g_remoteQty,
 * in sequence for each remote, following the rule:
 * 
 *     pointerAddr = 3 + code * 2 + remote * 8;
 *                   |          |            |
 *                   |          |            +-- 4 codes per remote * 2 bytes address
 *                   |          +-- 2 bytes address
 *                   +-- starting offset of dataAddr's
 *
 * Each IRData packet is written and then read back to ensure it was written
 * correctly on EEPROM.
 *
 * If successful, EEPROM[0] = 'p'
 */
void program()
{
    IRData data;
    signed char remote, code;
    bool error = false, success = false;
    uint8_t answer;
    uint16_t dataAddr = 0, pointerAddr = 3;
    String next, args;

    Serial.print(F("remote qty: "));
    do
    {
        g_remoteQty = readInt(false);
        error = g_remoteQty > 10;
        if(error) Serial.println("error");

    } while(error);

    Serial.print(F("projector? "));
    g_hasProjector = readInt(true);
    Serial.println(g_hasProjector ? " yes" : " no");

    EEPROM.write(0, 0);     // remotes programmed previously are now invalid

    EEPROM.write(1, g_remoteQty);
    EEPROM.write(2, g_hasProjector);

    // After the number of remotes, stored on EEPROM[1], and after
    // the projector flag on EEPROM[2] (hence the +3 at the end) follows
    // the 16-bit address to each code of each remote.
    // So, EEPROM[2-3] stores the addr of remote 1, code 0,
    // EEPROM[4-5] stores remote 1, code 1, and so on.
    // The +6 is for the projector remote addresses (power, freeze, mute)
    dataAddr += 8 * g_remoteQty +3 +6;

    char maxRemote = g_remoteQty;
    if(g_hasProjector) maxRemote++;

    for(remote = 0; remote < maxRemote; remote++)
    {
        char maxCode = 4;
        if(g_hasProjector && remote == g_remoteQty) maxCode = 3;

        for(code = 0; code < maxCode; code++)
        {
            Serial.println(F("\n========================================"));
            Serial.print(F("remote "));
            Serial.print(remote);
            Serial.print(F(", code "));
            Serial.print(code);
            Serial.print(F(" - pointerAddr "));
            Serial.print(pointerAddr);
            Serial.print(F(", dataAddr "));
            Serial.println(dataAddr);

            while(!Serial.available()) delay(1);

            String args = Serial.readStringUntil('\n');
            args.replace("\r", "");
            args.trim();
            Serial.print("\"");
            Serial.print(args);
            Serial.println("\"");

            next = nextArg(args);

            if(!next.length())
            {
                Serial.println(F("invalid sequence 1"));
                code -= 1;
                continue;
            }

            data.nBits = next.toInt();
            if(data.nBits == 0)
            {
                Serial.println(F("invalid n bits"));
                code -= 1;
                continue;
            }

            next = nextArg(args);
            if(!next.length())
            {
                Serial.println(F("invalid sequence 2"));
                code -= 1;
                continue;
            }

            success = hexStringToArray(next, data.data, data.Length());
            if(!success)
            {
                Serial.println(F("invalid data"));
                code -= 1;
                continue;
            }

            next = nextArg(args);
            if(!next.length())
            {
                Serial.println(F("invalid sequence 3"));
                code -= 1;
                continue;
            }

            data.protocol = g_irProtocols.GetProtocol(next.toInt());
            error = data.protocol == NULL;
            if(error)
            {
                Serial.println(F("invalid protocol"));
                code -= 1;
                continue;
            }

            next = nextArg(args);
            if(!next.length())
            {
                Serial.println(F("invalid sequence 4"));
                code -= 1;
                continue;
            }

            data.isRepeated = next.toInt() > 0;

            Serial.print(F("Result:  "));
            data.ToString();

            data.isValid = true;

            success = data.WriteToEEPROM(dataAddr);

            if(!success)
            {
                Serial.println(F("error while saving to eeprom"));
                return;
            }

            success = data.ReadFromEEPROM(dataAddr);

            if(!success)
            {
                Serial.println(F("error while saving to eeprom"));
                return;
            }
            data.ToString();

            Serial.println("saved");

            eeprom_write_block((const void*)&dataAddr, (void*)pointerAddr, sizeof(pointerAddr));
            pointerAddr += sizeof(pointerAddr);

            dataAddr += data.SizeOnEEPROM();
        }
    }

    EEPROM.write(0, 'p');
}

/**
 * Enters in IR reader mode. When a IR packet is received, its timings
 * are analyzed and printed, and then decode follows, trying to find a
 * matching IRProtocol. If successful, the decoded data is printed.
 *
 * If no matching protocol was found, a new IRProtocol must be created
 * by analyzing the timing statistics printed. See the lengths of marks
 * and spaces from header and data, and if there is a repeat and
 * what is the space between them.
 * 
 */
void dumper()
{
    char blinkStatus = 1, received = 0;
    unsigned long blinkTimer = 0;
    decode_results irRawData;

    Serial.println("dumper mode");

    g_irRecv.enableIRIn();

    while(1)
    {
        IRData data;

        if(g_irRecv.decode(&irRawData)) received = 1;

        digitalWrite(g_pins.ledBlink, blinkStatus);

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

        dumpRaw(&irRawData);
        analyze(&irRawData);
        decodeIR(&irRawData, data, 1);

        if(data.isValid)    // i.e. known protocol
        {
            data.ToString();

            digitalWrite(g_pins.led1, HIGH);
            delay(50);
            digitalWrite(g_pins.led1, LOW);
            delay(50);
            digitalWrite(g_pins.led1, HIGH);
            delay(50);
            digitalWrite(g_pins.led1, LOW);
        }
        else
        {
            digitalWrite(g_pins.led1, HIGH);
            delay(500);
            digitalWrite(g_pins.led1, LOW);
        }

        g_irRecv.resume();      // get another code
        received = 0;
        blinkTimer = 0;
        blinkStatus = 1;

        digitalWrite(g_pins.led1, LOW);
    }
}

void sendCode(char code)
{
    IRData irData;
    uint16_t pointerAddr, dataAddr;
    char remote = 0, success;

    for(remote = 0; remote < g_remoteQty; remote++)
    {
        pointerAddr = 3 + code * 2 + remote * 8;
        eeprom_read_block((void*)&dataAddr, (void*)pointerAddr, sizeof(pointerAddr));

        success = irData.ReadFromEEPROM(dataAddr);

        if(!success) return;

        digitalWrite(g_pins.ledBlink, LOW);
        sendIR(g_irSender, irData);
        delay(50);
        digitalWrite(g_pins.ledBlink, HIGH);
        delay(50);
    }
}

void sendProjector(char code)
{
    IRData irData;
    uint16_t pointerAddr, dataAddr;
    char success;

    pointerAddr = 3 + g_remoteQty * 8;      // power
    if(code != 0)
    {
        if(++g_projectorStatus == 3) g_projectorStatus = 0;

        if(g_projectorStatus == 1) pointerAddr += 2;    // freeze
        else pointerAddr += 4;                          // mute (and unmute)
    }
    
    eeprom_read_block((void*)&dataAddr, (void*)pointerAddr, sizeof(pointerAddr));
    success = irData.ReadFromEEPROM(dataAddr);

    if(!success) return;

    digitalWrite(g_pins.ledBlink, LOW);
    sendIR(g_irSender, irData);
    delay(50);
    digitalWrite(g_pins.ledBlink, HIGH);
    delay(50);
}
