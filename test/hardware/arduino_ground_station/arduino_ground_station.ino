const unsigned int HEARTBEAT_LED = 13;  // Uno
//const unsigned int HEARTBEAT_LED = 7;  // Ground station
//const unsigned int HEARTBEAT_LED = 9;  // Ground station test pin

/*
 * For packet formatting, assume data comes in as floats values -10 to +10. We'll
 * operate with a precision of 0.001. Therefore, we'll multiply incoming floats
 * by 1000 and convert them to ints before uploading them to the viewer application.
 * The viewer will then divide them by 1000 before interpreting them further.
 */

const unsigned int DELAY_TIME_MS = 10;
const unsigned int PACKET_LEN = 37;
const char START_SYMBOL = '|';
const char STOP_SYMBOL = '\n';
const unsigned int FLOAT_CONVERSION_FACTOR = 1000;

float i = 0.0;

char buf[PACKET_LEN];
float a1_f = 0.0;
float a2_f = 0.2;
float a3_f = 0.0;
float r1_f = 0.0;
float r2_f = 0.0;
float r3_f = 0.0;

int a1_i = 0;
int a2_i = 0;
int a3_i = 0;
int r1_i = 0;
int r2_i = 0;
int r3_i = 0;

void pause(unsigned int delay_time)
{
  digitalWrite(HEARTBEAT_LED, HIGH);
  delay(delay_time / 2);
  digitalWrite(HEARTBEAT_LED, LOW);
  delay(delay_time / 2);
}

/*
 * Noise while floating in place.
 */
void sim1()
{
  float a_noise = 5;
  float r_noise = 100;
  float y_offset = 2000;

  // Position.
  a1_f = random(a_noise) - a_noise / 2;
  a2_f = random(a_noise) - a_noise / 2;
  a3_f = random(a_noise) - a_noise / 2;

  // Orientation.
  r1_f = random(r_noise) - r_noise / 2;
  r2_f = random(r_noise) - r_noise / 2;
  r3_f = random(r_noise) - r_noise / 2;

  // Scale data.
  a1_i = (int)(a1_f * 100);
  a2_i = (int)(a2_f * 100 + y_offset);
  a3_i = (int)(a3_f * 100);
  r1_i = (int)(r1_f * 100);
  r2_i = (int)(r2_f * 100);
  r3_i = (int)(r3_f * 100);

  // Build and send packet.
  sprintf(buf, "%c%05i,%05i,%05i,%05i,%05i,%05i", START_SYMBOL, a1_i, a2_i, a3_i, r1_i, r2_i, r3_i);
  Serial.println(buf);
}

/*
 * Cycle between taking off and landing.
 */
void sim2()
{
  float a_noise = 5;
  float r_noise = 100;
  float min_height = 2000;
  float max_height = 2000;

  // Position.
  a1_f = random(a_noise) - a_noise / 2;
  a2_f = random(a_noise) - a_noise / 2 + 40 * abs(sin(i));
  a3_f = random(a_noise) - a_noise / 2;

  // Orientation.
  r1_f = random(r_noise) - r_noise / 2;
  r2_f = random(r_noise) - r_noise / 2;
  r3_f = random(r_noise) - r_noise / 2;

  // Scale data.
  a1_i = (int)(a1_f * 100);
  a2_i = (int)(a2_f * 100);
  a3_i = (int)(a3_f * 100);
  r1_i = (int)(r1_f * 100);
  r2_i = (int)(r2_f * 100);
  r3_i = (int)(r3_f * 100);

  // Build and send packet.
  sprintf(buf, "%c%05i,%05i,%05i,%05i,%05i,%05i", START_SYMBOL, a1_i, a2_i, a3_i, r1_i, r2_i, r3_i);
  Serial.println(buf);

  i += 0.025;
}

void setup()
{
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(HEARTBEAT_LED, OUTPUT);

  // Initialize serial output.
  Serial.begin(9600);
}

void loop()
{
  sim1();
//  sim2();
  pause(DELAY_TIME_MS);
}
