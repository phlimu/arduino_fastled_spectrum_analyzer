
#include <FastLED.h>            // FastLED library for addressable LEDs

static const uint8_t DATA_PIN = 22;
static const uint8_t STROBE_PIN = 4;
static const uint8_t RESET_PIN = 5;
static const uint8_t COLUMN = 14;
static const uint8_t ROWS = 21;
static const uint16_t NUM_LEDS = uint16_t(COLUMN) * uint16_t(ROWS);
static const EOrder LED_ORDER = GRB;
static const uint8_t BRIGHTNESS = 50; // Lower value for longer lifespan and lower current draw
static const uint16_t NOISECOMP = 150; // Noise floor compensation; 120 is good starting point
static const uint8_t BOOST = 6;      // Analog response boost for weaker signals

// Matrix Definition
CRGB leds[NUM_LEDS];      // Memory block for all LEDs

struct LEDCell {
  uint8_t hue;
  uint8_t sat;
  uint8_t val;
  uint16_t nled;
  bool active;
};

static LEDCell colors[COLUMN][ROWS];   // Color state matrix

// Globals
static int MSGEQ_Bands[COLUMN];    // Instantaneous band samples
static uint16_t DELTA;            // Scaling factor for column levels
static int hue_rainbow = 0;
static unsigned long rainbow_time = 0;
static unsigned long time_change = 0;
static uint8_t effect = 2;        // Startup effect index

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////------------------- SETUP ----------------------////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  // Serial.begin(57600);      // Enable this for serial monitor or troubleshooting

  pinMode(DATA_PIN, OUTPUT);
  pinMode(STROBE_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  DELTA = max<int>(1, (1024 / ROWS) - BOOST); // Derive scaling factor with safety clamp

  int count = 0;
  for (int col = 0; col < COLUMN; col++) { // Number the physical LED index with snake wiring support
    for (int row = 0; row < ROWS; row++) {
      int targetRow = (col % 2 == 0) ? row : (ROWS - row - 1);
      colors[col][targetRow].nled = count;
      colors[col][targetRow].active = false;
      count++;
    }
  }

  FastLED.addLeds<WS2813, DATA_PIN, LED_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  rainbow_time = millis();
  time_change = millis();
}


void loop()
{
  readMSGEQ7();                                  // Call to function that reads MSGEQ7 IC's via analogue inputs.

  if (millis() - time_change > 3000)             // Code that establishes how often to change effect. 1000 = 1 Second
  {
    //effect = 2;                                  // Enable this line to set a fixed mode
    effect++;                                  // Enable this line to cycle through different modes
    if (effect > 7)
    {
      effect = 0;
    }
    time_change = millis();
  }


  switch (effect)                                // Case logic to determine which color effect to use
  {
    case 0:                                      // Full column; each band different color; color gradient within each band
      rainbow_dot();
      full_column();
      updateHSV();
      break;

    case 1:                                      // Full column; each band the same color; gradual simultaneous color change across all bands
      if (millis() - rainbow_time > 15)
      {
        dynamic_rainbow();
        rainbow_time = millis();
      }
      full_column();
      updateHSV();
      break;

    case 2:                                      // Full column; each band a different static rainbow color for the specified interval
      if (millis() - rainbow_time > 600)
      {
        rainbow_column();
        rainbow_time = millis();
      }
      full_column();
      updateHSV();
      break;

    case 3:                                      // Full column; all bands same static color
      if (millis() - rainbow_time > 15)
      {
        total_color_hsv(255, 255, 255);
        rainbow_time = millis();
      }
      full_column();
      updateHSV();
      break;

    case 4:                                      // Dot column; each column a different static rainbow color
      if (millis() - rainbow_time > 15)
      {
        rainbow_dot();
        rainbow_time = millis();
      }
      full_column_dot();
      updateHSV();
      break;
    case 5:                                      // Dot column; each band the same color; gradual simultaneous color change across all bands
      if (millis() - rainbow_time > 15)
      {
        dynamic_rainbow();
        rainbow_time = millis();
      }
      full_column_dot();
      updateHSV();
      break;

    case 6:                                      // Dot column; each band a different static rainbow color
      if (millis() - rainbow_time > 15)
      {
        rainbow_column();
        rainbow_time = millis();
      }
      full_column_dot();
      updateHSV();
      break;

    case 7:                                      // Dot column; all bands same static color
      total_color_hsv(55, 255, 255);
      full_column_dot();
      updateHSV();
      break;
  }
  delay(30);                                     // Refresh rate; Values 20 thru 30 should look realistic
}

void readMSGEQ7(void) {
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(RESET_PIN, LOW);

  for (int band = 0; band < COLUMN; band += 2) {
    digitalWrite(STROBE_PIN, LOW);
    delayMicroseconds(30);
    MSGEQ_Bands[band] = max(0, analogRead(A0) - int(NOISECOMP));

    if (band + 1 < COLUMN) {
      MSGEQ_Bands[band + 1] = max(0, analogRead(A1) - int(NOISECOMP));
    }

    digitalWrite(STROBE_PIN, HIGH);
  }
}

void updateHSV(void) {
  int total_level = 0;

  for (int col = 0; col < COLUMN; col++) {
    for (int row = 0; row < ROWS; row++) {
      LEDCell &cell = colors[col][row];
      if (cell.active) {
        total_level += row;
        leds[cell.nled] = CHSV(cell.hue, cell.sat, cell.val);
      } else {
        leds[cell.nled] = CRGB::Black;
      }
    }
  }

  // Optional dynamic brightness scaling (uncomment to enable)
  // uint8_t dynamicBrightness = constrain((total_level * 30) / (COLUMN * ROWS), 16, BRIGHTNESS);
  // FastLED.setBrightness(dynamicBrightness);

  FastLED.show();
}

void full_column(void) {
  for (int col = 0; col < COLUMN; col++) {
    int nlevel = constrain(MSGEQ_Bands[col] / int(DELTA), 0, ROWS - 1);
    for (int row = 0; row < ROWS; row++) {
      colors[col][row].active = (row <= nlevel);
    }
  }
}

void full_column_dot(void) {
  for (int col = 0; col < COLUMN; col++) {
    int nlevel = constrain(MSGEQ_Bands[col] / int(DELTA), 0, ROWS - 1);
    for (int row = 0; row < ROWS; row++) {
      colors[col][row].active = (row == nlevel);
    }
  }
}

void total_color_hsv(int h, int s, int v)
{
  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      colors[i][j].hue = h;
      colors[i][j].sat = s;
      colors[i][j].val = v;
    }
  }
}

void rainbow_column(void) {
  int hue = 0;
  for (int col = 0; col < COLUMN; col++) {
    for (int row = 0; row < ROWS; row++) {
      colors[col][row].hue = hue;
      colors[col][row].sat = 230;
      colors[col][row].val = 240;
    }
    hue = (hue + 18) % 256;
  }
}

void rainbow_dot(void) {
  int hue = 36;
  for (int col = 0; col < COLUMN; col++) {
    for (int row = 0; row < ROWS; row++) {
      colors[col][row].hue = hue;
      colors[col][row].sat = 230;
      colors[col][row].val = 240;
      hue = (hue + 5) % 256;
    }
  }
}

void dynamic_rainbow(void) {
  for (int col = 0; col < COLUMN; col++) {
    for (int row = 0; row < ROWS; row++) {
      colors[col][row].hue = hue_rainbow;
      colors[col][row].sat = 230;
      colors[col][row].val = 240;
    }
  }
  hue_rainbow = (hue_rainbow + 1) % 256;
}
