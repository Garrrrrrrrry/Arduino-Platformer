//functions/declaration for LCD
#define RS 7
#define EN 6
#define D0 8
#define D1 9
#define D2 10
#define D3 11
#define D4 5
#define D5 4
#define D6 3
#define D7 13
//int RW = 0

//for shift register
int dataPin = A0;
int clockPin = A5;
int latchPin = A4;
//for 4 digit 7 segment led display
int S3 = 12;
int S4 = A3;

void lcd_enable() {
    delay(2);
    digitalWrite(EN, HIGH);
    delay(2);
    digitalWrite(EN, LOW);
}
void lcd_write(char c) {
    digitalWrite(D0, (c & 0b00000001));
    digitalWrite(D1, (c & 0b00000010));
    digitalWrite(D2, (c & 0b00000100));
    digitalWrite(D3, (c & 0b00001000));
    digitalWrite(D4, (c & 0b00010000));
    digitalWrite(D5, (c & 0b00100000));
    digitalWrite(D6, (c & 0b01000000));
    digitalWrite(D7, (c & 0b10000000));
    lcd_enable();
}
void lcd_init() {
    digitalWrite(RS, LOW);
    lcd_write(0x30);
    lcd_write(0x38);
    lcd_write(0x08);
    lcd_write(0x01);
    lcd_write(0x06);
    //lcd_write(0x0E);
    lcd_write(0x0C);
    delay(2);
}
void lcd_cursor(int col, int row) {
    digitalWrite(RS, 0);
    if (row == 0) {
        lcd_write(0x80);
    }
    else {
        lcd_write(0xC0);
    }
    for (int i = 0; i < col; ++i) {
        lcd_write(0x14);
    }
    delay(0.5);
}
void lcd_writeString(String s) {
    digitalWrite(RS, HIGH);
    int len = s.length();
    char* arr = new char[s.length()];
    strcpy(arr, s.c_str());
    for (unsigned int i = 0; i < len; ++i) {
        char sub = *arr;
        lcd_write(sub);
        arr++;
    }
}
void lcd_custom(byte b[8], int location) {
    digitalWrite(RS, 0);
    lcd_write(0x40 + (location * 8));
    digitalWrite(RS, 1);
    for (int i = 0; i < 8; ++i) {
        lcd_write(b[i]);
    }
}

void lcd_clear() {
    digitalWrite(RS, 0);
    lcd_write(0x01);
}

byte empty[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte platform[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
};
byte platChar[8] = {
  B01110,
  B01110,
  B00100,
  B01110,
  B10101,
  B00100,
  B01010,
  B11111
};
byte jumpChar[8] = {
  B01110,
  B01110,
  B10101,
  B01110,
  B00100,
  B00100,
  B01010,
  B00000
};
byte wall[8]{
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001
};
byte wallChar[8] = {
  B01111,
  B01111,
  B10101,
  B01111,
  B00101,
  B00101,
  B01011,
  B00001
};
byte stuckChar[8]{
  B00001,
  B11101,
  B11101,
  B01001,
  B11101,
  B01001,
  B01001,
  B10101
};
byte key[8]{
  B00000,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110,
  B00000,
  B11111
};

void game_init() {
    //both/other
    lcd_custom(key, 0);
    lcd_custom(empty, 1);
    //map
    lcd_custom(platform, 2);
    lcd_custom(wall, 3);
    lcd_custom(stuckChar, 4);
    //action
    lcd_custom(wallChar, 5);
    lcd_custom(platChar, 6);
    lcd_custom(jumpChar, 7);
}

void lcd_createLevel(int matrix[][16], int levelY) {
    //top
    lcd_clear();
    lcd_cursor(0, 0);
    digitalWrite(RS, 1);
    for (int i = 0; i < 16; ++i) {
        lcd_write(matrix[levelY + 1][i]);
    }
    //bottom
    lcd_cursor(0, 1);
    digitalWrite(RS, 1);
    for (int i = 0; i < 16; ++i) {
        lcd_write(matrix[levelY][i]);
    }
}

//==================================================================
//IR setup
int IR_RECV = 2;
volatile int timing;
unsigned long output;
volatile bool reached = false;
volatile int i;
volatile int IR_state = 0;
void IR_decode() {
    if (IR_state != 0) {
        timing = TCNT1;
        TCNT1 = 0;
    }
    switch (IR_state) {
        //checking for end of 9 ms
    case 0:
        TCNT1 = 0;
        TCCR1B = 2;
        IR_state = 1;
        i = 0;
        return;
    case 1:
        if (timing <= 19000 && timing >= 17000) {
            IR_state = 2;
        }
        else {
            IR_state = 0;
            TCCR1B = 0;
        }
        return;
    case 2:
        if (timing <= 10000 && timing >= 8000) {
            IR_state = 3;
        }
        else {
            IR_state = 0;
            TCCR1B = 0;
        }
        return;
    case 3:
        if (timing <= 1400 && timing >= 800) {
            IR_state = 4;
        }
        else {
            IR_state = 0;
            TCCR1B = 0;
        }
        return;
    case 4:
        if (timing <= 3600 && timing >= 800) {
            if (timing > 2000) {
                bitSet(output, (31 - i));
            }
            else {
                bitClear(output, (31 - i));
            }
            ++i;
            if (i > 31) {
                reached = true;
                detachInterrupt(IR_RECV);
                return;
            }
            IR_state = 3;
        }
        else {
            IR_state = 0;
            TCCR1B = 0;
            return;
        }
    }
}
//==================================================================
int num_9[] = { 0, 1, 3, 4, 5, 6, -1 };
int num_8[] = { 0, 1, 2, 3, 4, 5, 6 };
int num_7[] = { 4, 5, 6, -1, -1, -1, -1 };
int num_6[] = { 0, 1, 2, 3, 4, 6, -1 };
int num_5[] = { 0, 1, 3, 4, 6, -1, -1 };
int num_4[] = { 0, 1, 4, 5, -1, -1, -1 };
int num_3[] = { 0, 3, 4, 5, 6, -1, -1 };
int num_2[] = { 1, 1, 0, 1, 1, 0, 1, 0 };
int num_1[] = { 4, 5, -1, -1, -1, -1, -1, -1 };
int num_0[] = { 1, 2, 3, 4, 5, 6, -1 };

byte num = 0;
void shiftRegister() {
    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, LSBFIRST, num);
    digitalWrite(latchPin, 1);
}
void writeDisplay(int arr[]) {
    unsigned char i;
    for (int i = 0; i < 7; ++i) {
        if (arr[i] != -1) {
            bitSet(num, arr[i]);
            shiftRegister();
        }
    }
}
//==================================================================
int JS_X = A1;
int JS_Y = A2;
int JS_BTN = 12;
int j_x = 0, j_y = 0;

typedef struct task {
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct)(int);
} task;

int delay_gcd;
const unsigned short tasksNum = 2;
const unsigned short tasksHalf = 1;
task tasks[tasksNum];

//String pressConfirm = "a25d";
//String pressOne = "30cf"; //(or 30ce)
//String pressTwo = "18e7";
//String pressBack = "22dd";

String text = "";
bool game = true;
bool score = false; int scoreNum = 0;
enum Menu_States { Menu_INIT, select, start, Score };
int Menu_Tick(int state1) {
    if (reached) {
        output = output & 0x00FFFF;
        text = String(output, HEX);
        reached = false;
        IR_state = 0;
        TCCR1B = 0;
        Serial.println(text);
    }
    switch (state1) {
    case Menu_INIT:
        digitalWrite(S3, 1);
        digitalWrite(S4, 1);
        lcd_clear();
        lcd_writeString("Start");
        if (score) {
            state1 = Score;
        }
        else {
            state1 = select;
        }
        return state1;
    case select:
        if (text.equals("30cf") || text.equals("30ce")) {
            Serial.println("pressed One");
            lcd_clear();
            lcd_writeString("Select Start?");
            text = "";
            state1 = start;
            break;
        }
        attachInterrupt(digitalPinToInterrupt(IR_RECV), IR_decode, CHANGE);
    case start:
        if (text.equals("a25d") || text.equals("a25c")) {
            game = true;
            state1 = Menu_INIT;
            text = "";
            lcd_clear();
            lcd_cursor(3, 0);
            lcd_writeString("Starting..");
            delay(1000);
            break;
        }
        else if (text.equals("22dd") || text.equals("22dc")) {
            state1 = Menu_INIT;
            text = "";
        }
        attachInterrupt(digitalPinToInterrupt(IR_RECV), IR_decode, CHANGE);
    case Score:
        lcd_clear();
        lcd_cursor(0, 0);
        String sub = String(scoreNum);
        String sum = "Score: " + sub;
        lcd_writeString(sum);

        return state1;
    }
}
//==================================================================
//currentCol, currentRow is for map cursor. curr_X, curr_Y is for character placement.
int curr_X, curr_Y, m_width, m_height, levelY, objX, objY, currentTime, led_count;
bool mark = false, stuck = false;
int level[8][16];
enum Game_States { Game_INIT, level_1, level_2, level_3, level_4, level_5 }; //l2, l3, l4, l5;
int Game_Tick(int state2) {
    switch (state2) {
    case Game_INIT:
        game_init();
        m_width = 16;
        state2 = level_5;
        //state2 = level_1;
        return state2;
    case level_1:
        if (!mark) {
            lcd_clear();
            curr_X = 0, curr_Y = 1;
            levelY = 0;
            m_height = 2;
            objX = 7, objY = 1;
            int l1[2][16] = {
              {2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2},
              {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
            };
            memcpy(level[0], l1[0], sizeof(l1[0]));
            memcpy(level[1], l1[1], sizeof(l1[1]));
            lcd_createLevel(l1, levelY);
            lcd_cursor(0, 1);
            digitalWrite(RS, 1);
            lcd_write(6);
            mark = true;
        }
        if (levelY == 0 && curr_X == objX && curr_Y == objY) {
            lcd_clear();
            lcd_cursor(0, 0);
            lcd_writeString("level 1");
            lcd_cursor(0, 1);
            lcd_writeString("Complete");
            scoreNum = 1;
            delay(1000);
            mark = false;
            state2 = level_2;
        }
    case level_2:
        if (!mark) {
            lcd_clear();
            curr_X = 0, curr_Y = 1;
            levelY = 0;
            m_height = 2;
            objX = 15, objY = 0;
            int l2[2][16] = {
              {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
              {1, 1, 1, 2, 2, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 0}
            };
            memcpy(level[0], l2[0], sizeof(l2[0]));
            memcpy(level[1], l2[1], sizeof(l2[1]));
            lcd_createLevel(l2, levelY);
            lcd_cursor(0, 1);
            digitalWrite(RS, 1);
            lcd_write(6);
            mark = true;
        }
        if (levelY == 0 && curr_X == objX && curr_Y == objY) {
            lcd_clear();
            lcd_cursor(0, 0);
            lcd_writeString("level 2");
            lcd_cursor(0, 1);
            lcd_writeString("Complete");
            scoreNum = 2;
            delay(1000);
            mark = false;
            state2 = level_3;
        }
    case level_3:
        if (!mark) {
            lcd_clear();
            curr_X = 0, curr_Y = 0;
            levelY = 0;
            m_height = 4;
            objX = 15, objY = 1;
            int l3[4][16] = {
              {1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1},
              {2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2},
              {1, 1, 1, 1, 1, 1, 2, 2, 3, 2, 2, 2, 2, 2, 2, 0},
              {1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1}
            };
            memcpy(level[0], l3[0], sizeof(l3[0]));
            memcpy(level[1], l3[1], sizeof(l3[1]));
            memcpy(level[2], l3[2], sizeof(l3[2]));
            memcpy(level[3], l3[3], sizeof(l3[3]));
            lcd_createLevel(l3, levelY);
            lcd_cursor(0, 0);
            digitalWrite(RS, 1);
            lcd_write(6);
            mark = true;
        }
        if (levelY == 2 && curr_X == objX && curr_Y == objY) {
            lcd_clear();
            lcd_cursor(0, 0);
            lcd_writeString("level 3");
            lcd_cursor(0, 1);
            lcd_writeString("Complete");
            scoreNum = 3;
            delay(1000);
            mark = false;
            state2 = level_4;
        }
    case level_4:
        if (!mark) {
            //
            lcd_clear();
            curr_X = 0, curr_Y = 0;
            levelY = 0;
            m_height = 4;
            objX = 15, objY = 0;
            int l4[4][16] = {
              {1, 1, 1, 1, 1, 1, 3, 1, 1, 3, 1, 1, 1, 1, 1, 1},
              {2, 1, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2},
              {1, 2, 1, 3, 1, 1, 2, 2, 3, 2, 2, 2, 2, 2, 2, 0},
              {1, 1, 2, 3, 1, 3, 1, 2, 1, 1, 1, 1, 1, 3, 1, 1}
            };
            memcpy(level[0], l4[0], sizeof(l4[0]));
            memcpy(level[1], l4[1], sizeof(l4[1]));
            memcpy(level[2], l4[2], sizeof(l4[2]));
            memcpy(level[3], l4[3], sizeof(l4[3]));
            lcd_createLevel(l4, levelY);
            lcd_cursor(0, 0);
            digitalWrite(RS, 1);
            lcd_write(6);
            mark = true;
        }
        if (levelY == 0 && curr_X == objX && curr_Y == objY) {
            lcd_clear();
            lcd_cursor(0, 0);
            lcd_writeString("level 4");
            lcd_cursor(0, 1);
            lcd_writeString("Complete");
            scoreNum = 4;
            delay(1000);
            mark = false;
            state2 = level_5;
        }
    case level_5:
        if (!mark) {
            lcd_clear();
            digitalWrite(S4, 0);
            curr_X = 0, curr_Y = 1;
            levelY = 0;
            m_height = 6;
            objX = 7, objY = 0;
            int l5[6][16] = {
              {2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
              {2, 3, 3, 2, 2, 2, 3, 0, 2, 2, 2, 2, 2, 2, 2, 2},
              {2, 2, 3, 3, 1, 3, 1, 1, 3, 1, 2, 2, 2, 2, 2, 2},
              {2, 1, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 2, 3, 1, 1},
              {2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2},
              {2, 2, 1, 1, 1, 2, 1, 2, 2, 2, 1, 1, 1, 3, 1, 1}
            };
            memcpy(level[0], l5[0], sizeof(l5[0]));
            memcpy(level[1], l5[1], sizeof(l5[1]));
            memcpy(level[2], l5[2], sizeof(l5[2]));
            memcpy(level[3], l5[3], sizeof(l5[3]));
            memcpy(level[4], l5[4], sizeof(l5[4]));
            memcpy(level[5], l5[5], sizeof(l5[5]));
            lcd_createLevel(l5, levelY);
            lcd_cursor(0, 1);
            digitalWrite(RS, 1);
            lcd_write(6);
            mark = true;
            currentTime = millis();
            led_count = 9;
            writeDisplay(num_1);
        }
        if (millis() - currentTime > 2000) {
            currentTime = millis();
            led_count--;
            switch (led_count) {
            case 0:
                stuck = true;
                game = false;
                digitalWrite(S3, 1);
                writeDisplay(num_0);
                delay(1000);
                digitalWrite(S4, 1);
                scoreNum = 4;
            }
        }
        if (levelY == 0 && curr_X == objX && curr_Y == objY) {
            state2 = level_3;
            lcd_clear();
            lcd_cursor(0, 0);
            lcd_writeString("level 5");
            lcd_cursor(0, 1);
            lcd_writeString("Complete");
            delay(1000);
            mark = false;
            game = false;
            score = true;
            scoreNum = 5;
        }
        j_x = analogRead(JS_X);
        j_y = analogRead(JS_Y);
        //up
        if (!stuck) {
            if (j_x < 300) {
                if (curr_Y == 0) {
                    if (levelY + 2 < m_height) {
                        levelY += 2;
                        curr_Y = 1;
                        lcd_createLevel(level, levelY);
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(6);
                        delay(500);
                    }
                }
                else if (curr_Y == 1) {
                    if (level[levelY + 1][curr_X] != 1) {
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(2);
                        curr_Y = 0;
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(6);
                        delay(500);
                    }
                }
            }
            //down
            else if (j_x > 800) {
                if (curr_Y == 0) {
                    if (level[levelY][curr_X] != 1) {
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(level[levelY + 1][curr_X]);
                        curr_Y = 1;
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(6);
                        delay(500);
                    }
                }
                else if (curr_Y == 1) {
                    if (levelY - 2 > -1) {
                        levelY -= 2;
                        curr_Y = 0;
                        lcd_createLevel(level, levelY);
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(6);
                        delay(500);
                    }
                }
            }
            //left
            else if (j_y > 800) {
                if ((curr_X - 1) > -1) {
                    if (level[levelY][curr_X - 1] != 1 && level[levelY + 1][curr_X - 1] == 1) {
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(2);
                        curr_Y = 1;
                        curr_X--;
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(6);
                        delay(500);
                    }
                    else {
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(2);
                        curr_X--;
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(6);
                        delay(500);
                    }
                }
            }
            //right
            else if (j_y < 300) {
                Serial.println(curr_X);
                if ((curr_X + 1) < 16) {
                    if (level[levelY + 1][curr_X + 1] == 3 && level[levelY][curr_X + 1] == 3) {
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(2);
                        curr_Y = 1;
                        curr_X++;
                        levelY = 0;
                        lcd_createLevel(level, levelY);
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(4);
                        delay(500);
                        stuck = true;
                    }
                    else if (level[levelY][curr_X + 1] == 2) {
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(2);
                        curr_Y = 1;
                        curr_X++;
                        lcd_cursor(curr_X, curr_Y);
                        digitalWrite(RS, 1);
                        lcd_write(6);
                        delay(500);
                    }
                    else {
                        if (level[levelY + 1][curr_X + 1] == 2) {
                            lcd_cursor(curr_X, curr_Y);
                            digitalWrite(RS, 1);
                            lcd_write(2);
                            curr_X++;
                            lcd_cursor(curr_X, curr_Y);
                            digitalWrite(RS, 1);
                            lcd_write(6);
                            delay(500);
                        }
                    }
                }
            }
        }
        return state2;
    }
}
//==================================================================
void setup() {
    unsigned char i = 0;
    tasks[i].state = Menu_INIT;
    tasks[i].period = 50;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &Menu_Tick;
    i++;
    tasks[i].state = Game_INIT;
    tasks[i].period = 1;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &Game_Tick;
    delay_gcd = 1;
    for (unsigned int i = 3; i < 14; ++i) {
        pinMode(i, OUTPUT);
    }
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    pinMode(S4, OUTPUT);
    Serial.begin(9600);
    delay(50);
    //LCD init
    lcd_init();
    //IR init
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TIMSK1 = 1;
}
ISR(TIMER1_OVF_vect) {
    IR_state = 0;
    TCCR1B = 0;
}
//==================================================================
void loop() {
    if (!game) {
        for (i = 0; i < tasksHalf; ++i) {
            if ((millis() - tasks[i].elapsedTime) >= tasks[i].period) {
                tasks[i].state = tasks[i].TickFct(tasks[i].state);
                tasks[i].elapsedTime = millis();
            }
        }
    }
    else {
        for (i = tasksHalf; i < tasksNum; ++i) {
            if ((millis() - tasks[i].elapsedTime) >= tasks[i].period) {
                tasks[i].state = tasks[i].TickFct(tasks[i].state);
                tasks[i].elapsedTime = millis();
            }
        }
    }
}