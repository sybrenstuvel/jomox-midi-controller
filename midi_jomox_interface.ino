// Source of MIDI library: https://github.com/FortySevenEffects/arduino_midi_library
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const float UPDATE_ALPHA = 0.1;
const int DEBOUNCE_DIGITAL_PIN_TIME = 50; // milliseconds

// MIDI definitions
const int CONTROL_CHANGE = 0xC0;
const int MIDI_CHANNEL = 10;  // base-1
const int NOTE_NUMBER = 38;

// Jomox M.Brane 11 MIDI sound parameters
const int CC_DECAY = 110;
const int CC_M1_PITCH = 90;
const int CC_M1_DAMPEN = 92;
const int CC_M2_PITCH = 94;
const int CC_M2_DAMPEN = 96;

// Analogue inputs.
float decay = 0;
float m1_pitch = 0;
float m1_dampen = 0;
float m2_pitch = 0;
float m2_dampen = 0;

// Digital inputs.
const int BUTTON_1_PIN = 8;
int button1 = 0;
int last_button1_read_time = 0;
int debounced_button1 = 0;

const int BUTTON_2_PIN = 9;
int button2 = 0;
int last_button2_read_time = 0;
int debounced_button2 = 0;


float p_controller(const float & value, int input) {
  // Compute new value based on analog input.
  int curval = analogRead(input);
  float diff = curval - value;
  return value + diff * UPDATE_ALPHA;
}

void update_controller(float &value, int input, int midi_controller_nr, bool is_twobytes, int shift) {
  // Remember old value, as stored in Jomox
  int old_rounded_value = round(value) >> shift;
  
  value = p_controller(value, input);

  int new_rounded_value = round(value) >> shift;
  if (old_rounded_value == new_rounded_value) return;

  // Send new value as MIDI control value.
  send_midi_control(midi_controller_nr, new_rounded_value, is_twobytes);
}

void update_controllers() {
  update_controller(m1_pitch, A1, CC_M1_PITCH, true, 2);
  update_controller(m1_dampen, A0, CC_M1_DAMPEN, true, 2);
  update_controller(decay, A2, CC_DECAY, false, 3);
  update_controller(m2_pitch, A3, CC_M2_PITCH, true, 2);
}


void read_button(int &value, int &debounced_value, int input_pin, int &last_read_time) {
  // read the state of the switch into a local variable:
  int reading = digitalRead(input_pin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != value) {
    // reset the debouncing timer
    last_read_time = millis();
  }

  if ((millis() - last_read_time) > DEBOUNCE_DIGITAL_PIN_TIME) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != debounced_value) {
      debounced_value = value = reading;
      button_value_changed(debounced_value, input_pin);
    }
  }

  value = reading;
}

void button_value_changed(int value, int input_pin) {
  switch(input_pin) {
  case BUTTON_1_PIN:
    MIDI.sendNoteOn(NOTE_NUMBER, value ? 0x7F : 0x00, MIDI_CHANNEL);
    break;
  case BUTTON_2_PIN:
    if (value) {
      decay = 0;
      m1_pitch = 0;
      m1_dampen = 0;
      m2_pitch = 0;
      m2_dampen = 0;
    }
    break;
  }
}

void setup() {
  MIDI.begin(MIDI_CHANNEL);
  pinMode(BUTTON_1_PIN, INPUT);
}

void loop() {
  update_controllers();

  read_button(button1, debounced_button1, BUTTON_1_PIN, last_button1_read_time);
  read_button(button2, debounced_button2, BUTTON_2_PIN, last_button2_read_time);

  delay(10);
}

void send_midi_control(int midi_controller_nr, int new_rounded_value, bool is_twobytes) {
  if (is_twobytes) {
    send_midi_control(midi_controller_nr, new_rounded_value >> 7, false);
    send_midi_control(midi_controller_nr + 1, new_rounded_value, false);
  } else {
    MIDI.sendControlChange(midi_controller_nr, new_rounded_value, MIDI_CHANNEL);
  }
}

