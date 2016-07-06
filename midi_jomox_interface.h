#include <Arduino.h>

// Analogue inputs.
class AnalogueInput {
  public:
    char name[16];
    int last_read_value;
    int last_midi_value;

    int midi_value;
    float last_smoothed_value;
    float smoothed_value;

    // For reading the right knob.
    int input_pin;
    int mplex_select;

    // For translating to MIDI CC signals.
    int midi_cc;
    int bit_shifts;
    bool two_bytes;

    AnalogueInput(const char *name, int input_pin, int mplex_select, int midi_cc,
                  int bit_shifts = 2, bool two_bytes = true);
    void update();
    void reset();
};

void send_midi_control(int midi_controller_nr, int new_rounded_value, bool is_twobytes);
void send_midi_note(int velocity);

void update_controllers();
void reset_all_knobs();

void read_button(int &value, int &debounced_value, int input_pin, int &last_read_time);
void button_value_changed(int value, int input_pin);

void setup();
void loop();

