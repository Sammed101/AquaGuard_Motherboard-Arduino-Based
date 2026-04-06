# 💧 AquaGuard Motherboard — Arduino Recreation

An open-source recreation of the AquaGuard water purifier control board using an Arduino Uno. Controls UV sterilization, solenoid water intake, and monitors water pressure and tank level — all with non-blocking logic and serial debugging.

---

## 🔧 Features

- **Water pressure monitoring** via input switch (short = pressure present)
- **Water level monitoring** via float switch (short = tank full, open = tank empty)
- **UV light control** via relay — 5-second warm-up before water flows
- **Solenoid valve control** via relay — opens after UV warm-up completes
- **Blue LED** — static when pressure OK, blinks when no pressure
- **Green LED** — static when tank full, blinks when tank empty
- **Non-blocking blink logic** using `millis()` — never freezes the system
- **State machine architecture** — IDLE → UV_WARMUP → RUNNING → FAULT
- **Full serial debug output** with `F()` macro to save RAM
- **Fault recovery** — automatically resets after 2 seconds

---

## 📌 Pin Map

| Pin | Component | Direction | Notes |
|-----|-----------|-----------|-------|
| D2  | Water pressure switch | INPUT_PULLUP | Short to GND = pressure present |
| D3  | Water level switch | INPUT_PULLUP | Short to GND = tank full |
| D4  | UV light relay | OUTPUT | Active-LOW relay module |
| D5  | Solenoid valve relay | OUTPUT | Active-HIGH relay module |
| D6  | Blue LED | OUTPUT | 220Ω series resistor to GND |
| D7  | Green LED | OUTPUT | 220Ω series resistor to GND |
| 5V  | Relay module VCC | POWER | Both relay modules |
| GND | Common ground | POWER | All components |

---

## ⚡ Circuit Overview

```
+5V rail ────────────────────────────────────────────────
         │                                    │
    [Arduino Uno]                      [Relay VCC x2]
         │                                    │
    D2 ──┤── [Pressure SW] ── GND       D4 ──┤── UV Relay IN
    D3 ──┤── [Level SW]    ── GND       D5 ──┤── Solenoid Relay IN
    D6 ──┤── [220Ω] ── [Blue LED] ── GND
    D7 ──┤── [220Ω] ── [Green LED] ── GND
         │
GND rail ────────────────────────────────────────────────
```

> ⚠️ The solenoid coil and UV ballast are powered by their **AC supply** switched by relay COM/NO contacts. Never connect mains voltage to Arduino pins.

---

## 🔄 State Machine

```
         ┌─────────────────────────────────┐
         │                                 │
         ▼                                 │
      [IDLE]                               │
   UV OFF, Solenoid CLOSED                 │
         │                                 │
   Pressure OK + Level Low?                │
         │ YES         │
         ▼                                 │
   [UV_WARMUP] — 5 seconds                 │
   UV ON, Solenoid CLOSED                  │
         │                                 │
   5 sec elapsed?                          │
         │ YES                             │
         ▼                                 │
     [RUNNING]                             │
   UV ON, Solenoid OPEN                    │
         │                                 │
   Tank full OR pressure lost?             │
         │ YES                             │
         ▼                                 │
      [FAULT]  ── 2s cooldown ─────────────┘
   UV OFF, Solenoid CLOSED
```

---

## 🕐 Timing

| Event | Duration |
|-------|----------|
| Level low stabilizing delay | 5 seconds |
| UV warm-up before solenoid opens | 5 seconds |
| Fault recovery cooldown | 2 seconds |
| LED blink interval | 500 ms |

---

## 📟 Serial Debug Output

Open Serial Monitor at **9600 baud** to see real-time status:

```
===========================================
  AquaGuard Controller - Initializing...  
===========================================
[INIT] UV Relay      → OFF
[INIT] Solenoid      → CLOSED
[INIT] System ready. Monitoring...
-------------------------------------------
[SENSORS] Pressure: HIGH (OK)        | Blue  LED: STATIC  (Pressure OK)
[SENSORS] Level:    LOW  (Fill Needed)| Green LED: BLINKING (Low Level)
[WAIT] Level low - stabilizing 5 sec...
[STABILIZING] 5 sec...
[STABILIZING] 4 sec...
[STABILIZING] 3 sec...
[TRIGGER] Confirmed → Starting UV warm-up...
[STATE] → UV WARM-UP (5 sec)
[UV WARMUP] 5 sec remaining...
[UV] Warm-up complete! Opening Solenoid...
[STATE] → RUNNING - Water Purifying
[STOP] Tank is FULL! Closing solenoid & UV OFF.
[STATE] → IDLE
```

---

## 🛠️ Hardware Required

| Component | Qty | Notes |
|-----------|-----|-------|
| Arduino Uno | 1 | Or Nano with old bootloader |
| 5V Relay module | 2 | One active-LOW (UV), one active-HIGH (solenoid) |
| Water pressure switch | 1 | Normally open, closes when pressure present |
| Float level switch | 1 | Closes when tank full, opens when empty |
| Blue LED | 1 | 3mm or 5mm |
| Green LED | 1 | 3mm or 5mm |
| 220Ω resistor | 2 | One per LED |
| Jumper wires | — | — |
| 12V DC supply | 1 | For solenoid coil |
| UV ballast/choke | 1 | Ready-made, controlled via relay |

---

## 🚀 Getting Started

1. **Clone this repo**
   ```bash
   git clone https://github.com/yourusername/aquaguard-arduino.git
   ```

2. **Open in Arduino IDE**
   - File → Open → `aquaguard.ino`

3. **Select correct board**
   - Tools → Board → **Arduino Uno**
   - Tools → Port → your COM port
   - Tools → Programmer → **AVRISP mkII**

4. **Upload**
   - Click Upload (→)
   - Open Serial Monitor at 9600 baud

5. **Wire up** per the pin map above

---

## ⚙️ Configuration

At the top of `aquaguard.ino` you can adjust timing:

```cpp
const unsigned long UV_WARMUP_MS  = 5000;  // UV warm-up duration (ms)
const unsigned long LED_BLINK_MS  = 500;   // LED blink speed (ms)
// Stabilizing delay is also 5000ms — search "5000" in STATE_IDLE block
```

---

## 🔍 Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Upload fails with `openocd` error | Wrong board selected | Tools → Board → Arduino Uno |
| `Low memory` warning | Too many strings in RAM | Wrap all `Serial.println` strings in `F()` |
| Relay clicks on for 1-2 sec then off | Wrong relay polarity | Swap `LOW`/`HIGH` in `setSolenoid()` or `setUV()` |
| LED blinks when it should be static | Inverted switch logic | Flip `== LOW` to `== HIGH` in switch read function |
| Solenoid opens immediately, no UV delay | State machine skipping warmup | Check `uvStartTime` and `UV_WARMUP_MS` |
| False triggers on level switch | Switch bouncing | Increase stabilizing delay from 5000 to 8000ms |

---

## 📁 File Structure

```
aquaguard-arduino/
├── aquaguard.ino       # Main Arduino sketch
├── README.md           # This file
├── LICENSE             # MIT License
└── CHANGELOG.md        # Version history
```

---

## 🤝 Contributing

Pull requests welcome! If you've adapted this for a different purifier brand or added features like:
- LCD display support
- WiFi/IoT monitoring
- Filter life tracking
- Flow rate sensor

...feel free to open a PR.

---

## 📄 License

MIT License — free to use, modify, and distribute. See [LICENSE](LICENSE) for details.

---
