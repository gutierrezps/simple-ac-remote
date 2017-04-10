#include <IRremote.h>
#include <IRremoteInt.h>

extern int MATCH(int measured, int desired);
extern int MATCH_MARK(int measured_ticks, int desired_us);
extern int MATCH_SPACE(int measured_ticks, int desired_us);

#define NEC_HDR_MARK    9000
#define NEC_HDR_SPACE   4500
#define NEC_BIT_MARK    560
#define NEC_ONE_SPACE   1690
#define NEC_ZERO_SPACE  560

const struct {
    char button_level;
    char button_off;
    char led_1;
    char led_2;
    char led_3;
    char ir_sensor;
} g_pins = {12, 11, 4, 5, 6, 2};

unsigned int g_raw[4][100] = {{0}, {0}, {0}, {0}};
char g_raw_size[4] = {0};

IRrecv g_ir_recv(g_pins.ir_sensor);
IRsend g_ir_sender;      // LED IR conectado ao pino 3

decode_results g_ir_data;

char g_status = 0;      // 0: desligado, 1-3: nível refrigeração
char g_send = 0;

void program()
{
    char current_code = 0, blink_status = 1, received = 0;
    unsigned long blink_timer = 0;

    g_ir_recv.enableIRIn();        // habilita receptor IR

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

        g_ir_sender.sendRaw(g_raw[g_status], g_raw_size[g_status], 38);
        delay(500);
        g_send = 0;
    }
}


/*


bool decodeNECHVAC(decode_results *results, long *data0, long *data1) {
    int offset = 1; // Skip initial space
    int n_bits = 0;

    // Initial mark
    if (!MATCH_MARK(results->rawbuf[offset], NEC_HDR_MARK)) {
        return false;
    }
    offset++;

    // Initial space
    if (!MATCH_SPACE(results->rawbuf[offset], NEC_HDR_SPACE)) {
        return false;
    }
    offset++;

    // ignore start space, header mark and space, and last mark
    n_bits = (results->rawlen - 4)/2;
    if(n_bits > 64) {
        Serial.println("Overflow");
        return false;
    }

    for (int i = 0; i < n_bits; i++) {
        if (!MATCH_MARK(results->rawbuf[offset], NEC_BIT_MARK)) {
            return false;
        }
        offset++;
        if (MATCH_SPACE(results->rawbuf[offset], NEC_ONE_SPACE)) {
            if(i < 32) {
                *data0 = (*data0 << 1) | 1;
            } else {
                *data1 = (*data1 << 1) | 1;
            }
        } else if (MATCH_SPACE(results->rawbuf[offset], NEC_ZERO_SPACE)) {
            if(i < 32) {
                *data0 <<= 1;
            } else {
                *data1 <<= 1;
            }
        } else {
            return false;
        }
        offset++;
    }

    return true;
}


void dump(decode_results *results)
{
    int count = results->rawlen;
    Serial.print("Raw (");
    Serial.print(count, DEC);
    Serial.print("): ");
    unsigned long space[2], mark[2];


    for (int i = 1; i < count; i++) {
        if (i & 1) {
            Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
        }
        else {
            Serial.write('-');
            Serial.print((unsigned long) results->rawbuf[i]*USECPERTICK, DEC);
        }
        Serial.print(" ");
    }
    Serial.println();
}


void loop()
{
    if (g_ir_recv.decode(&results)) {
        long data0 = 0, data1 = 0;
        unsigned int raw_signal[100] = {0};
        Serial.print("Detected - ");

        
        if(decodeNECHVAC(&results, &data0, &data1)) {
            Serial.print("NEC ");
            Serial.print(data0, HEX);
            Serial.print("-");
            Serial.println(data1, HEX);
        } else {
            dump(&results);
        }
        
        
        //Serial.println(results.value, HEX);
        dump(&results);

        for(unsigned char i = 1; i < results.rawlen; i++)
        {
            raw_signal[i-1] = results.rawbuf[i]*USECPERTICK;

            Serial.print(raw_signal[i-1], DEC);
            Serial.print(" ");
        }
        Serial.println(" ");

        g_ir_recv.resume(); // Receive the next value

        g_ir_sender.sendRaw(raw_signal, results.rawlen - 1, 38);

        g_ir_recv.enableIRIn(); // Start the receiver

        Serial.println("sent");
    }

    
}

*/