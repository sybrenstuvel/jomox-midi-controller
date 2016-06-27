#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const float UPDATE_ALPHA = 0.01;

// MIDI definitions
const int CONTROL_CHANGE = 0xC0;
const int MIDI_CHANNEL = 10;  // base-1

// Jomox M.Brane 11 MIDI sound parameters
const int CC_DECAY = 110;
const int CC_M1_PITCH = 90;
const int CC_M1_DAMPEN = 92;
const int CC_M2_PITCH = 94;
const int CC_M2_DAMPEN = 96;

float decay = 0;
float m1_pitch = 0;
float m1_dampen = 0;
float m2_pitch = 0;
float m2_dampen = 0;

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

void setup() {
  MIDI.begin(MIDI_CHANNEL);
}

void loop() {
  update_controllers();
}

void send_midi_control(int midi_controller_nr, int new_rounded_value, bool is_twobytes) {
  if (is_twobytes) {
    send_midi_control(midi_controller_nr, new_rounded_value >> 7, false);
    send_midi_control(midi_controller_nr + 1, new_rounded_value, false);
  } else {
    MIDI.sendControlChange(midi_controller_nr, new_rounded_value, MIDI_CHANNEL);
  }
}

