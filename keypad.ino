// #include <Keyboard.h> // uncomment if needed - not present in 1.6.5

const int COLUMNS = 4;
const int ROWS = 5;
const int HB_LED = 17;

#define HEARTBEAT_ENABLED 1

// These are the column pins. They're configured for input with
// internal pullup

int input_pins[COLUMNS] = { 20, 19, 18, 15 } ;

// These are the row strobes. They're configured for output
// and initially all set to high. Individual pins are set to
// low to read that row. Only one row is low at any time.

int strobe_pins[ROWS] = { 8, 7, 6, 5, 4 };

unsigned long key_state[ROWS][COLUMNS];

// Codes for numeric keypad
// NUM  /   *   -
//  7   8   9   +
//  4   5   6   x
//  1   2   3   x
//  0   x  . ENTER

int keycode[ROWS][COLUMNS] = { { 0xDB, 0xDC, 0xDD, 0xDE },
                               { 0xE7, 0xE8, 0xE9, 0xDF },
                               { 0xE4, 0xE5, 0xE6, 0    },
                               { 0xE1, 0xE2, 0xE3, 0 },
                               { 0xEA, 0,    0xEB, 0XE0 } };
                      
int strobe_row = 0;
int q = 0;

void setup() {
  int cnt;
  int cnt2;

#ifdef HEARTBEAT_ENABLED  
  pinMode(HB_LED, OUTPUT);
#endif
  
  for (cnt = 0; cnt < ROWS; cnt++) {
    pinMode(strobe_pins[cnt], OUTPUT);
    digitalWrite(strobe_pins[cnt], HIGH);

    for (cnt2 = 0; cnt2 < COLUMNS; cnt2++) key_state[cnt][cnt2] = 0;
  }

  for (cnt = 0; cnt < COLUMNS; cnt++) {
    pinMode(input_pins[cnt], INPUT_PULLUP);
  }

  Keyboard.begin();
}

const int DEBOUNCE_MS = 20;

bool debounce(unsigned long t_now, unsigned long t_prev) {
  unsigned long diff;

  diff = t_now - t_prev; // need to check for underflow?

  if (diff <= DEBOUNCE_MS) return true;
  else return false;
}

void loop() {
  unsigned long tick_now = millis();
  int cnt;

#ifdef HEARTBEAT_ENABLED
  if (q == 0) digitalWrite(HB_LED, LOW);
  else if (q == 128) digitalWrite(HB_LED, HIGH);
  q++;
  q &= 0xff;
  // should just make this into an unsigned char, but keeping it as int
  // in case we want to adjust heartbeat freq.
#endif

  // since we use non zero to indicate pressed state, we need
  // to handle the edge case where millis() returns 0

  if (tick_now == 0) tick_now = 1;

  // every time we enter this loop, we're reading only the switches
  // on strobe_row
  
  if (strobe_row >= ROWS) strobe_row = 0;
  
  digitalWrite(strobe_pins[strobe_row], LOW);
  delay(2); // give it some time to stabilize just in case

  // We check all the switches in the row

  for (cnt = 0; cnt < COLUMNS; cnt++) {
    // ignore state change for pin if in debounce period
    if (key_state[strobe_row][cnt] != 0)
      if (debounce(tick_now, key_state[strobe_row][cnt]) == true)
        continue;

    if (digitalRead(input_pins[cnt]) == HIGH) {
      if (key_state[strobe_row][cnt] != 0) {
        Keyboard.release(keycode[strobe_row][cnt]);
        key_state[strobe_row][cnt] = 0;
      }
    } else {
      if (key_state[strobe_row][cnt] == 0) {
        Keyboard.press(keycode[strobe_row][cnt]);
        key_state[strobe_row][cnt] = tick_now;
      }
    }
  }

  digitalWrite(strobe_pins[strobe_row], HIGH);
  strobe_row++;
  delay(5);
}
