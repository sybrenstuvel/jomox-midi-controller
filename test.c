const float UPDATE_ALPHA = 0.4;
const int DEBOUNCE_DIGITAL_PIN_TIME = 50; // milliseconds

// Multiplexer selector
int mplex_select = 0;
const int MPLEX_SELECT_1 = 10;
const int MPLEX_SELECT_2 = 11;
const int MPLEX_SELECT_3 = 12;
const int MPLEX_DISABLE = 9;

// Analogue inputs.
float m1_pitch = 0;
float m1_dampen = 0;
float m2_pitch = 0;
float m2_dampen = 0;
float decay = 0;

// Digital inputs.
const int BUTTON_1_PIN = 2;
int button1 = 0;
int last_button1_read_time = 0;
int debounced_button1 = 0;

const int BUTTON_2_PIN = 4;
int button2 = 0;
int last_button2_read_time = 0;
int debounced_button2 = 0;

const int BUTTON_3_PIN = 7;
int button3 = 0;
int last_button3_read_time = 0;
int debounced_button3 = 0;

const int BUTTON_4_PIN = 8;
int button4 = 0;
int last_button4_read_time = 0;
int debounced_button4 = 0;


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

  Serial.print("Input ");
  Serial.print(input);
  Serial.print("/");
  Serial.print(mplex_select);
  Serial.print(" => ");
  Serial.println(new_rounded_value);
}

void update_controllers() {
//  update_controller(m1_pitch, A1, 0, false, 3);
//  update_controller(m1_dampen, A2, 0, false, 3);
//  update_controller(m2_pitch, A3, 0, false, 3);
  update_controller(m2_dampen, A4, 0, false, 3);
  update_controller(decay, A5, 0, false, 3);
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
//  Serial.print("BUTTON ");
//  Serial.print(input_pin);
//  Serial.print(" value ");
//  Serial.println(value);

  switch (input_pin) {
    case BUTTON_1_PIN:
      // MIDI.sendNoteOn(NOTE_NUMBER, value ? 0x7F : 0x00, MIDI_CHANNEL);
      //    Serial.println("BUTTON 1 toggled");
      break;
    case BUTTON_2_PIN:
      if (!value) {
        decay = 0;
      }
      break;
    case BUTTON_4_PIN:
      if (!value) {
        mplex_select = (mplex_select + 1) & 7;
        Serial.print("Switching mplex to ");
        Serial.println(mplex_select);
      }
      break;
  }
}

void setup() {
  //  MIDI.begin(MIDI_CHANNEL);
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_4_PIN, INPUT_PULLUP);
  
  pinMode(MPLEX_DISABLE, OUTPUT);
  pinMode(MPLEX_SELECT_1, OUTPUT);
  pinMode(MPLEX_SELECT_2, OUTPUT);
  pinMode(MPLEX_SELECT_3, OUTPUT);
  digitalWrite(MPLEX_DISABLE, LOW);

  pinMode(13, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("Yay!");
}

void loop() {
  update_controllers();

  read_button(button1, debounced_button1, BUTTON_1_PIN, last_button1_read_time);
  read_button(button2, debounced_button2, BUTTON_2_PIN, last_button2_read_time);
  read_button(button3, debounced_button3, BUTTON_3_PIN, last_button3_read_time);
  read_button(button4, debounced_button4, BUTTON_4_PIN, last_button4_read_time);

  digitalWrite(MPLEX_SELECT_1, (mplex_select >> 0) & 1 ? HIGH : LOW);
  digitalWrite(MPLEX_SELECT_2, (mplex_select >> 1) & 1 ? HIGH : LOW);
  digitalWrite(MPLEX_SELECT_3, (mplex_select >> 2) & 1 ? HIGH : LOW);
  
  delay(10);
}

