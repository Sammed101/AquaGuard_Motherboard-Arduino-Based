/*
 * AquaGuard Motherboard Recreation
 * Controls: Water Pressure Switch, Water Level Switch, UV Light Relay, Solenoid Valve, Blue LED (Pressure Status) + Green LED (Water Level Status)  
 * Pin Definitions:
 * - Pin 2: Water Pressure Switch (INPUT_PULLUP) → SHORT = Pressure Present   //Your RO's logic might be opposite
 * - Pin 3: Water Level Switch   (INPUT_PULLUP) → SHORT = Low Water, OPEN = Full  //Your RO's logic might be opposite
 * - Pin 4: UV Light Relay       (OUTPUT)       → HIGH = UV ON
 * - Pin 5: Solenoid Valve       (OUTPUT)       → HIGH = Solenoid OPEN (water flows)
 * - Pin 6: Blue LED             (OUTPUT)       → Pressure Indicator  if BLINKING → Low Water Pressure  (no water supply) and if STATIC   → Water Pressure OK   (supply present)
 * - Pin 7: Green LED            (OUTPUT)       → Water Level Indicator if  BLINKING → Water Level LOW     (tank needs filling) and if  → Water Level FULL    (tank is full) 
 *
 * LED behavior is same as original Aquaguard controller.
 * RED LED (UV not working indicator) is skipped.
 * Add it if you want. * Logic Flow:
 * 1. Water Pressure OK + Water Level LOW → Start UV light (5 sec warm-up)
 * 2. After 5 sec UV warm-up → Open Solenoid (water starts flowing)
 * 3. If Pressure drops OR Tank Full → Turn OFF both UV + Solenoid
 */

// ──────────────────────────────────────────────
//  PIN CONFIGURATION
// ──────────────────────────────────────────────
const int PIN_PRESSURE_SWITCH = 2;   // Water pressure switch input
const int PIN_LEVEL_SWITCH    = 3;   // Water level switch input
const int PIN_UV_RELAY        = 4;   // UV light relay output
const int PIN_SOLENOID        = 5;   // Solenoid valve relay output
const int PIN_LED_BLUE        = 6;   // Blue LED  - Pressure status indicator
const int PIN_LED_GREEN       = 7;   // Green LED - Water level status indicator

// ──────────────────────────────────────────────
//  TIMING CONFIGURATION
// ──────────────────────────────────────────────
const unsigned long UV_WARMUP_MS  = 5000;  // 5 seconds UV warm-up before solenoid opens
const unsigned long LED_BLINK_MS  = 500;   // LED blink interval (ms) when status is LOW/BAD

// ──────────────────────────────────────────────
//  STATE MACHINE
// ──────────────────────────────────────────────
enum SystemState {
  STATE_IDLE,        // Everything OFF - no pressure or tank full
  STATE_UV_WARMUP,   // UV ON, Solenoid OFF - waiting 5 sec
  STATE_RUNNING,     // UV ON, Solenoid ON  - water purifying
  STATE_FAULT        // Something went wrong mid-cycle
};

SystemState currentState     = STATE_IDLE;
unsigned long uvStartTime    = 0;
SystemState  lastState       = STATE_FAULT; // Force a print on first loop

// ──────────────────────────────────────────────
//  LED BLINK TRACKING
// ──────────────────────────────────────────────
unsigned long lastBlueBlinkTime  = 0;
unsigned long lastGreenBlinkTime = 0;
bool          blueLedState       = false;
bool          greenLedState      = false;

// ──────────────────────────────────────────────
//  SETUP
// ──────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  Serial.println(F("==========================================="));
  Serial.println(F("  AquaGuard Controller - Initializing...  "));
  Serial.println(F("==========================================="));

  // Switch inputs with internal pull-up resistors
  // Switch SHORTED to GND → reads LOW
  // Switch OPEN           → reads HIGH (pulled up)
  pinMode(PIN_PRESSURE_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LEVEL_SWITCH,    INPUT_PULLUP);

  // Relay outputs - start OFF (LOW = relay OFF for active-low relay modules)
  pinMode(PIN_UV_RELAY,  OUTPUT);
  pinMode(PIN_SOLENOID,  OUTPUT);
  digitalWrite(PIN_UV_RELAY,  HIGH);  // UV OFF

  // LED outputs - start OFF
  pinMode(PIN_LED_BLUE,  OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  digitalWrite(PIN_LED_BLUE,  LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  Serial.println(F("[INIT] UV Relay      → OFF"));
  Serial.println(F("[INIT] Solenoid      → CLOSED"));
  Serial.println(F("[INIT] Blue  LED     → OFF (waiting for pressure read)"));
  Serial.println("[INIT] Green LED     → OFF (waiting for level read)");
  Serial.println(F("[INIT] System ready. Monitoring..."));
  Serial.println(F("-------------------------------------------"));
}

// ──────────────────────────────────────────────
//  HELPER: Read Switches (active LOW logic)
// ──────────────────────────────────────────────
bool isPressurePresent() {
  // Switch shorted to GND = LOW = Pressure IS there
  return digitalRead(PIN_PRESSURE_SWITCH) == LOW;
}

bool isWaterLevelLow() {
  // Switch shorted to GND = LOW = Water IS low
  return digitalRead(PIN_LEVEL_SWITCH) == LOW;
}

// ──────────────────────────────────────────────
//  HELPER: Relay Control (active-LOW relay modules)
// ──────────────────────────────────────────────
void setUV(bool on) {
  digitalWrite(PIN_UV_RELAY, on ? LOW : HIGH);
}

void setSolenoid(bool on) {
  digitalWrite(PIN_SOLENOID, on ? HIGH : LOW);
}

// ──────────────────────────────────────────────
//  HELPER: LED Updates (non-blocking blink)
//  Called every loop iteration
// ──────────────────────────────────────────────
void updateLEDs(bool pressureOK, bool levelLow) {
  unsigned long now = millis();

  // ── BLUE LED: Pressure Status ──
  if (pressureOK) {
    // Pressure HIGH → Static Blue ON
    digitalWrite(PIN_LED_BLUE, HIGH);
    blueLedState = true;
  } else {
    // Pressure LOW → Blink Blue
    if (now - lastBlueBlinkTime >= LED_BLINK_MS) {
      lastBlueBlinkTime = now;
      blueLedState = !blueLedState;
      digitalWrite(PIN_LED_BLUE, blueLedState ? HIGH : LOW);
    }
  }

  // ── GREEN LED: Water Level Status ──
  if (!levelLow) {
    // Tank FULL → Static Green ON
    digitalWrite(PIN_LED_GREEN, HIGH);
    greenLedState = true;
  } else {
    // Water LOW → Blink Green
    if (now - lastGreenBlinkTime >= LED_BLINK_MS) {
      lastGreenBlinkTime = now;
      greenLedState = !greenLedState;
      digitalWrite(PIN_LED_GREEN, greenLedState ? HIGH : LOW);
    }
  }
}

// ──────────────────────────────────────────────
//  HELPER: Print state only when it changes
// ──────────────────────────────────────────────
void printStateChange(SystemState newState) {
  if (newState == lastState) return;
  lastState = newState;

  Serial.println(F("-------------------------------------------"));
  switch (newState) {
    case STATE_IDLE:
      Serial.println(F("[STATE] → IDLE"));
      Serial.println(F("         UV=OFF | Solenoid=CLOSED"));
      break;
    case STATE_UV_WARMUP:
      Serial.println(F("[STATE] → UV WARM-UP (5 sec)"));
      Serial.println(F("         UV=ON  | Solenoid=CLOSED"));
      break;
    case STATE_RUNNING:
      Serial.println(F("[STATE] → RUNNING - Water Purifying"));
      Serial.println(F("         UV=ON  | Solenoid=OPEN"));
      break;
    case STATE_FAULT:
      Serial.println(F("[STATE] → FAULT - Interrupted mid-cycle"));
      Serial.println(F("         UV=OFF | Solenoid=CLOSED (Safety OFF)"));
      break;
  }
  Serial.println(F("-------------------------------------------"));
}

// ──────────────────────────────────────────────
//  MAIN LOOP
// ──────────────────────────────────────────────
void loop() {
  bool pressureOK  = isPressurePresent();
  bool levelLow    = isWaterLevelLow();

  // ── Always update LEDs every loop (non-blocking) ──
  updateLEDs(pressureOK, levelLow);

  // ── Debug: Print sensor + LED status every 2 sec ──
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint >= 2000) {
    lastDebugPrint = millis();

    Serial.print(F("[SENSORS] Pressure: "));
    Serial.print(pressureOK ? "HIGH (OK)       " : "LOW  (No Supply)");
    Serial.print(F(" | Blue  LED: "));
    Serial.println(pressureOK ? "STATIC  (Pressure OK)"  : "BLINKING (No Pressure)");

    Serial.print(F("[SENSORS] Level:    "));
    Serial.print(levelLow   ? "LOW  (Fill Needed)" : "FULL (Tank OK)  ");
    Serial.print(F(" | Green LED: "));
    Serial.println(levelLow   ? "BLINKING (Low Level)" : "STATIC  (Tank Full)");
  }

  // ════════════════════════════════════════════
  //  STATE MACHINE TRANSITIONS
  // ════════════════════════════════════════════
  switch (currentState) {

    // ─── IDLE: Waiting for conditions to be met ───
    case STATE_IDLE:
      setUV(false);
      setSolenoid(false);

      if (pressureOK && levelLow) {
        // Conditions met → start UV warm-up
        Serial.println(F("[TRIGGER] Pressure OK + Tank Low → Starting UV warm-up..."));
        uvStartTime  = millis();
        currentState = STATE_UV_WARMUP;
      }
      printStateChange(currentState);
      break;

    // ─── UV WARM-UP: UV on, solenoid still closed ───
    case STATE_UV_WARMUP:
      setUV(true);
      setSolenoid(false);

      // Safety check - if pressure drops or tank fills during warm-up → abort
      if (!pressureOK) {
        Serial.println(F("[ABORT] Pressure lost during UV warm-up! Aborting."));
        Serial.println(F("[LED]   Blue LED → BLINKING (pressure lost)"));
        currentState = STATE_FAULT;
        break;
      }
      if (!levelLow) {
        Serial.println(F("[ABORT] Tank filled during UV warm-up! Aborting."));
        Serial.println(F("[LED]   Green LED → STATIC (tank full)"));
        currentState = STATE_FAULT;
        break;
      }

      // Check if 5 seconds have passed
      if (millis() - uvStartTime >= UV_WARMUP_MS) {
        Serial.println(F("[UV] Warm-up complete! Opening Solenoid..."));
        currentState = STATE_RUNNING;
      } else {
        // Print countdown every second
        static unsigned long lastCountdown = 0;
        if (millis() - lastCountdown >= 1000) {
          lastCountdown = millis();
          int remaining = (int)((UV_WARMUP_MS - (millis() - uvStartTime)) / 1000) + 1;
          Serial.print(F("[UV WARMUP] "));
          Serial.print(remaining);
          Serial.println(F(" sec remaining..."));
        }
      }
      printStateChange(currentState);
      break;

    // ─── RUNNING: UV + Solenoid both ON ───
    case STATE_RUNNING:
      setUV(true);
      setSolenoid(true);

      // Stop if pressure drops
      if (!pressureOK) {
        Serial.println(F("[STOP] Water pressure lost! Closing solenoid & UV OFF."));
        Serial.println(F("[LED]  Blue LED → BLINKING (pressure lost mid-run)"));
        currentState = STATE_FAULT;
        break;
      }

      // Stop if tank is now full
      if (!levelLow) {
        Serial.println(F("[STOP] Tank is FULL! Closing solenoid & UV OFF."));
        Serial.println(F("[LED]  Green LED → STATIC (tank full - mission complete)"));
        currentState = STATE_IDLE;
        break;
      }
      printStateChange(currentState);
      break;

    // ─── FAULT: Safe shutdown, then return to IDLE ───
    case STATE_FAULT:
      setUV(false);
      setSolenoid(false);
      printStateChange(currentState);

      // Wait a moment then return to IDLE to re-evaluate
      delay(2000);
      Serial.println(F("[RECOVERY] Returning to IDLE state..."));
      currentState = STATE_IDLE;
      lastState    = STATE_FAULT; // Force reprint on next state
      break;
  }
}
