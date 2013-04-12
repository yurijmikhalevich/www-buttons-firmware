#include <MsTimer2.h>

#define PULSE_WIDTH_USEC 5                   // Импульс "загрузка/чтение" для 74HC165. 

const int casc_ds = 2; //data pin
const int casc_st_cp =  3; //latch pin
const int casc_sh_cp =  4; //clock pin
const int buzzer = 9; //buzzer pin
const int btn_sh_ld = 5;
const int btn_clk = 6;
const int btn_qh = 7;
const int fstart = 10;
const int reset = 11;
const int tmblr = 12;

//кнопки левые  : синяя - 4 (7); зелёная - 5 (5); жёлтая - 6 (3); красная - 7 (1).
//кнопки правые : синяя - 3 (8); зелёная - 2 (6); жёлтая - 1 (4); красная - 0 (2).

int buttons[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int digits[8] = {2, 4, 6, 8, 7, 5, 3, 1};

boolean casc_state[24] = {0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 
                          0, 0, 0, 0, 0, 0};
boolean numbers[23][8] = {{0, 0, 0, 0, 0, 0, 1, 1},
                          {1, 0, 0, 1, 1, 1, 1, 1},
                          {0, 0, 1, 0, 0, 1, 0, 1},
                          {0, 0, 0, 0, 1, 1, 0, 1},
                          {1, 0, 0, 1, 1, 0, 0, 1},
                          {0, 1, 0, 0, 1, 0, 0, 1},
                          {0, 1, 0, 0, 0, 0, 0, 1},
                          {0, 0, 0, 1, 1, 1, 1, 1},
                          {0, 0, 0, 0, 0, 0, 0, 1},
                          {0, 0, 0, 0, 1, 0, 0, 1},
                          {0, 0, 0, 0, 0, 0, 1, 0},
                          {1, 0, 0, 1, 1, 1, 1, 0},
                          {0, 0, 1, 0, 0, 1, 0, 0},
                          {0, 0, 0, 0, 1, 1, 0, 0},
                          {1, 0, 0, 1, 1, 0, 0, 0},
                          {0, 1, 0, 0, 1, 0, 0, 0},
                          {0, 1, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 1, 1, 1, 1, 0},
                          {0, 0, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 1, 0, 0, 0},
                          {1, 1, 1, 1, 1, 1, 1, 1},
                          {1, 1, 1, 1, 1, 1, 1, 0},
                          {1, 1, 1, 1, 1, 0, 1, 1}};
                          
int sounds[8] = {60, 80, 100, 120, 140, 160, 180, 200};
int order[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int beep_pointer = 0;
int beeped = 0;

boolean just_one_pressed; //флаг, показывающий, нажата ли хотя бы одна кнопка, важно для работы тумблера

/*void write_casc_state();
void set_casc_state(boolean state = 0);
int pow(int number, int exponent); //because internal does not work properly
void display_number(int at, int number); /*at should be in [1:8] and number in [0:21],
                                          where 0:9 is numbers without dot and 10-19 - with,
                                          20 completely switches of led and 21 blip only dot.*/
/*int readButtonsState(int j);
void beep();*/

void setup() {
  Serial.begin(9600);
  pinMode(casc_ds, OUTPUT);
  pinMode(casc_st_cp, OUTPUT);
  pinMode(casc_sh_cp, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(btn_sh_ld, OUTPUT);
  pinMode(btn_clk, OUTPUT);
  pinMode(btn_qh, INPUT);
  pinMode(fstart, INPUT);
  pinMode(reset, INPUT);
  pinMode(tmblr, INPUT);
  digitalWrite(btn_clk, HIGH);
  digitalWrite(btn_sh_ld, HIGH);
  resetApp();
  MsTimer2::set(100, mloop);
  MsTimer2::start();
}

void resetApp() {
  analogWrite(buzzer, 0);
  for (int i = 0; i < 8; ++i) {
    display_number(i + 1, 21);
    order[i] = 0;
  }
  Serial.write("Hello, reset!\n");
  just_one_pressed = false;
}

void loop() {
}

//главный цикл

void mloop() {
  boolean b = false;
  if (digitalRead(fstart) == HIGH) {
    return;
  } else if (digitalRead(reset) == HIGH) {
    resetApp();
  }
  if (digitalRead(tmblr) == HIGH && just_one_pressed) {
    return;
  }
  for (int i = 0; i < 8; ++i) {
    if (readButtonsState(i) == HIGH) {
      buttonPressed(digits[i]);
      analogWrite(buzzer, 200);
      b = true;
      just_one_pressed = true;
      if (digitalRead(tmblr) == HIGH) {
        return;
      }
    }
  }
  if (!b) {
    analogWrite(buzzer, 0);
  }
}

// конец

int readButtonsState(int j) {
  int state[8];
  digitalWrite(btn_sh_ld, LOW);
  digitalWrite(btn_sh_ld, HIGH);
  for (int i = 0; i < 8; ++i) {
    state[i] = digitalRead(btn_qh);
//    Serial.write(state[i]);
    digitalWrite(btn_clk, LOW);
    digitalWrite(btn_clk, HIGH);
  }
  return state[j];
}

void buttonPressed(int button) {
  for (int i = 0; i < 8; ++i) {
    if (order[i] == button) {
      return;
    } else if (order[i] == 0) {
      order[i] = button;
      display_number(button, i + 1);
      return;
    }
  }
}

void beep() {
//  MsTimer2::stop();
  if (beep_pointer != 8) {
    if (order[beep_pointer] != 0) {
      analogWrite(buzzer, sounds[order[beep_pointer++] - 1]);
//      MsTimer2::set(1000, beep);
    } else {
      analogWrite(buzzer, 0);
//      MsTimer2::set(250, beep);
    }
  }
//  MsTimer2::start();
}

void display_number(int at, int number) {
  int ds = 3 * (at - 1);
  int st_cp = ds + 1;
  int sh_cp = ds + 2;
  casc_state[st_cp] = 0;
  write_casc_state();
  casc_state[ds] = 0;
  write_casc_state();
  for (int i = 7; i >= 0; --i) {
    casc_state[sh_cp] = 0;
    write_casc_state();
    casc_state[ds] = numbers[number][i];
    write_casc_state();
    casc_state[sh_cp] = 1;
    write_casc_state();
    casc_state[ds] = 0;
    write_casc_state();
  }
  casc_state[st_cp] = 1;
  write_casc_state();
}

void write_casc_state() {
  digitalWrite(casc_st_cp, LOW);
  for (int i = 2; i >= 0; --i) {
    int data = 0;
    for (int j = 7; j >= 0; --j) {
      boolean cstate = casc_state[8 * i + j];
      if (cstate) {
        data += pow(int(2 * cstate), int(7 - j));
      }
    }
    shiftOut(casc_ds, casc_sh_cp, LSBFIRST, data);
  }
  digitalWrite(casc_st_cp, HIGH);
}

void set_casc_state(boolean state) {
  for (int i = 0; i < 24; ++i) {
    casc_state[i] = state;
  }
}

int pow(int number, int exponent) {
  int result = 1;
  for (int i = 0; i < exponent; ++i) {
    result *= number;
  }
  return result;
}

