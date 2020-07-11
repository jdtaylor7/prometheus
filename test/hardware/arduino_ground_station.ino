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

char buf[PACKET_LEN];
float a1_f = 0.0;
float a2_f = 0.1;
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

void heartbeat()
{
  a1_i = (int)(sin(a1_f) * 1000);
  a2_i = (int)(a2_f * 1000);
  a3_i = (int)(a3_f * 1000);
  r1_i = (int)(r1_f * 1000);
  r2_i = (int)(r2_f * 1000);
  r3_i = (int)(r3_f * 1000);
  
  int r1 = random(50);
  a1_i = (a1_i * 0.95) + r1;
  int r2 = random(50);
  a2_i = (a2_i * 0.95) + r2;

  sprintf(buf, "%c%05i,%05i,%05i,%05i,%05i,%05i", START_SYMBOL, a1_i, a2_i, a3_i, r1_i, r2_i, r3_i);
  Serial.println(buf);

//  a2_f += 0.001;
  a1_f += 0.05;
  pause(DELAY_TIME_MS);
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
  heartbeat();
}
