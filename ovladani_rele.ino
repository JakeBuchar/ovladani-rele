#include <Adafruit_NeoPixel.h>

#define RGB_PIN     48
#define NUM_PIXELS  1
#define RELAY_PIN   13
#define RELAY_ACTIVE_LOW false

// === TEST SETTINGS ===
#define TEST_DURATION_MS  5UL * 60UL * 1000UL  // 5 minutes total
#define TEST_INTERVAL_MS  2000                  // 2 seconds between open/close

Adafruit_NeoPixel pixel(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

bool relayEngaged = false;
int ledBrightness = 40;

// Automatic test state
bool autoTest = false;
unsigned long autoTestStart = 0;
unsigned long lastToggle = 0;
int cycleCount = 0;

char inputBuffer[50];
byte inputIndex = 0;

void setLedRed() {
  pixel.setBrightness(ledBrightness);
  pixel.setPixelColor(0, pixel.Color(255, 0, 0));
  pixel.show();
}

void setLedGreen() {
  pixel.setBrightness(ledBrightness);
  pixel.setPixelColor(0, pixel.Color(0, 255, 0));
  pixel.show();
}

void setLedBlue() {
  pixel.setBrightness(ledBrightness);
  pixel.setPixelColor(0, pixel.Color(0, 0, 255));
  pixel.show();
}

void setRelayPin(bool engage) {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, engage ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_PIN, engage ? HIGH : LOW);
  }
}

void setRelay(bool engage) {
  relayEngaged = engage;
  setRelayPin(engage);
  delay(50);

  if (relayEngaged) {
    setLedGreen();
    Serial.println("State: OPEN");
  } else {
    setLedRed();
    Serial.println("State: CLOSED");
  }
}

void startAutoTest() {
  autoTest = true;
  autoTestStart = millis();
  lastToggle = millis();
  cycleCount = 0;
  Serial.println();
  Serial.println("=== AUTO TEST STARTED ===");
  Serial.print("Duration: 5 minutes, interval: ");
  Serial.print(TEST_INTERVAL_MS / 1000);
  Serial.println("s");
  Serial.println("Send stop to abort");
  Serial.println();
  setRelay(true); // start open
}

void stopAutoTest(bool completed) {
  autoTest = false;
  setRelay(false); // safely close valve
  Serial.println();
  if (completed) {
    Serial.println("=== AUTO TEST COMPLETED ===");
  } else {
    Serial.println("=== AUTO TEST STOPPED ===");
  }
  Serial.print("Total cycles: ");
  Serial.println(cycleCount);
  Serial.println();
}

void processAutoTest() {
  if (!autoTest) return;

  unsigned long now = millis();

  // Check if 5 minutes have elapsed
  if (now - autoTestStart >= TEST_DURATION_MS) {
    stopAutoTest(true);
    return;
  }

  // Toggle every 2 seconds
  if (now - lastToggle >= TEST_INTERVAL_MS) {
    lastToggle = now;

    if (!relayEngaged) cycleCount++; // count full cycles (open+close = 1 cycle)

    // Print remaining time
    unsigned long remainingSec = (TEST_DURATION_MS - (now - autoTestStart)) / 1000;
    Serial.print("[");
    Serial.print(remainingSec / 60);
    Serial.print("m");
    Serial.print(remainingSec % 60);
    Serial.print("s left | cycle ");
    Serial.print(cycleCount);
    Serial.print("] -> ");

    setRelay(!relayEngaged);
  }
}

void printHelp() {
  Serial.println();
  Serial.println("Available commands:");
  Serial.println("  open             = open valve");
  Serial.println("  closed           = close valve");
  Serial.println("  test             = start 5min auto test");
  Serial.println("  stop             = stop auto test");
  Serial.println("  status           = current state");
  Serial.println("  help             = help");
  Serial.println();
}

void printStatus() {
  Serial.println();
  Serial.println("=== STATUS ===");
  Serial.print("Valve: ");
  Serial.println(relayEngaged ? "OPEN" : "CLOSED");
  Serial.print("Auto test: ");
  if (autoTest) {
    unsigned long remainingSec = (TEST_DURATION_MS - (millis() - autoTestStart)) / 1000;
    Serial.print("RUNNING (");
    Serial.print(remainingSec / 60);
    Serial.print("m");
    Serial.print(remainingSec % 60);
    Serial.println("s left)");
    Serial.print("Cycles: ");
    Serial.println(cycleCount);
  } else {
    Serial.println("OFF");
  }
  Serial.println();
}

void toLowerCase(char *text) {
  for (int i = 0; text[i]; i++) {
    if (text[i] >= 'A' && text[i] <= 'Z') text[i] += 32;
  }
}

void trimTrailingWhitespace(char *text) {
  int len = strlen(text);
  while (len > 0 && (text[len-1]=='\n'||text[len-1]=='\r'||text[len-1]==' '||text[len-1]=='\t')) {
    text[--len] = '\0';
  }
}

char* trimLeadingWhitespace(char *text) {
  while (*text == ' ' || *text == '\t') text++;
  return text;
}

void processCommand(char *rawCmd) {
  trimTrailingWhitespace(rawCmd);
  char *cmd = trimLeadingWhitespace(rawCmd);
  toLowerCase(cmd);
  if (strlen(cmd) == 0) return;

  if (strcmp(cmd, "open") == 0) {
    if (autoTest) { Serial.println("Stop the test first with: stop"); return; }
    setRelay(true);
  }
  else if (strcmp(cmd, "closed") == 0) {
    if (autoTest) { Serial.println("Stop the test first with: stop"); return; }
    setRelay(false);
  }
  else if (strcmp(cmd, "test") == 0) {
    if (autoTest) { Serial.println("Test already running!"); return; }
    startAutoTest();
  }
  else if (strcmp(cmd, "stop") == 0) {
    if (!autoTest) { Serial.println("Test is not running."); return; }
    stopAutoTest(false);
  }
  else if (strcmp(cmd, "status") == 0) printStatus();
  else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) printHelp();
  else {
    Serial.print("Unknown command: "); Serial.println(cmd);
    printHelp();
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  pixel.begin();
  pixel.setBrightness(ledBrightness);
  pixel.clear();
  pixel.show();

  pinMode(RELAY_PIN, OUTPUT);
  relayEngaged = false;
  setRelayPin(false);
  delay(50);
  setLedRed();

  Serial.println();
  Serial.println("=== ESP32-S3 VALVE TEST + AUTO ===");
  printHelp();
}

void loop() {
  // Process serial input
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      inputBuffer[inputIndex] = '\0';
      if (inputIndex > 0) processCommand(inputBuffer);
      inputIndex = 0;
    } else {
      if (inputIndex < sizeof(inputBuffer) - 1) {
        inputBuffer[inputIndex++] = c;
      }
    }
  }

  // Auto test logic (non-blocking)
  processAutoTest();

  delay(1);
}
