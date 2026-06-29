#include <Adafruit_NeoPixel.h>

#define RGB_PIN     48
#define NUM_PIXELS  1
#define RELAY_PIN   13
#define RELAY_ACTIVE_LOW false

// === NASTAVENÍ TESTU ===
#define TEST_DOBA_MS      5UL * 60UL * 1000UL  // 5 minut celkem
#define TEST_PAUZA_MS     2000                  // 2 sekundy mezi open/close

Adafruit_NeoPixel pixel(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

bool releSepnute = false;
int ledBrightness = 40;

// Stav automatického testu
bool autoTest = false;
unsigned long autoTestStart = 0;
unsigned long posledniPrepnuti = 0;
int pocetCyklu = 0;

char inputBuffer[50];
byte inputIndex = 0;

void nastavLedCervena() {
  pixel.setBrightness(ledBrightness);
  pixel.setPixelColor(0, pixel.Color(255, 0, 0));
  pixel.show();
}

void nastavLedZelena() {
  pixel.setBrightness(ledBrightness);
  pixel.setPixelColor(0, pixel.Color(0, 255, 0));
  pixel.show();
}

void nastavLedModra() {
  pixel.setBrightness(ledBrightness);
  pixel.setPixelColor(0, pixel.Color(0, 0, 255));
  pixel.show();
}

void nastavRelePin(bool sepnout) {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, sepnout ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_PIN, sepnout ? HIGH : LOW);
  }
}

void nastavRele(bool sepnout) {
  releSepnute = sepnout;
  nastavRelePin(sepnout);
  delay(50);

  if (releSepnute) {
    nastavLedZelena();
    Serial.println("Stav: OPEN / OTEVRENO");
  } else {
    nastavLedCervena();
    Serial.println("Stav: CLOSED / ZAVRENO");
  }
}

void spustAutoTest() {
  autoTest = true;
  autoTestStart = millis();
  posledniPrepnuti = millis();
  pocetCyklu = 0;
  Serial.println();
  Serial.println("=== AUTO TEST SPUSTEN ===");
  Serial.print("Doba: 5 minut, pauza: ");
  Serial.print(TEST_PAUZA_MS / 1000);
  Serial.println("s");
  Serial.println("Pro zastaveni posli: stop");
  Serial.println();
  nastavRele(true); // začni otevřením
}

void zastavAutoTest(bool dokoncen) {
  autoTest = false;
  nastavRele(false); // bezpečně zavři ventil
  Serial.println();
  if (dokoncen) {
    Serial.println("=== AUTO TEST DOKONCEN ===");
  } else {
    Serial.println("=== AUTO TEST ZASTAVEN ===");
  }
  Serial.print("Celkem cyklu: ");
  Serial.println(pocetCyklu);
  Serial.println();
}

void zpracujAutoTest() {
  if (!autoTest) return;

  unsigned long ted = millis();

  // Zkontroluj jestli uplynulo 5 minut
  if (ted - autoTestStart >= TEST_DOBA_MS) {
    zastavAutoTest(true);
    return;
  }

  // Přepni každé 2 sekundy
  if (ted - posledniPrepnuti >= TEST_PAUZA_MS) {
    posledniPrepnuti = ted;

    if (!releSepnute) pocetCyklu++; // počítej celé cykly (open+close = 1 cyklus)

    // Výpis zbývajícího času
    unsigned long zbyvaSec = (TEST_DOBA_MS - (ted - autoTestStart)) / 1000;
    Serial.print("[");
    Serial.print(zbyvaSec / 60);
    Serial.print("m");
    Serial.print(zbyvaSec % 60);
    Serial.print("s zbývá | cyklus ");
    Serial.print(pocetCyklu);
    Serial.print("] → ");

    nastavRele(!releSepnute);
  }
}

void vypisNapovedu() {
  Serial.println();
  Serial.println("Dostupne prikazy:");
  Serial.println("  open / otevreno  = otevrit ventil");
  Serial.println("  closed / zavreno = zavrit ventil");
  Serial.println("  test             = spustit 5min auto test");
  Serial.println("  stop             = zastavit auto test");
  Serial.println("  status           = aktualni stav");
  Serial.println("  help             = napoveda");
  Serial.println();
}

void vypisStatus() {
  Serial.println();
  Serial.println("=== STATUS ===");
  Serial.print("Ventil: ");
  Serial.println(releSepnute ? "OPEN" : "CLOSED");
  Serial.print("Auto test: ");
  if (autoTest) {
    unsigned long zbyvaSec = (TEST_DOBA_MS - (millis() - autoTestStart)) / 1000;
    Serial.print("BEZ (zbývá ");
    Serial.print(zbyvaSec / 60);
    Serial.print("m");
    Serial.print(zbyvaSec % 60);
    Serial.println("s)");
    Serial.print("Cyklu: ");
    Serial.println(pocetCyklu);
  } else {
    Serial.println("VYPNUT");
  }
  Serial.println();
}

void prevedNaMalaPismena(char *text) {
  for (int i = 0; text[i]; i++) {
    if (text[i] >= 'A' && text[i] <= 'Z') text[i] += 32;
  }
}

void odstranMezeryNaKonci(char *text) {
  int len = strlen(text);
  while (len > 0 && (text[len-1]=='\n'||text[len-1]=='\r'||text[len-1]==' '||text[len-1]=='\t')) {
    text[--len] = '\0';
  }
}

char* odstranMezeryNaZacatku(char *text) {
  while (*text == ' ' || *text == '\t') text++;
  return text;
}

void zpracujPrikaz(char *rawCmd) {
  odstranMezeryNaKonci(rawCmd);
  char *cmd = odstranMezeryNaZacatku(rawCmd);
  prevedNaMalaPismena(cmd);
  if (strlen(cmd) == 0) return;

  if (strcmp(cmd, "open") == 0 || strcmp(cmd, "otevreno") == 0) {
    if (autoTest) { Serial.println("Nejdrive zastav test prikazem: stop"); return; }
    nastavRele(true);
  }
  else if (strcmp(cmd, "closed") == 0 || strcmp(cmd, "zavreno") == 0) {
    if (autoTest) { Serial.println("Nejdrive zastav test prikazem: stop"); return; }
    nastavRele(false);
  }
  else if (strcmp(cmd, "test") == 0) {
    if (autoTest) { Serial.println("Test uz bezi!"); return; }
    spustAutoTest();
  }
  else if (strcmp(cmd, "stop") == 0) {
    if (!autoTest) { Serial.println("Test nebezi."); return; }
    zastavAutoTest(false);
  }
  else if (strcmp(cmd, "status") == 0) vypisStatus();
  else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) vypisNapovedu();
  else {
    Serial.print("Neznamy prikaz: "); Serial.println(cmd);
    vypisNapovedu();
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
  releSepnute = false;
  nastavRelePin(false);
  delay(50);
  nastavLedCervena();

  Serial.println();
  Serial.println("=== ESP32-S3 VENTIL TEST + AUTO ===");
  vypisNapovedu();
}

void loop() {
  // Zpracuj sériový vstup
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      inputBuffer[inputIndex] = '\0';
      if (inputIndex > 0) zpracujPrikaz(inputBuffer);
      inputIndex = 0;
    } else {
      if (inputIndex < sizeof(inputBuffer) - 1) {
        inputBuffer[inputIndex++] = c;
      }
    }
  }

  // Auto test logika (neblokující)
  zpracujAutoTest();

  delay(1);
}