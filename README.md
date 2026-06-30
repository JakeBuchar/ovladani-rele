# Relay Control – RO Valve (ESP32-S3)

Firmware for controlling a solenoid valve on a reverse osmosis (RO) system via ESP32-S3 DevKitC-1. The project offers two deployment options:

- **ESPHome** – recommended for Home Assistant integration
- **Arduino sketch** – fallback firmware for manual testing over the serial monitor

## Hardware

| Component | Pin | Description |
|-----------|-----|-------------|
| Relay / valve | GPIO 13 | `RELAY_ACTIVE_LOW = false` (active HIGH) |
| Status LED (WS2812) | GPIO 48 | On-board RGB LED |
| Board | ESP32-S3 DevKitC-1 | USB CDC variant |

### LED Indication

| Color | Meaning |
|-------|---------|
| Red | Valve closed (CLOSED) |
| Green | Valve open (OPEN) |

## Repository Structure

```
ovladani-rele/
├── esphome/
│   ├── ro_ventil.yaml        # ESPHome configuration
│   └── secrets.yaml.example  # Template for Wi-Fi and API key
├── ovladani_rele.ino         # Arduino fallback firmware
├── .gitignore
├── LICENSE
└── README.md
```

## ESPHome (recommended)

### Requirements

- [ESPHome](https://esphome.io/) 2024.6.0 or newer
- Home Assistant (optional, but typical use case)

### Installation

1. Copy the secrets template:

   ```bash
   cp esphome/secrets.yaml.example esphome/secrets.yaml
   ```

2. Edit `esphome/secrets.yaml` – fill in Wi-Fi credentials and API encryption key:

   ```yaml
   wifi_ssid: "YOUR_WIFI_SSID"
   wifi_password: "YOUR_WIFI_PASSWORD"
   api_encryption_key: "generate_base64_key_or_from_esphome"
   ```

   Generate the key with `esphome secrets generate` or when pairing the device for the first time.

3. Flash the firmware:

   ```bash
   esphome run esphome/ro_ventil.yaml
   ```

### Home Assistant Entities

| Entity | Type | Description |
|--------|------|-------------|
| `switch.ventil_ro` | Switch | Open / close the valve |
| `light.status_led` | Light | Status RGB LED |
| `binary_sensor.ventil_ro_otevren` | Binary sensor | Mirrors valve state for automations |

When the valve is turned on, the LED turns green; when off, red. After a restart, the valve always returns to the off state (`restore_mode: ALWAYS_OFF`).

## Arduino Sketch (fallback / testing)

For manual testing without Wi-Fi, upload `ovladani_rele.ino` in the Arduino IDE.

### Dependencies

- **Adafruit NeoPixel** library

### Arduino IDE Settings

- Board: **ESP32S3 Dev Module**
- USB CDC On Boot: **Enabled** (for serial monitor)
- Baud rate: **115200**

### Serial Commands

| Command | Action |
|---------|--------|
| `open` | Open the valve |
| `closed` | Close the valve |
| `test` | Start 5-minute automatic test (toggle every 2 s) |
| `stop` | Stop the automatic test |
| `status` | Show current state |
| `help` / `?` | Help |

The automatic test runs for 5 minutes, toggling the valve every 2 seconds. When it finishes (or after `stop`), the valve closes safely.

## Valve Wiring

```
ESP32-S3 GPIO 13 ──► Relay module IN ──► COM/NO ──► Solenoid valve (+12 V / GND per module)
```

Before first use, verify:

- Valve supply voltage matches the specification
- Relay module is rated for the valve load
- Water lines are properly sealed

## Safety

- The valve controls the water supply – a fault may cause a leak.
- After a device restart, the valve is always closed.
- Manual control is disabled during the automatic test – send `stop` first.
- The file `esphome/secrets.yaml` contains sensitive data and **must not** be committed to git.

## License

MIT – see [LICENSE](LICENSE).
