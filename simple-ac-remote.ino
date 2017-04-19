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

const struct {
    char button_level;
    char button_off;
    char led_1;
    char led_2;
    char led_3;
    char led_blink;
    char ir_sensor;
} g_pins = {12, 11, 4, 5, 6, 7, 2};

unsigned int g_raw[4][100] = {{0}, {0}, {0}, {0}};      // stores raw timings
char g_raw_size[4] = {0};

IRrecv g_ir_recv(g_pins.ir_sensor);
IRsend g_ir_sender;      // IR LED connected on pin 3

decode_results g_ir_data;

char g_status = 0;      // 0: off, 1-3: cooling level
char g_send = 0;

void program()
{
    char current_code = 0, blink_status = 1, received = 0;
    unsigned long blink_timer = 0;

    g_ir_recv.enableIRIn();

    while(current_code < 4)
    {
        if(g_ir_recv.decode(&g_ir_data))
        {
            received = 1;
        }

        switch(current_code)
        {
            case 0: 
                digitalWrite(g_pins.led_1, blink_status ? LOW : HIGH);
                digitalWrite(g_pins.led_2, blink_status ? HIGH : LOW);
                digitalWrite(g_pins.led_3, blink_status ? LOW : HIGH);
                break;

            case 3:
                digitalWrite(g_pins.led_3, blink_status);
            case 2:
                digitalWrite(g_pins.led_2, blink_status);
            case 1:
                digitalWrite(g_pins.led_1, blink_status);
                break;
        }

        if(received)
        {
            Serial.print("code ");
            Serial.print(current_code, DEC);
            Serial.print(": ");
            for(unsigned char i = 1; i < g_ir_data.rawlen; i++)
            {
                g_raw[current_code][i-1] = (unsigned int) g_ir_data.rawbuf[i]*USECPERTICK;

                Serial.print(g_raw[current_code][i-1], DEC);
                Serial.print(" ");
            }
            Serial.println(" ");

            g_raw_size[current_code] = g_ir_data.rawlen - 1;

            g_ir_recv.resume(); // Receive the next value
            received = 0;
            current_code++;
            blink_timer = 0;
            blink_status = 1;

            digitalWrite(g_pins.led_1, LOW);
            digitalWrite(g_pins.led_2, LOW);
            digitalWrite(g_pins.led_3, LOW);
        }
        else
        {
            if(++blink_timer > 500)
            {
                blink_status = blink_status ? 0 : 1;
                blink_timer = 0;
            }
        }

        delay(1);
    }

    for(char i = 0; i <= 4; i++)
    {
        digitalWrite(g_pins.led_1, i % 2);
        digitalWrite(g_pins.led_2, i % 2);
        digitalWrite(g_pins.led_3, i % 2);
        delay(100);
    }
}

void setup()
{
    pinMode(g_pins.ir_sensor, INPUT);
    pinMode(g_pins.led_1, OUTPUT);
    pinMode(g_pins.led_2, OUTPUT);
    pinMode(g_pins.led_3, OUTPUT);
    pinMode(g_pins.led_blink, OUTPUT);
    pinMode(g_pins.button_off, INPUT_PULLUP);
    pinMode(g_pins.button_level, INPUT_PULLUP);

    Serial.begin(115200);

    if(digitalRead(g_pins.button_level) == LOW)
    {
        Serial.println("Programming mode");
        program();
    }
}

void loop()
{
    if(digitalRead(g_pins.button_level) == LOW)
    {
        delay(100);
        while(digitalRead(g_pins.button_level) == LOW);

        if(++g_status == 4) g_status = 1;
        g_send = 1;
    }

    if(digitalRead(g_pins.button_off) == LOW)
    {
        delay(100);
        while(digitalRead(g_pins.button_off) == LOW);
        g_status = 0;
        g_send = 1;
    }

    if(g_send)
    {
        Serial.print("sending ");
        Serial.println(g_status, DEC);

        digitalWrite(g_pins.led_1, LOW);
        digitalWrite(g_pins.led_2, LOW);
        digitalWrite(g_pins.led_3, LOW);

        switch(g_status)
        {
            case 3:
                digitalWrite(g_pins.led_3, HIGH);
            case 2:
                digitalWrite(g_pins.led_2, HIGH);
            case 1:
                digitalWrite(g_pins.led_1, HIGH);
                break;
        }

        digitalWrite(g_pins.led_blink, HIGH);
        g_ir_sender.sendRaw(g_raw[g_status], g_raw_size[g_status], 38);
        delay(100);
        digitalWrite(g_pins.led_blink, LOW);
        delay(400);
        g_send = 0;
    }
}
