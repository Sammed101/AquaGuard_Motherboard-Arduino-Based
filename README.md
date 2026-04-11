# 💧 Aquaguard Motherboard — Arduino Based

Built this Arduino-based controller to replace a faulty Aquaguard motherboard. It uses pressure and water level sensors to control the UV light and solenoid with simple timing logic. If your RO system has a similar configuration and you’re comfortable working with hardware, this project is for you.

---

## 📸 Project Overview

<h3>🔌 Complete Setup</h3>
<p align="center">
  <img src="Images/Mother%20Board%20connections%20Diagram.png" width="600"/>
</p>

### ⚡ UV Ballast (Choke)

<p align="center">
  <img src="Images/UV%20Light%20Choke%2011w.png" width="600"/>
</p>

<br>

🔗Link: [Philips UV Ballast (11W–14W)](https://www.amazon.in/Philips-11W-14W-Ballast-Light-Filter/dp/B01MR89ZJO)

<h3>📊 Original Wiring Reference</h3>
<p align="center">
  <img src="Images/Water%20Flow%20Diagram%20%26%20Wiring%20Diagram.jpeg" width="600"/>
</p>

<h3>🏷️ Original Controller Specs</h3>
<p align="center">
  <img src="Images/Mother_Board_Name%26Features.JPEG" width="600"/>
</p>
---

## 🔧 Features

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

## 🔄 State Flow
IDLE → UV_WARMUP → RUNNING → IDLE

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

- Arduino Uno / Nano 
- 2x Relay modules
- Pressure switch
- Float level switch
- UV ballast (11W)
- Solenoid valve
- LEDs + resistors
- Power supply

---

## 🚀 Getting Started

1. Upload `aquaguard.ino`
2. Connect sensors + relays
3. Power the system
4. Monitor via Serial (9600 baud)

---

## 💡 Notes

- LED behavior matches Exactly with the original Aquaguard controller  
- RED LED (UV fault indicator) is not implemented (optional)

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
