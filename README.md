# Aquaguard Motherboard — Arduino Based

Built this Arduino-based controller to replace a faulty Aquaguard motherboard. It uses pressure and water level sensors to control the UV light and solenoid with simple timing logic. If your RO system has a similar configuration and you’re comfortable working with hardware, this project is for you.

---

## 📸 Project Overview

<h3> Complete Setup</h3>
<p align="center">
  <img src="Images/Mother%20Board%20connections%20Diagram.png" width="600"/>
</p>

### UV Ballast (Choke)

<p align="center">
  <img src="Images/UV%20Light%20Choke%2011w.png" width="600"/>
</p>

<br>

🔗Link: [Philips UV Ballast (11W–14W)](https://www.amazon.in/Philips-11W-14W-Ballast-Light-Filter/dp/B01MR89ZJO)

<h3> Original Wiring Reference</h3>
<p align="center">
  <img src="Images/Water%20Flow%20Diagram%20%26%20Wiring%20Diagram.jpeg" width="600"/>
</p>

<h3> Original Controller Specs</h3>
<p align="center">
  <img src="Images/Mother_Board_Name%26Features.JPEG" width="600"/>
</p>
---

##  Features

- Water pressure detection (input switch)
- Tank water level detection (float switch)
- UV light control via relay
- Solenoid valve control via relay
- UV warm-up delay (~5 seconds)
- Non-blocking logic using `millis()`
- Serial debugging support
- LED indicators (same behavior as original controller)

---

## ⚙️ How It Works

### Logic:

- If **pressure is present** AND **tank is low**:
  - Turn ON UV
  - Wait ~5 seconds
  - Turn ON solenoid (start filling)

- If:
  - Tank becomes full OR  
  - Pressure is lost  

👉 System turns OFF UV + solenoid

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
         │ YES (after 5s stabilize)        │
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

## 📌 Pin Map

| Pin | Component              |
|-----|----------------------|
| D2  | Pressure switch       |
| D3  | Level switch          |
| D4  | UV relay              |
| D5  | Solenoid relay        |
| D6  | Blue LED              |
| D7  | Green LED             |

---

## ⚡ Wiring Notes

- Switches use `INPUT_PULLUP`
- Short to GND = ACTIVE
- Relays handle AC and DC load R1 for AC for UV Choke and R2 to the 12 v to the solenoid
- Arduino only controls relays (low voltage)

---

## ⚠️ Safety

- UV ballast runs on **220V AC**
- Never connect AC directly to Arduino
- Use proper insulation and enclosure
- Double-check relay wiring

---

## 🛠️ Hardware Required

| Component | Qty | Notes |
|-----------|-----|-------|
| Arduino Uno | 1 | Or nano |
| 5V Relay module | 2 | One (UV), one (solenoid) |
| Water pressure switch | 1 | Normally open, closes when pressure present |
| Float level switch | 1 | Closes when tank Empty, opens when Full |
| Blue LED | 1 | 3mm or 5mm |
| Green LED | 1 | 3mm or 5mm |
| 220Ω resistor | 2 | Additional |
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

## 💡 Notes

- LED behavior matches Exactly with the original Aquaguard controller  
- RED LED (UV fault indicator) is not implemented (optional)

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

## 🤝 Contributing

Pull requests welcome! If you've adapted this for a different purifier brand or added features like:
- LCD display support
- WiFi/IoT monitoring
- Filter life tracking
- Flow rate sensor

...feel free to open a PR.

---

## 📄 License

Aapache License — free to use, modify, and distribute. See [LICENSE](LICENSE) for details.

---
