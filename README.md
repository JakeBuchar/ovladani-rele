# Ovládání relé – RO ventil (ESP32-S3)

Firmware pro ovládání solenoidového ventilu u reverzní osmózy (RO) přes ESP32-S3 DevKitC-1. Projekt nabízí dvě varianty nasazení:

- **ESPHome** – doporučená varianta pro integraci do Home Assistant
- **Arduino sketch** – záložní firmware pro ruční testování přes sériový monitor

## Hardware

| Komponenta | Pin | Popis |
|------------|-----|-------|
| Relé / ventil | GPIO 13 | `RELAY_ACTIVE_LOW = false` (aktivní HIGH) |
| Status LED (WS2812) | GPIO 48 | Vestavěná RGB LED na desce |
| Deska | ESP32-S3 DevKitC-1 | Varianta s USB CDC |

### Indikace LED

| Barva | Význam |
|-------|--------|
| Červená | Ventil zavřený (CLOSED) |
| Zelená | Ventil otevřený (OPEN) |

## Struktura repozitáře

```
ovladani-rele/
├── esphome/
│   ├── ro_ventil.yaml        # ESPHome konfigurace
│   └── secrets.yaml.example  # Šablona pro Wi-Fi a API klíč
├── ovladani_rele.ino         # Arduino záložní firmware
├── .gitignore
├── LICENSE
└── README.md
```

## ESPHome (doporučeno)

### Požadavky

- [ESPHome](https://esphome.io/) 2024.6.0 nebo novější
- Home Assistant (volitelné, ale typické použití)

### Instalace

1. Zkopírujte šablonu secrets:

   ```bash
   cp esphome/secrets.yaml.example esphome/secrets.yaml
   ```

2. Upravte `esphome/secrets.yaml` – doplňte Wi-Fi a API encryption key:

   ```yaml
   wifi_ssid: "TVOJE_WIFI_SSID"
   wifi_password: "TVOJE_WIFI_HESLO"
   api_encryption_key: "vygeneruj_base64_klic_nebo_z_esphome"
   ```

   Klíč vygenerujete příkazem `esphome secrets generate` nebo při prvním připojení zařízení.

3. Nahrajte firmware:

   ```bash
   esphome run esphome/ro_ventil.yaml
   ```

### Entity v Home Assistant

| Entity | Typ | Popis |
|--------|-----|-------|
| `switch.ventil_ro` | Switch | Otevření / zavření ventilu |
| `light.status_led` | Light | Stavová RGB LED |
| `binary_sensor.ventil_ro_otevren` | Binary sensor | Zrcadlí stav ventilu pro automatizace |

Při zapnutí ventilu se LED rozsvítí zeleně, při vypnutí červeně. Ventil se po restartu vždy vrátí do vypnutého stavu (`restore_mode: ALWAYS_OFF`).

## Arduino sketch (záložní / test)

Pro ruční testování bez Wi-Fi nahrajte `ovladani_rele.ino` do Arduino IDE.

### Závislosti

- Knihovna **Adafruit NeoPixel**

### Nastavení v Arduino IDE

- Deska: **ESP32S3 Dev Module**
- USB CDC On Boot: **Enabled** (pro sériový monitor)
- Baud rate: **115200**

### Sériové příkazy

| Příkaz | Alias | Akce |
|--------|-------|------|
| `open` | `otevreno` | Otevřít ventil |
| `closed` | `zavreno` | Zavřít ventil |
| `test` | — | Spustit 5min automatický test (přepínání každé 2 s) |
| `stop` | — | Zastavit automatický test |
| `status` | — | Zobrazit aktuální stav |
| `help` | `?` | Nápověda |

Automatický test běží 5 minut, přepíná ventil každé 2 sekundy a po skončení (nebo příkazem `stop`) ventil bezpečně zavře.

## Zapojení ventilu

```
ESP32-S3 GPIO 13 ──► IN relé modulu ──► COM/NO ──► Solenoid ventil (+12 V / GND dle modulu)
```

Před prvním zapnutím ověřte:

- Napájecí napětí ventilu odpovídá specifikaci
- Relé modul je dimenzovaný na zátěž ventilu
- Vodní vedení je správně utěsněné

## Bezpečnost

- Ventil ovládá přívod vody – při poruše může dojít k úniku.
- Po restartu zařízení je ventil vždy zavřený.
- Při automatickém testu nelze ventil ovládat ručně – nejdříve pošlete `stop`.
- Soubor `esphome/secrets.yaml` obsahuje citlivé údaje a **nesmí** být commitován do gitu.

## Licence

MIT – viz soubor [LICENSE](LICENSE).
