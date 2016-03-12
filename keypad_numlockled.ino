// These are the column pins, to be configured for input with
// internal pullup

int input_pins[4] = { 20, 19, 18, 15 } ;

// These are the row strobes. They're configured for output
// and initially all set to high. Individual pins are set to
// low to read that row. Only one row is low at any time.

int strobe_pins[5] = { 8, 7, 6, 5, 4 };
unsigned long key_state[5][4];
int statusLed = -1;

// Codes for numeric keypad
// NUM  /   *   -
//  7   8   9   +
//  4   5   6   x
//  1   2   3   x
//  0   x  . ENTER

int keycode[5][4] = { { 0xDB, 0xDC, 0xDD, 0xDE },
                      { 0xE7, 0xE8, 0xE9, 0xDF },
                      { 0xE4, 0xE5, 0xE6, 0    },
                      { 0xE1, 0xE2, 0xE3, 0 },
                      { 0xEA, 0,    0xEB, 0XE0 } };
                      
int strobe_row = 0;
int q = 0;
const int NUMLOCK_PIN = 3;
const int HEARTBEAT_PIN = 17;

void setup() {
  int cnt;
  int cnt2;

  // This is for the heartbeat
  
  pinMode(HEARTBEAT_PIN, OUTPUT);
  
  for (cnt = 0; cnt < 5; cnt++) {
    pinMode(strobe_pins[cnt], OUTPUT);
    digitalWrite(strobe_pins[cnt], HIGH);

    for (cnt2 = 0; cnt2 < 4; cnt2++) key_state[cnt][cnt2] = 0;
  }

  for (cnt = 0; cnt < 4; cnt++) {
    pinMode(input_pins[cnt], INPUT_PULLUP);
  }

  // set num lock off
  pinMode(NUMLOCK_PIN, OUTPUT);
  digitalWrite(NUMLOCK_PIN, LOW);
  
  Keyboard.begin();
}

bool debounce(unsigned long t_now, unsigned long t_prev) {
  unsigned long diff;

  diff = t_now - t_prev; // need to check for underflow?

  if (diff <= 20) return true;
  else return false;
}

void loop() {
  unsigned long tick_now = millis();
  int cnt;
  int ledStatus;

  // Num Lock

  ledStatus = Keyboard.getLedStatus();

  if (statusLed != ledStatus) {
    if ((ledStatus & LED_NUM_LOCK) == LED_NUM_LOCK) {
      digitalWrite(NUMLOCK_PIN, HIGH);
    } else {
      digitalWrite(NUMLOCK_PIN, LOW);
    }
    statusLed = ledStatus;
  }

  // Heartbeat LED. Comment this out if you don't want blinkenlights

  if (q == 0) digitalWrite(HEARTBEAT_PIN, LOW);
  else if (q == 128) digitalWrite(HEARTBEAT_PIN, HIGH);
  q++;
  if (q == 256) q = 0;
  
  // end of heartbeat code

  // since we use non zero to indicate pressed state, we need
  // to handle the edge case where millis() returns 0

  if (tick_now == 0) tick_now = 1;

  // every time we enter this loop, we're reading only the switches
  // on strobe_row
  
  if (strobe_row >= 5) strobe_row = 0;
  
  digitalWrite(strobe_pins[strobe_row], LOW);
  delay(2); // give it some time to stabilize just in case

  // We check all the switches in the row

  for (cnt = 0; cnt < 4; cnt++) {
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
