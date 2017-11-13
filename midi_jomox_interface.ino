//#define DEBUG
#define DO_MIDI

#if !defined(DEBUG) && defined(DO_MIDI)
// Source of MIDI library: https://github.com/FortySevenEffects/arduino_midi_library
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();
#endif

const float UPDATE_ALPHA = 0.1;
const int DEBOUNCE_DIGITAL_PIN_TIME = 50; // milliseconds
const int TIMING_DEBUG_PIN = 5;

// Multiplexer selector
const int MPLEX_SELECT_1 = 10;
const int MPLEX_SELECT_2 = 11;
const int MPLEX_SELECT_3 = 12;
const int MPLEX_DISABLE = 9;
int last_mplex_select = -1;

// MIDI definitions
const int CONTROL_CHANGE = 0xC0;
const int MIDI_CHANNEL = 10;  // base-1
const int NOTE_NUMBER = 38;

// Hit detection threshold values
const int HDT_READ_PEAK_DURATION = 5000; // in microseconds
const uint16_t PIEZO_MAX_VALUE = 500;
const int32_t HDT_MIN_THRESHOLD = 30;

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
                  int bit_shifts = 2, bool two_bytes = true)
      : last_read_value(0), last_midi_value(0),
        midi_value(0), last_smoothed_value(0), smoothed_value(0),
        input_pin(input_pin), mplex_select(mplex_select),
        midi_cc(midi_cc),
        bit_shifts(bit_shifts), two_bytes(two_bytes)
    {
      strncpy(this->name, name, 16);
      this->name[15] = 0;
    }

    void update();
    void reset();
};

#define INPUT_BLOCK_LEFT A4
#define INPUT_BLOCK_RIGHT A5

// Column 1
AnalogueInput m1_pitch("m1_pitch", INPUT_BLOCK_LEFT, 7, 90);
AnalogueInput m1_dampen("m1_dampen", INPUT_BLOCK_LEFT, 5, 92);
AnalogueInput m12_coupling("m12_coupling", INPUT_BLOCK_LEFT, 0, 100);
AnalogueInput lfo_wave("lfo_wave", INPUT_BLOCK_LEFT, 3, 119, 7, false);

// Column 2
AnalogueInput m2_pitch("m2_pitch", INPUT_BLOCK_LEFT, 6, 94);
AnalogueInput m2_dampen("m2_dampen", INPUT_BLOCK_LEFT, 4, 96);
AnalogueInput m21_coupling("m21_coupling", INPUT_BLOCK_LEFT, 1, 102);
AnalogueInput lfo_intensity("lfo_intensity", INPUT_BLOCK_LEFT, 2, 121, 3, false);

// Column 3
AnalogueInput noise("noise", INPUT_BLOCK_RIGHT, 7, 109, 3, false);
AnalogueInput noise_filter("noise_filter", INPUT_BLOCK_RIGHT, 5, 112, 3, false);
AnalogueInput metal_noise_a("metal_noise_a", INPUT_BLOCK_RIGHT, 3, 106);
AnalogueInput lfo_speed("lfo_speed", INPUT_BLOCK_RIGHT, 0, 122, 3, false);

// Column 4
AnalogueInput decay("decay", INPUT_BLOCK_RIGHT, 6, 110, 3, false);
AnalogueInput gate("gate", INPUT_BLOCK_RIGHT, 4, 114, 3, false);
AnalogueInput lfo_select("lfo_select", INPUT_BLOCK_RIGHT, 1, 120, 3, false);
AnalogueInput lfo_one_shot("lfo_one_shot", INPUT_BLOCK_RIGHT, 2, 123, 3, false);

AnalogueInput threshold_decay("threshold_decay", A3, 0, 0, 0, false);

AnalogueInput* analogue_inputs[] = {
  &m1_pitch,
  &m1_dampen,
  &m12_coupling,
  &lfo_wave,
  &m2_pitch,
  &m2_dampen,
  &m21_coupling,
  &lfo_intensity,
  &noise,
  &noise_filter,
  &metal_noise_a,
  &lfo_speed,
  &decay,
  &gate,
  &lfo_select,
  &lfo_one_shot,
  &threshold_decay,
  NULL
};


const int LED_1_PIN = 8;
const int LED_2_PIN = 7;

class Piezo {
  public:
    char name[16];

    int piezo_pin;
    int reset_pin;
    int led_pin;

    bool is_peaking;
    uint16_t piezo_level;
    uint16_t last_level;
    uint16_t last_hit_level;
    uint32_t hit_detect_time;
    uint32_t time_since_hit;

    bool hit_detected;
    unsigned int hit_threshold;

    Piezo(const char *name, int piezo_pin, int reset_pin, int led_pin)
      : piezo_pin(piezo_pin), reset_pin(reset_pin), led_pin(led_pin),
        is_peaking(false), hit_detected(false), hit_threshold(HDT_MIN_THRESHOLD),
        last_hit_level(0)
    {
      strncpy(this->name, name, 16);
      this->name[15] = 0;
    }

    void update();
};

Piezo piezo_head("head", A0, 3, LED_1_PIN);
// Piezo piezo_rim("rim", A2, 4, LED_2_PIN);


// Digital inputs.
const int BUTTON_1_PIN = 4;
int button1 = 0;
int last_button1_read_time = 0;
int debounced_button1 = 0;

const int BUTTON_2_PIN = 2;
int button2 = 0;
int last_button2_read_time = 0;
int debounced_button2 = 0;


void send_midi_control(int midi_controller_nr, int new_rounded_value, bool is_twobytes) {
#ifdef DO_MIDI
#ifdef DEBUG
  Serial.print("MIDI CC: ctrl=");
  Serial.print(midi_controller_nr);
  Serial.print(" value=");
  Serial.println(new_rounded_value);
  (void)is_twobytes;
#else
  if (is_twobytes) {
    send_midi_control(midi_controller_nr, new_rounded_value >> 7, false);
    send_midi_control(midi_controller_nr + 1, new_rounded_value, false);
  } else {
    MIDI.sendControlChange(midi_controller_nr, new_rounded_value, MIDI_CHANNEL);
  }
#endif
#endif
}

void send_midi_note(int velocity) {
#ifdef DO_MIDI
#ifdef DEBUG
  Serial.print("MIDI note: velocity=");
  Serial.println(velocity);
#else
  MIDI.sendNoteOn(NOTE_NUMBER, velocity, MIDI_CHANNEL);
#endif
#endif
}

void AnalogueInput::update()
{
  // Set the multiplexer
  digitalWrite(MPLEX_SELECT_1, (mplex_select >> 0) & 1 ? HIGH : LOW);
  digitalWrite(MPLEX_SELECT_2, (mplex_select >> 1) & 1 ? HIGH : LOW);
  digitalWrite(MPLEX_SELECT_3, (mplex_select >> 2) & 1 ? HIGH : LOW);
  last_mplex_select = mplex_select;

  // Read the value, smoothing it over time.
  last_read_value = analogRead(input_pin);
  float diff = last_read_value - smoothed_value;
  smoothed_value += diff * UPDATE_ALPHA;

  // Don't bother sending out small updates.
  if (fabs(smoothed_value - last_smoothed_value) < 2) return;
  last_smoothed_value = smoothed_value;

  // Channel 0 means "don't send MIDI at all"
  if (midi_cc == 0) return;

  // Compute which value to send over the MIDI channel.
  last_midi_value = midi_value;
  midi_value = round(smoothed_value) >> bit_shifts;
  if (last_midi_value == midi_value) return;

#if defined(DEBUG) && defined(DO_MIDI)
  Serial.print("Input ");
  Serial.print(name);
  Serial.print(" @ ");
  Serial.print(input_pin);
  Serial.print("/");
  Serial.print(mplex_select);
  Serial.print(" = ");
  Serial.print(last_read_value);
  Serial.print(" => ");
  Serial.println(midi_value);
#endif

  // Send new value as MIDI control value.
  send_midi_control(midi_cc, midi_value, two_bytes);
}

void AnalogueInput::reset()
{
  midi_value = 0;
  smoothed_value = 0;
}

void update_controllers()
{
  static int controller_idx = 0;
  AnalogueInput *controller = analogue_inputs[controller_idx];

  if (!controller) {
    // Reset loop
    controller_idx = 0;
    controller = analogue_inputs[0];
    digitalWrite(TIMING_DEBUG_PIN, HIGH);
} else {
    digitalWrite(TIMING_DEBUG_PIN, LOW);
}

  controller->update();
  controller_idx++;
}

void reset_all_knobs()
{
  AnalogueInput **controller = analogue_inputs;

  while (*controller) {
    (*controller)->reset();
    controller++;
  }
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
  switch (input_pin) {
    case BUTTON_1_PIN:
      send_midi_note(value ? 0x00 : 0x7F);
      break;
    case BUTTON_2_PIN:
      if (!value) reset_all_knobs();
      break;
  }
}

uint32_t last_report_time = 0;

void Piezo::update()
{
    piezo_level = analogRead(piezo_pin);
    time_since_hit = (micros() - hit_detect_time) & 0xffffffff;

    if (piezo_level > hit_threshold) {
        if (piezo_level > last_level) {
            /* We're still going up in signal strength; wait until it goes down again
             * because then we've found our peak. */
        } else {
            hit_detected = true;
            last_hit_level = last_level;
            hit_detect_time = micros();

            // Send the midi note ASAP.
            int midi_velo = min(0x7F * (float)last_hit_level / PIEZO_MAX_VALUE, 0x7F);
            send_midi_note(midi_velo);

            #ifdef DEBUG
            Serial.print(micros());
            Serial.print(": Piezo '");
            Serial.print(name);
            Serial.print("' was hit at level ");
            Serial.println(last_hit_level);
            #endif

            // Reset the peak detector.
            digitalWrite(3, HIGH);
            do {
                Serial.write('.');
                delay(10);
            } while((piezo_level = analogRead(piezo_pin)) > HDT_MIN_THRESHOLD);
            Serial.write('\n');
            digitalWrite(3, LOW);

            hit_threshold = max(last_hit_level, PIEZO_MAX_VALUE);
            time_since_hit = 0;
            last_report_time = 0;
        }
    }
    else if (hit_threshold > HDT_MIN_THRESHOLD) {
        // Lower threshold dynamically, taking care of time that can wrap around.
        float decay_factor = threshold_decay.last_smoothed_value / 100.0f;
        int32_t time_since_detect = time_since_hit - HDT_READ_PEAK_DURATION;
        int32_t delta = decay_factor * time_since_detect / 1000;

        // The HDT_DECAYX_Y values are determined under the assumption of
        // a hit level of 1000, so we have to factor that in.
        delta = delta * last_hit_level / 1000;
        if (delta > last_hit_level - HDT_MIN_THRESHOLD) {
            hit_threshold = HDT_MIN_THRESHOLD;
        }
        else {
            hit_threshold = last_hit_level - delta;
        }

        // last_report_time = 0;
    }

    #ifdef DEBUG
    if (last_report_time == 0 || last_report_time + 1000000 < micros()) {
        last_report_time = micros();
        Serial.print(last_report_time);
        Serial.print(" level is ");
        Serial.print(piezo_level);
        Serial.print(" -- threshold is now ");
        Serial.print(hit_threshold);
        Serial.print(" -- threshold decay ");
        Serial.print(threshold_decay.last_smoothed_value  / 100.0f);
        Serial.println("");
    }
    #endif

    if (hit_detected) {
    }
    digitalWrite(LED_1_PIN, hit_detected ? HIGH : LOW);
    last_level = piezo_level;
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Yay! This is Sybren's Jomox interface");
  #ifndef DO_MIDI
  Serial.println("MIDI has been disabled for debugging purposes.");
  #endif
#else
#ifdef DO_MIDI
  MIDI.begin(MIDI_CHANNEL);
#endif
#endif

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);

  pinMode(MPLEX_DISABLE, OUTPUT);
  pinMode(MPLEX_SELECT_1, OUTPUT);
  pinMode(MPLEX_SELECT_2, OUTPUT);
  pinMode(MPLEX_SELECT_3, OUTPUT);
  pinMode(TIMING_DEBUG_PIN, OUTPUT);
  digitalWrite(MPLEX_DISABLE, LOW);

  pinMode(13, OUTPUT);
}

void loop() {
  update_controllers();

  read_button(button1, debounced_button1, BUTTON_1_PIN, last_button1_read_time);
  read_button(button2, debounced_button2, BUTTON_2_PIN, last_button2_read_time);

  piezo_head.update();
  //  piezo_rim.update();
}
