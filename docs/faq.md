# ESP Rowing Monitor — Community FAQ (Enriched for LLM use)

This FAQ consolidates community knowledge from GitHub discussions into dense, LLM-friendly answers. It complements the [official ESP Rowing Monitor documentation](./README.md) by capturing real-world user experiences, troubleshooting workflows, and hardware-specific insights that emerge from community discussions.

## How to Use This FAQ

This document works best alongside the official documentation:

- **For first-time setup:** Start with the [Installation Guide](./installation.md) and [README](../README.md), then refer to this FAQ for specific questions
- **For troubleshooting:** Use this FAQ to find community-validated solutions and diagnostic workflows
- **For hardware selection:** Consult FAQ entries #2-#7 for sensor and hardware specifics
- **For configuration:** Reference [Settings Guide](./settings.md) for parameter explanations, and this FAQ for tuning guidance based on community experience

Each FAQ entry contains:

- **Short answer:** Quick solution or guidance
- **What / Why / How:** Contextual explanation and step-by-step approach
- **Community-proven specifics:** Real part numbers, config snippets, and examples from working setups
- **Failure modes:** Common mistakes and how to avoid them
- **Paraphrases:** Alternative phrasings to help with search/query matching
- **Tags & References:** Links to official docs and relevant GitHub discussions

Note: This document is authored for machines first and humans second — answers are deliberately contextual and include examples, failure modes, and recommended next steps.

---

## 1) What is the best way to start with ESP Rowing Monitor (first-time setup)?

**Short answer:** Assess your current hardware setup, determine what changes are needed for ESP Rowing Monitor compatibility, flash the firmware, then follow the calibration documentation.

What / Why / How

- What: Before installing, understand what hardware you have and what needs to be added or modified for ESP Rowing Monitor to work.
- Why: ESP Rowing Monitor requires rotational impulse detection from the flywheel — most rowers need additional sensors. Knowing your hardware situation upfront prevents troubleshooting later.
- How (assessment checklist):
  1. **Identify your rower model** — Check if your model is already documented or similar to supported configurations
  2. **Determine sensor compatibility** — Inspect existing sensors; understand if they're compatible or if you need to add new ones (see [Entry #3](#3-should-i-use-a-hall-sensor-or-a-reed-switch) and [Entry #6](#6-which-hall-sensor-modules-are-known-to-work))
  3. **Plan sensor installation** — If no compatible sensor exists, you'll need to add a magnet + hall/reed sensor; decide on magnet count (see [Entry #7](#7-how-many-magnets-should-i-use-and-where-should-they-be-placed))
  4. **Choose your ESP32 board** — FireBeetle 2 ESP32-E, Lolin S3 Mini, ESP32 DevKit, or other supported boards (see [Entry #11](#11-which-esp32-boards-are-supported))
  5. **Flash firmware** — Use precompiled binaries from releases or compile with PlatformIO. See official [Installation Guide](./installation.md)
  6. **Test sensor pulses** — Rotate flywheel manually and verify impulses in the WebGUI or serial logs before full configuration
  7. **Configure rower profile** — Set `IMPULSES_PER_REVOLUTION` and tune other parameters (see [Settings Guide](./settings.md))

Community experience

- New users often solved their first blockers by ensuring the flywheel produced clean impulses before trying tuning.
- Recommended: Record deltaTime data first using the WebGUI, this helps with debugging and parameter tuning later.
- Use the `dynamic` rower profile initially as it allows runtime settings adjustment via the WebGUI.

Debug commands (ESP32):

```bash
# Connect via serial to see logs
platformio device monitor --baud 1500000

# Or use online serial terminal:
# https://googlechromelabs.github.io/serial-terminal/
```

Paraphrases

- "How do I get started with ESP Rowing Monitor?"
- "First steps to run ESP Rowing Monitor on my ESP32"
- "What hardware do I need for ESP Rowing Monitor?"

Tags: getting-started, hardware-assessment, installation

References:

- Official documentation: [Installation Guide](./installation.md), [Settings Guide](./settings.md)
- [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/)

---

## 2) How do I check if my rower is compatible?

**Short answer:** Any rower with a rotating flywheel or shaft is potentially compatible — you just need a way to detect that rotation reliably.

What / Why / How

- What: ESP Rowing Monitor calculates power and strokes from rotational dynamics. Compatibility means the ability to measure rotational impulses from the flywheel or drive shaft.
- Why: The fundamental requirement is consistent, measurable rotation — the specific mechanism (air, water, magnetic resistance) doesn't matter for compatibility.
- How:
  1. **Check for existing sensor** — Open your rower housing and look for magnets or sensors near the flywheel/shaft
  2. **Verify rotation detection** — If a sensor exists, determine if it produces digital pulses (compatible) or requires signal conditioning
  3. **Add sensor if needed** — If no sensor exists, add a magnet to the flywheel and a hall sensor nearby (see [Entry #6](#6-which-hall-sensor-modules-are-known-to-work))
  4. **Test pulse detection** — Manually spin the flywheel and verify pulses in debug logs

General principle: **Every rower is compatible if it has a rotating flywheel or shaft where rotation can be detected.** The challenge is adding the appropriate sensor if one doesn't exist.

Example of compatible rower types confirmed by community (both Open Rowing Monitor and ESP Rowing Monitor):

- **Air rowers:** Generic Chinese clones, Concept2 (with sensor modification), ForceUSA R3, DKN R-320
- **Water rowers:** WaterRower S4, Oartec Slider, CRIVIT (Lidl), Sportstech WRX700
- **Magnetic rowers:** Kettler Stroker, NordicTrack RX800
- **Kayak ergometers:** KayakFirst (Orange and Blue), Old Danube

Community tips

- If unsure, try opening the rower housing and look for existing magnets and sensors
- ESP Rowing Monitor shares core algorithms with OpenRowingMonitor, so rowers working with ORM will work with ESP Rowing Monitor

Paraphrases

- "Will ESP Rowing Monitor work on my [brand/model]?"
- "How to check compatibility for my rowing machine"
- "Is every rower compatible with ESP Rowing Monitor?"

Tags: compatibility, hardware, getting-started

References:

- Official documentation: [Settings Guide](./settings.md), [README](../README.md)
- GitHub discussions: [Does this work with WATERROWER?](https://github.com/Abasz/ESPRowingMonitor/discussions/17)

---

## 3) Should I use a Hall sensor or a Reed switch?

**Short answer:** Use a **Hall effect sensor** if possible; reed switches work but are inferior due to mechanical bounce issues.

What / Why / How

- What: Hall sensors detect magnetic fields electronically and yield clean digital impulses. Reeds are mechanical contact closures.
- Why: Hall sensors do not suffer contact bounce and are more tolerant to alignment issues; they produce more stable timing and cleaner power estimates.
- How: Mount a small magnet on the flywheel and position the hall sensor so it trips once per magnet pass. Configure `IMPULSES_PER_REVOLUTION` if more than one magnet is present. Bear in mind that **ESPRM uses internal pull-up and falling edge** for impulse detection.

Community-proven specifics

- See [Entry #6](#6-which-hall-sensor-modules-are-known-to-work) for confirmed working hall sensors
- Hall sensors should be < 1mm from magnets for reliable detection
- All magnets should face with the same polarity towards the hall (south or north)

Failure modes

- Wrong polarity (in case of unipolar or latching sensor), wiring issues → no pulses detected.
- Hall module designed for 5V used directly on 3.3V MCU → unreliable readings or no detection.
- **Magnetic polarity issue:** Alternating N/S magnets can cause measurement inconsistencies because some hall sensors are biased toward N/S detection. Check magnet orientation consistency if multiple magnets are present.
- Vibration from rower on hard floor when using reed → spurious pulses. Solution: place rower on soft foam mat.

Reed switch considerations

- If using reed switches, watch [this video on reed switch mechanics](https://www.youtube.com/watch?v=7LimjYS04FQ&t=278s) to understand bounce
- Orient the reed **perpendicular** to magnet travel for best results
- As a last resort use `ENABLE_DEBOUNCE_FILTER true` in settings if experiencing sever bounce issues (read the docs!)
- Reed switches can work for both wakeup and impulse detection with the same sensor

Paraphrases

- "Should I use hall or reed sensors?"
- "Which sensor is less noisy for ESP Rowing Monitor?"
- "Best sensor to detect flywheel rotation?"

Tags: sensors, wiring, hardware, configuration, hall-effect

References:

- Official documentation: [Settings Guide](./settings.md)
- External resources: [Reed Switch Mechanics (YouTube)](https://www.youtube.com/watch?v=7LimjYS04FQ&t=278s)
- GitHub discussions: [Wakeup sensor wiring](https://github.com/Abasz/ESPRowingMonitor/discussions/13), [Stroke detection works but no distance](https://github.com/Abasz/ESPRowingMonitor/discussions/15)

---

## 4) My rower already has reed switches on the handle — can I reuse them for ESP Rowing Monitor?

**Short answer:** Not directly — handle reed switches measure linear handle travel and usually don't provide the rotational timing ESP Rowing Monitor needs.

What / Why / How

- What: Handle-mounted reeds detect translation/position, not flywheel rotation.
- Why: ESP Rowing Monitor models power and strokes from rotational dynamics; using handle sensors often yields inconsistent timing and incorrect power calculations.
- How: Keep the handle sensor for the rower's stock electronics if needed, but add a dedicated flywheel magnet + hall sensor for ESP Rowing Monitor. See [Entry #6](#6-which-hall-sensor-modules-are-known-to-work) for sensor recommendations.

Community experience

- Multiple users tried reuse; successful conversions ultimately added a new flywheel sensor and ignored the handle switch for ESP Rowing Monitor.
- Original sensing mechanisms (like dual reed switches for direction detection) are generally not suitable for ESP Rowing Monitor's requirements

Paraphrases

- "Can I use my handle reed switch for ESP Rowing Monitor?"
- "Do handle sensors give correct strokes for ESP Rowing Monitor?"

Tags: sensors, hardware, compatibility

References:

- Official documentation: [Settings Guide](./settings.md)
- GitHub discussions: [CRIVIT water rower](https://github.com/Abasz/ESPRowingMonitor/discussions/22)

---

## 5) Reed switches bounce — can I reduce bounce reliably?

**Short answer:** You can reduce bounce mechanically and with software debounce, but it often cannot reach the reliability of Hall sensors.

What / Why / How

- What: Bounce is mechanical contact chatter when the reed closes. Watch this [excellent explanation of how reed switches work](https://www.youtube.com/watch?v=7LimjYS04FQ&t=278s) to understand the mechanical bounce phenomenon.
- Why: Bounce causes double counts, false high cadence, or power spikes when signals are interpreted as clean impulses.
- How: Improve mechanical mounting (rigid bracket), orient magnet face perpendicular to the switch, increase spacing slightly, and use software debounce (as last resort only). If issues persist, replace with Hall sensor (see [Entry #6](#6-which-hall-sensor-modules-are-known-to-work)).

Software debounce in ESP Rowing Monitor:

Enable in your settings file:

```cpp
#define ENABLE_DEBOUNCE_FILTER true
```

This filter rejects impulses that are highly inconsistent with the previous measured interval.

Community-proven techniques

- Rigid mounts and careful magnet spacing reduce, but do not eliminate, bounce
- Quote from user who switched to Hall: *"the noise seems to disappear even with no debounce"* — Hall sensors fundamentally eliminate the bounce problem
- Reed positioning matters: signal settled in ~200µs with proper placement (scope-verified by community member)
- Check datasheet for max switching frequency (300-500Hz minimum recommended)

Failure modes

- Flexible mounting bracket amplifies mechanical resonance → persistent double-counts
- Too-close magnet → continuous trigger instead of discrete pulses
- Distance between reed and magnets too large → unreliable triggering

Paraphrases

- "How to stop reed switches double-counting?"
- "Reed switch debouncing tips for ESP Rowing Monitor"

Tags: sensors, troubleshooting, reed-switches

References:

- Official documentation: [Settings Guide - ENABLE_DEBOUNCE_FILTER](./settings.md)
- External resources: [Reed Switch Mechanics (YouTube)](https://www.youtube.com/watch?v=7LimjYS04FQ&t=278s)
- GitHub discussions: [Stroke detection works but no distance](https://github.com/Abasz/ESPRowingMonitor/discussions/15), [CRIVIT water rower](https://github.com/Abasz/ESPRowingMonitor/discussions/22), [ORM noise reduction discussion #122](https://github.com/laberning/openrowingmonitor/discussions/122)

---

## 6) Which Hall sensor modules are known to work?

**Short answer:** DRV5023, KY-024, A3144, SS443A and similar digital hall sensors work well; pick components rated for your system voltage (3.3V preferred for ESP32), but if you use the 5v rail you can power the hall from there (keep in mind that halls use 2-5ma so if battery operation is planned you need to turn power off to the hall).

What / Why / How

- What: Small digital hall sensors produce clean digital outputs suitable for GPIO input.
- Why: They are inexpensive, widely available, and eliminate mechanical bounce issues.
- How: Confirm the module operates at 3.3V (or use 5V rail if ESP32 is powered trhough that), wire Vcc, GND and output (internal pull-up is enabled automatically), and test pulses by rotating the flywheel.

Community-proven specifics

| Sensor | Voltage | Notes | Community Reports |
| --- | --- | --- | --- |
| **DRV5023** | 2.5V-5.5V | Best option for low-power 3.3V systems. Works reliably with ESP32 without level shifting. | Confirmed working |
| **KY-003** | 4.5V | Popular module, clean signal. | Multiple confirmed working |
| **KY-024** | 3.3V-5V | Linear hall with voltage comparator. Less reliable than digital halls due to adjustable trigger point. May not achieve full functionality at 3.3V. | Works but not recommended |
| **A3144** | 4.5V-24V | Requires 5V supply minimum. Cannot be powered from ESP32 3.3V directly only from the 5v rail. | Requires voltage consideration |
| **SS443A** | 3.8V+ | Honeywell sensor, rated at 3.8V. Works from LiPo battery. Avoid "power efficient" versions (sleep mode doesn't work). | Confirmed working |
| **Allegro A1120** | 3.8V-24V | Used in Oartec Slider. Works at 3.3V | Confirmed working |

Wiring recommendations:

- **ESP32 GPIO:** Connect sensor output to configured sensor pin (default varies by board profile).

Failure modes

- Using a 5V-only sensor on a 3.3V MCU can cause unreliable reads.
- Polarity-sensitive: ensure magnet is oriented with correct pole toward sensor.
- Distance too large: Hall sensor should be < 1mm from magnets for reliable detection.

Paraphrases

- "Which hall sensor part number should I buy?"
- "What hall sensors do people use for ESP Rowing Monitor?"
- "Best hall sensor for 3.3V ESP32"

Tags: sensors, hardware, hall-effect, specifications

References:

- Official documentation: [Settings Guide](./settings.md), [Installation Guide](./installation.md)
- GitHub discussions: [Wakeup sensor wiring](https://github.com/Abasz/ESPRowingMonitor/discussions/13), [Oartec Slider](https://github.com/Abasz/ESPRowingMonitor/discussions/19)

---

## 7) How many magnets should I use and where should they be placed?

**Short answer:** Start with **1-4 magnets** for simplest setup; advanced users can use 6 magnets for higher resolution, but placement precision becomes more important. Machines that produce less rotation per stroke (typically water rowers and magnetically braked rowers) benefit more from additional magnets. Note, firmware has the ability to correct systeamtic errors - bias - to some extent, yet hardware precision is important.

What / Why / How

- What: Magnets produce discrete pulses; count and configure impulses per revolution to avoid miscalculated cadence.
- Why: ESP Rowing Monitor converts impulses into rotational speed — extra impulses per revolution will multiply the resolution of the power estimates unless configured correctly. More magnets = higher timing resolution but more complexity.
- How:
  - **1-2 magnets (beginner):** Easiest to set up, minimal placement issues. Place anywhere on the flywheel where they can be stably mounted.
  - **3-4 magnets (recommended):** Good balance of resolution and complexity. Requires even spacing (90° or 120° apart). Good for most rowers.
  - **6 magnets (advanced):** Maximum resolution, but placement precision is critical — uneven spacing causes timing jitter. A 3D printed bracket helps with placement.

Community experience

- Uneven magnet polarity caused timing errors — ensure all magnets face the same direction (all north or all south toward sensor)
- More magnets = higher resolution timing, but also more complexity in configuration and higher risk of noise/error amplification
- Start with 2-4 magnets. If it works well, you can experiment with 6 for nicer force curves
- Version 7.0.0 includes cyclic error filter that corrects for magnet misalignment

Example configuration:

```cpp
// In your rower profile:
#define IMPULSES_PER_REVOLUTION 4  // Match this to your magnet count
#define IMPULSE_DATA_ARRAY_LENGTH 7  // Generally 1.5-2x magnets, adjust as needed
```

Failure modes

- `IMPULSES_PER_REVOLUTION` wrong → cadence shows 2x, 3x, etc. of actual rate
- Inconsistent magnet spacing → timing jitter, unstable power readings
- Alternating polarity (N/S/N/S) → detection bias with some sensors

Paraphrases

- "Do I need more than one magnet?"
- "How to set IMPULSES_PER_REVOLUTION"
- "Should I use 1, 3, or 4 magnets on my flywheel?"

Tags: sensors, configuration, magnets, setup

References:

- Official documentation: [Settings Guide](./settings.md)
- GitHub discussions: [Kettler Stroker](https://github.com/Abasz/ESPRowingMonitor/discussions/20), [CRIVIT water rower](https://github.com/Abasz/ESPRowingMonitor/discussions/22)

---

## 8) Where to measure sprocket radius and why it matters?

**Short answer:** Measure the effective sprocket radius at the point where the chain/belt contacts the pulley, not the outer edge of any cover.

What / Why / How

- What: `SPROCKET_RADIUS` affects translation between rotations and handle travel; erroneous measurement introduces scaling errors in handle forces array but does not affect stroke detectoin or metrics calculation (it is purely a presentation matter).
- Why: Using the wrong radius (e.g. measuring rim instead of contact point) causes consistent over- or under-estimation of handle force.
- How: Remove covers if necessary and measure the radius to the point where the belt/chain contacts the pulley;

Measurement tips:

- The sprocket radius is the radius of the wheel rolling up the belt/chain
- For pulleys with teeth, measure to the pitch diameter (center of teeth)
- Units should be in centimeters

Paraphrases

- "How do I measure sprocket radius?"
- "Why is my distance off — is the sprocket radius wrong?"

Tags: configuration, calibration

References:

- Official documentation: [Settings Guide - SPROCKET_RADIUS](./settings.md)
- External resources: [Sprocket pitch diameter calculator](https://www.engineersedge.com/calculators/sprocket_pitch_diameter_15579.htm)

---

## 9) How do I calibrate ESP Rowing Monitor?

**Short answer:** Use the recommended calibration workflow: configure basic parameters, row test sessions, export deltaTime data, analyze with simulation tools, and iteratively adjust parameters until metrics are accurate.

What / Why / How

- What: Calibration tunes ESP Rowing Monitor parameters to match your rower's physical characteristics for accurate metrics.
- Why: Different flywheels have different inertia, friction, and gearing — calibration compensates for these machine-specific characteristics.
- How: Set initial estimates → row test session → export deltaTime data → analyze with simulation tool → adjust parameters → repeat until satisfied.

Key parameters for calibration:

- `IMPULSES_PER_REVOLUTION` — **must be correct first** (see [Entry #7](#7-how-many-magnets-should-i-use-and-where-should-they-be-placed))
- `IMPULSE_DATA_ARRAY_LENGTH` — samples used for regression calculation, typically 1.5-2× your magnet count. Critical for accurate angular velocity calculation. Setting this too high can lead to skipped strokes while too low can result in ghost strokes.
- `FLYWHEEL_INERTIA` — rotational inertia of flywheel (in kg\*m²). Affects calculated dragFactor and handle force/power along with distance, speed etc. **This is an input setting, not calculated.** See [Entry #10](#10-how-do-i-measure-flywheel-inertia) for measurement methods.
- `MINIMUM_DRAG_TORQUE` — torque threshold below which recovery is triggered. **This is the most important tuning parameter for stroke detection.** Higher values are more conservative (trigger recovery earlier) but setting it too high can include drive force in drag calculation, poisoning dragFactor and all derived metrics.
- `DRIVE_HANDLE_FORCES_MAX_CAPACITY` — failsafe maximum number of impulses per drive phase. Should be based on how many datapoints a drive can realistically produce for your setup (magnet count × typical number of flywheel rotations per drive).
- `GOODNESS_OF_FIT_THRESHOLD` — minimum "Goodness of Fit" (0.0-1.0) required to accept calculated dragFactor (necessary for power and distance). Water rowers and magnetic rowers may need lower values (0.6-0.8).

**Important:** Even with perfect calibration, incorrect stroke detection parameters can cause issues. If metrics seem wrong or strokes are being missed, review stroke detection first (see [Entry #15](#15-strokes-are-getting-lost-or-stroke-detection-needs-tuning)).

Calibration workflow:

1. **Set `IMPULSES_PER_REVOLUTION`** — count your magnets, this must be correct
2. **Set `IMPULSE_DATA_ARRAY_LENGTH`** — typically 1.5-2× magnet count (e.g., 6-7 for 4 magnets)
3. **Estimate `FLYWHEEL_INERTIA`** — use similar machines, CAD model, or measurement (see [Entry #10](#10-how-do-i-measure-flywheel-inertia))
4. **Row test session** and export deltaTime data from WebGUI
5. **Run e2e simulation tool** to analyze metrics
6. **Tune stroke detection parameters** if strokes are being missed or incorrectly detected:
   - Adjust `MINIMUM_DRAG_TORQUE` (see parameter explanation above)
   - Adjust `DRIVE_HANDLE_FORCES_MAX_CAPACITY` based on expected drive datapoints
   - etc.
7. View output in Calibration Helper GUI tool
8. Repeat until satisfied

Calibration without simulation by using the `dynamic` rower profile:

1. Flash firmware with dynamic profile
2. Connect to [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/)
3. Go to Settings
4. Adjust parameters in real-time
5. Changes take effect immediately (though devices will restart) and persist

This allows iteration without recompilation of the firmware if you are not able to use e2e simulation tool.

Using the e2e simulation tool:

```bash
# Setup project (Linux/WSL)
cmake -S . -B build

# Build e2e target
./build/test/e2e/build-e2e [ENVIRONMENT NAME]

# Run simulation
./build/test/e2e/e2e_test.out [deltaTimes.txt] > OUTPUT
```

Paraphrases

- "How to calibrate ESP Rowing Monitor?"
- "What parameters do I need to set?"
- "How to tune ESP Rowing Monitor settings?"

Tags: calibration, configuration, parameters

References:

- Official documentation: [Settings Guide](./settings.md)
- [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/)
- GitHub discussions: [How to use e2e calibration tool](https://github.com/Abasz/ESPRowingMonitor/discussions/18), [Kettler Stroker](https://github.com/Abasz/ESPRowingMonitor/discussions/20)

---

## 10) How do I measure flywheel inertia?

**Short answer:** Measure flywheel inertia using the swing period method, CAD modeling, [estimate per component](https://dvernooy.github.io/projects/ergware/) or estimate from similar machines. Typical values range from 0.03 to 0.1 kg\*m².

What / Why / How

- What: Flywheel inertia (`FLYWHEEL_INERTIA`) is the rotational mass of the flywheel in kg\*m². It's a physical property of your rower that must be measured or estimated.
- Why: Inertia directly affects calculated power, distance, and drag factor. Wrong inertia = wrong metrics.
- How: Use one of the measurement methods below, then validate by comparing calculated metrics to expected performance.

### Measurement methods

#### Method 1: CAD modeling (Most accurate)

If you have CAD software and can model the flywheel geometry:

1. Model the flywheel as accurately as possible
2. Weigh each component separately and take a note
3. Assign material density so the component match the the measured weight
4. Use CAD software to calculate moment of inertia about the rotation axis

#### Method 2: Component-by-component calculation

Calculate inertia by breaking down the flywheel into individual components (disc, spokes, dampers, etc.) and summing their contributions. This approach is detailed in [Dave Vernooy's ErgWare project](https://dvernooy.github.io/projects/ergware/):

1. Identify each component (main disc, fan blades/dampers, hub, etc.)
2. Calculate each component's contribution using standard moment of inertia formulas
3. Sum all contributions: $J_{total} = J_{disc} + J_{dampers} + J_{hub} + ...$

This method is practical when you can measure or estimate the mass and geometry of each component. Accuracy is typically within 10-15%.

#### Method 3: Swing period method

1. Remove flywheel from rower (or access it)
2. Suspend flywheel from its center axis so it can swing freely
3. Displace slightly and time the swing period (time for one complete oscillation)
4. Calculate: $I = \frac{m \cdot g \cdot d \cdot T^2}{4\pi^2}$
   - $I$ = moment of inertia (kg\*m²)
   - $m$ = mass of flywheel (kg)
   - $g$ = 9.81 m/s²
   - $d$ = distance from pivot to center of mass (m)
   - $T$ = swing period (s)

See [ORM discussion on swing period method](https://github.com/laberning/openrowingmonitor/discussions/113) for detailed instructions.

#### Method 4: Estimate from similar machines

Use known values as starting points:

| Rower Type | Typical Inertia |
| --- | --- |
| Concept2 RowErg | ~0.1 kg\*m² |
| Kettler Stroker | ~0.03 kg\*m² |
| Generic air rower | 0.05-0.1 kg\*m² |
| Water rower | 0.02-0.05 kg\*m² (varies with fill) |

#### Method 5: Iterative tuning

1. Start with an estimate from similar machines
2. Row at known effort level
3. Compare calculated power/pace to expected values
4. Adjust inertia: too low = metrics too high, too high = metrics too low
5. Repeat until metrics feel right

Note on water rowers:

Water rowers present a unique challenge because effective inertia changes with water level:

- More water = higher effective inertia
- The relationship is non-linear
- Drag factor is also speed-dependent
- Consider using higher `DRAG_COEFFICIENTS_ARRAY_LENGTH` to smooth variations
- Lower `GOODNESS_OF_FIT_THRESHOLD` (0.6-0.8) is often necessary

Start with an estimated value and tune iteratively.

Paraphrases

- "How to measure flywheel inertia?"
- "What is the inertia of my rower?"
- "Flywheel moment of inertia calculation"

Tags: calibration, inertia, measurement, physics

References:

- External resources: [Oxford Rowing Physics](http://eodg.atm.ox.ac.uk/user/dudhia/rowing/physics/ergometer.html), [Dave Vernooy's Flywheel Moment of Inertia Method](https://dvernooy.github.io/projects/ergware/)
- GitHub discussions: [ORM swing period method](https://github.com/laberning/openrowingmonitor/discussions/113)

---

## 11) Which ESP32 boards are supported?

**Short answer:** Any ESP32 development board can work but specific board profiles exist for FireBeetle 2, Lolin S3 Mini, DevKit v1, and Waveshare ESP32-S3 Zero. Dual-core ESP32 modules are recommended for better performance.

What / Why / How

- What: ESP Rowing Monitor runs on ESP32 microcontrollers. Different boards have different features (battery measurement, RGB LEDs, etc.).
- Why: Board-specific profiles optimize pin configurations, power management, and features for each board.
- How: Use a supported board with its profile, or create a custom board profile for your hardware.

Specifically supported boards:

| Board | Features | Notes |
| --- | --- | --- |
| **FireBeetle 2 ESP32-E** | Battery ADC, RGB LED, sensor power switching | Best battery-powered option. 3D printable case available. |
| **FireBeetle 2 ESP32-S3 (N4)** | USB-C, RGB LED | S3 variant with native USB |
| **ESP32 DevKit v1.0** | Basic GPIO | Common development board |
| **Lolin S3 Mini** | Compact, USB-C | Small form factor. 3D printable case available. |
| **Waveshare ESP32-S3 Zero** | Ultra-compact | AA battery powered project available. Low deep sleep current (~0.5mA). |
| **LilyGo T-Display** | Built-in LCD | LCD not supported but board works |
| **ESP32-S3 Super Mini** | Very compact | Higher deep sleep current (~1mA) |

Using unsupported boards:

1. Set the correct board name in `platformio.ini` under `[generic-board]`
2. Create a board profile in `src/profiles/your-board.board-profile.h`
3. Configure pins for sensor, LED, battery ADC, etc.
4. Compile with `custom` environment

Example board profile:

```cpp
#pragma once
#include "../utils/enums.h"

#define BAUD_RATE BaudRates::Baud1500000
#define LED_PIN GPIO_NUM_NC  // No LED or set your pin
#define IS_RGB false
#define SENSOR_PIN_NUMBER GPIO_NUM_26  // Your sensor pin
#define SENSOR_ON_SWITCH_PIN_NUMBER GPIO_NUM_NC
#define WAKEUP_SENSOR_PIN_NUMBER GPIO_NUM_NC
#define BATTERY_PIN_NUMBER GPIO_NUM_NC
// ... other settings
```

Deep sleep and wakeup:

- Wakeup pins must be RTC-capable GPIO pins
- FireBeetle 2 board profile supports separate wakeup pin (reed switch) and sensor pin (hall sensor)
- If no separate wakeup pin is needed, set `WAKEUP_SENSOR_PIN_NUMBER` to `GPIO_NUM_NC` and the sensor pin will be used

Paraphrases

- "What ESP32 board should I use?"
- "Does ESP Rowing Monitor work with ESP32-S3?"
- "How to add support for my board?"

Tags: hardware, esp32, boards, installation

References:

- Official documentation: [Installation Guide](./installation.md), [Settings Guide](./settings.md)
- 3D printable cases: [FireBeetle case](https://www.printables.com/model/441912), [Lolin S3 Mini case](https://www.printables.com/model/1002665), [Waveshare S3 Zero box](https://www.printables.com/model/1438697)
- GitHub discussions: [Support for TTGO ESP32](https://github.com/Abasz/ESPRowingMonitor/discussions/24)

---

## 12) How do I wire the wakeup sensor for battery-powered setups?

**Short answer:** Reed switch goes directly between ground and ESP32 pin (no transistor needed). Hall sensor may need transistor if it requires voltage higher than 3.3V.

What / Why / How

- What: For battery-powered setups, the ESP32 enters deep sleep and wakes up when motion is detected by the sensor or a designated wakup pin.
- Why: Hall sensors consume ~4mA which drains battery; reed switches consume no power (passive component).
- How: Configure wakeup pin, wire sensor appropriately based on voltage requirements.

### Wiring options

#### Option 1: Reed switch only (simplest)

- Reed one side to GND, other side to ESP32 wakeup pin
- No transistor or power switching needed
- Can use same pin for both wakeup and impulse detection (though reed for singal detection in inferior - see [Entry #3](#3-should-i-use-a-hall-sensor-or-a-reed-switch))

#### Option 2: Hall sensor at 3.3V (test first)

- If hall works at 3.3V, connect its VCC directly to `SENSOR_ON_SWITCH_PIN`
- When pin goes high, it supplies voltage; when low, sensor is off
- ESP32 can supply ~20-30mA, enough for most hall sensors

#### Option 3: Hall sensor needing higher voltage (with transistor)

- Use NPN transistor or MOSFET (2N7000 recommended) as low-side switch
- Emitter/Source to GND, Collector/Drain to hall sensor GND
- ESP32 `SENSOR_ON_SWITCH_PIN` to Base/Gate
- Hall VCC connected to battery/higher voltage rail

Configuration in board profile:

```cpp
// Using reed for both wakeup and detection
#define SENSOR_PIN_NUMBER GPIO_NUM_25
#define SENSOR_ON_SWITCH_PIN_NUMBER GPIO_NUM_NC
#define WAKEUP_SENSOR_PIN_NUMBER GPIO_NUM_NC  // Uses sensor pin

// Using separate wakeup reed and switched hall
#define SENSOR_PIN_NUMBER GPIO_NUM_25  // Hall sensor
#define SENSOR_ON_SWITCH_PIN_NUMBER GPIO_NUM_26  // Controls hall power
#define WAKEUP_SENSOR_PIN_NUMBER GPIO_NUM_27  // Reed switch
```

Community experience

- 3.3V setup without transistor confirmed working on Waterrower with KY-024

Failure modes

- Using non-RTC GPIO pin for wakeup → device won't wake
- Dead reed switch → no wakeup after long sleep periods
- Hall sensor drawing too much current → battery drains in sleep

Paraphrases

- "How to wire wakeup sensor for battery power?"
- "Reed switch wiring for ESP32 deep sleep"
- "Do I need a transistor for hall sensor?"

Tags: wiring, power-management, battery, deep-sleep

References:

- Official documentation: [Settings Guide](./settings.md)
- GitHub discussions: [Wakeup sensor wiring](https://github.com/Abasz/ESPRowingMonitor/discussions/13)

---

## 13) How do I connect ESP Rowing Monitor to fitness apps via Bluetooth?

**Short answer:** ESP Rowing Monitor implements several standard BLE profiles including FTMS Rower (experimental), Cycling Power, and Cycling Speed/Cadence. Connect from apps like EXR, Kinomap, or use the official WebGUI.

What / Why / How

- What: ESP Rowing Monitor implements standard BLE fitness profiles for compatibility with fitness apps and smartwatches.
- Why: Standard BLE profiles allow connection to fitness apps, watches, and training platforms.
- How: Flash firmware, power on ESP32, scan from your app/device, connect.

Supported BLE profiles:

1. **Cycling Power Meter (CPS)** — Default profile. Works with Garmin watches (cycling activity required).
2. **Cycling Speed and Cadence (CSC)** — Alternative profile. Set wheel circumference to 10mm when pairing.
3. **Fitness Machine (FTMS) Rower** — Experimental. Supports distance, pace, resistance, power. Limited control options.

Tested apps and platforms:

- **WebGUI** — [Official app, full feature support](https://abasz.github.io/ESPRowingMonitor-WebGUI/)
- **EXR** — Confirmed working
- **Kinomap** — Confirmed working
- **Garmin watches** — Works with CPS profile. Must select cycling activity. Tested: FR235, FR645, FR255

Dual connection support:

ESP Rowing Monitor supports **two simultaneous BLE connections**, allowing use of:

- WebGUI + smartwatch
- WebGUI + fitness app
- Two WebGUI instances (useful for coach and athlete displays)
- Any other combination of the above (i.e. possible to exclude WebGUI)

Switching profiles via WebGUI:

1. Connect to ESP Rowing Monitor with WebGUI
2. Go to Settings
3. Select desired BLE service type
4. Changes take effect immediately

**Important for Garmin:** Due to Garmin limitations, the watch will connect but won't use sensor data unless a cycling activity is selected.

**Important for CSC/CPS:** Set wheel circumference to 10mm when pairing for accurate speed/distance.

Paraphrases

- "How to connect ESP Rowing Monitor to EXR?"
- "Which apps work with ESP Rowing Monitor?"
- "ESP Rowing Monitor Bluetooth setup for Garmin"

Tags: bluetooth, FTMS, apps, connectivity

References:

- Official documentation: [Features](./features.md), [Custom BLE Services](./custom-ble-services.md)
- [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/)

---

## 14) Where are logs and raw data — how do I get them for debugging?

**Short answer:** Real-time logs stream via serial (USB); deltaTime data exports from WebGUI; optional SD card logging available.

What / Why / How

- What: ESP Rowing Monitor writes real-time logs to serial output, exports session data via BLE to WebGUI, and optionally logs to SD card.
- Why: Logs show live stroke detection, phase transitions, and calculated metrics; raw data reveals sensor quality issues; stored data allows calibration and debugging.
- How: Connect via USB and use serial monitor, or export from WebGUI logbook.

Enable detailed logging:

In your settings file:

```cpp
#define DEFAULT_CPS_LOGGING_LEVEL ArduinoLogLevel::LogLevelTrace
```

Or change at runtime via WebGUI or BLE OpCode 17.

View real-time logs:

```bash
# Using PlatformIO
platformio device monitor --baud 1500000

# Or use online serial terminal:
# https://googlechromelabs.github.io/serial-terminal/
```

Export deltaTime data from WebGUI:

1. Enable `ENABLE_BLUETOOTH_DELTA_TIME_LOGGING true` in settings
2. Row a session while connected to WebGUI
3. Go to Logbook in WebGUI
4. Export session as JSON/CSV

SD card logging (optional):

Enable in settings:

```cpp
#define SUPPORT_SD_CARD_LOGGING true
```

DeltaTimes written incrementally on every stroke or every 4 seconds.

Paraphrases

- "Where does ESP Rowing Monitor write log files?"
- "How do I get raw recordings for troubleshooting?"
- "What do stroke detection logs look like?"

Tags: data, logs, troubleshooting

References:

- Official documentation: [Settings Guide](./settings.md), [Features - Logging](./features.md)
- GitHub discussions: [How to use e2e calibration tool](https://github.com/Abasz/ESPRowingMonitor/discussions/18)

---

## 15) Strokes are getting lost or stroke detection needs tuning

**Short answer:** Lost strokes can be caused by sensor/signal issues OR incorrect configuration parameters. Follow a systematic diagnostic to isolate the problem and tune the stroke detection parameters for your rower.

What / Why / How

- What: The algorithm converts impulse timing into angular velocity, detects acceleration/deceleration flanks, and determines stroke phases (drive vs recovery).
- Why: Different rowers have vastly different characteristics — tuning is needed. Clean signals don't guarantee stroke detection if parameters are wrong for your rower.
- How: Start with a profile close to your rower type, record raw data, analyze with simulation tool, tune parameters based on observed patterns.

Diagnostic flow

1. **Verify sensor pulses:** Rotate the flywheel by hand and confirm pulses in logs or WebGUI
2. **Check sensor hardware:** Verify sensor distance (< 1mm recommended), polarity, and mounting rigidity
3. **Review configuration parameters:** Even with clean pulses, wrong settings can cause missed strokes
4. **Analyze deltaTime data:** Export from WebGUI, run e2e simulation to generate output file
5. **Visualize with calibration helper GUI:** Use the Python GUI tool (available executable on the release page) to visualize and analyze the simulation output:

   The GUI provides:

   - **Delta Times tab:** View raw vs cleaned delta times
   - **Handle Forces tab:** Navigate through force curves for each stroke
   - **Stroke Detection tab:** Analyze stroke detection accuracy and identify anomalies
   - **Summary tab:** Shows average metrics for all opened files

6. **Share for community review:** If still failing, post your trace and config for community analysis

Stroke detection types:

```cpp
#define STROKE_DETECTION_TYPE STROKE_DETECTION_TORQUE  // Recommended for most
// Options: STROKE_DETECTION_SLOPE, STROKE_DETECTION_TORQUE, STROKE_DETECTION_BOTH
```

- **TORQUE:** Detects phase changes based on handle force. Requires good signal and sufficient resolution.
- **SLOPE:** Detects phase changes based on angular velocity trend (speed up and slow down). Works well with any signal but tends to trigger recovery too early.
- **BOTH:** Uses both methods (requires knowing and setting the recovery slope otherwise with `MINIMUM_RECOVERY_SLOPE` of zero will essentially fold into **SLOPE**).

Key parameters explained:

| Parameter | Purpose | Typical Values |
| --- | --- | --- |
| `IMPULSE_DATA_ARRAY_LENGTH` | Samples for regression (like flankLength in ORM) | 1.5-2× magnet count |
| `MINIMUM_DRIVE_TIME` | Minimum drive phase duration | 300-500 ms |
| `MINIMUM_RECOVERY_TIME` | Minimum recovery phase duration | 500-1'000 ms |
| `MINIMUM_DRAG_TORQUE` | Torque threshold below which recovery is triggered | 0-0.3 |
| `MINIMUM_POWERED_TORQUE` | Torque threshold above which drive is detected | 0 |
| `GOODNESS_OF_FIT_THRESHOLD` | Min R² for drag calculation | 0.6-0.97 |
| `DRIVE_HANDLE_FORCES_MAX_CAPACITY` | Max impulses per drive (failsafe for missed recovery detection) | 50-70 |

Common issues and fixes:

**No distance calculated (stroke count works):**

- `GOODNESS_OF_FIT_THRESHOLD` too high → lower to 0.6-0.8 for water/magnetic rowers
- `MAX_DRAG_FACTOR_RECOVERY_PERIOD` too short → increase to 4'000ms
- Drag factor thresholds too narrow → widen `LOWER_DRAG_FACTOR_THRESHOLD` and `UPPER_DRAG_FACTOR_THRESHOLD`

**Ghost strokes at end of session:**

- Signal noise at low RPM causes false stroke detection
- Increase `MINIMUM_RECOVERY_TIME`
- Increase `MINIMUM_POWERED_TORQUE`
- Check sensor mounting/distance

**Strokes missed during rowing:**

- `MINIMUM_DRIVE_TIME` or `MINIMUM_RECOVERY_TIME` too high
- `IMPULSE_DATA_ARRAY_LENGTH` too small for magnet count
- `MINIMUM_DRAG_TORQUE` is too low
- Signal bounce causing rejected impulses

Magnetic rower specifics:

Magnetic rowers have different drag characteristics:

- Drag is friction-based (constant) not cubic (like air)
- Drag factor varies more with rotation speed
- Lower `GOODNESS_OF_FIT_THRESHOLD` needed (0.6)
- Higher `UPPER_DRAG_FACTOR_THRESHOLD` needed (5'000-10'000)
- High resistance settings may stop flywheel completely (algorithm fails) as it needs freely rotating flywheel for recovery.

Community experience:

- Many lost-stroke reports were fixed by adjusting configuration parameters rather than changing sensor hardware
- For high-magnet setups: stroke detection can fail due to measurement error amplification — filtering parameters need tuning
- Slope-based detection often more reliable than slope-based for noisy signals

Paraphrases

- "Why is ESP Rowing Monitor missing my strokes?"
- "How to tune stroke detection?"
- "What does IMPULSE_DATA_ARRAY_LENGTH mean?"
- "ESP Rowing Monitor parameters explained"
- "Clean sensor signal but strokes still lost"

Tags: troubleshooting, stroke-detection, configuration, calibration, parameters

References:

- Official documentation: [Settings Guide](./settings.md)
- GitHub discussions: [Stroke detection works but no distance](https://github.com/Abasz/ESPRowingMonitor/discussions/15), [Kettler Stroker](https://github.com/Abasz/ESPRowingMonitor/discussions/20), [How to use e2e calibration tool](https://github.com/Abasz/ESPRowingMonitor/discussions/18)

---

## 16) Older Concept2 rowers (Model C and earlier) — sensor compatibility

**Short answer:** Concept2 Model C and earlier models use coil-based sensors that produce analog signals. These are **not directly compatible** with ESP Rowing Monitor. You need to add a new sensor (hall or reed) or use signal conditioning.

What / Why / How

- What: Older Concept2 rowers (Model C and earlier) use specialized VR (Variable Reluctance) sensors that output analog sine waves, not digital pulses that ESP Rowing Monitor expects.
- Why: VR sensors generate AC-like voltage proportional to rotation speed, which requires amplification and conversion to digital signal.
- How: The factory VR sensor won't work with ESP Rowing Monitor directly. You have two options:
  1. **Add a new sensor (recommended):** Install magnets on the flywheel and add a hall sensor nearby (see [Entry #6](#6-which-hall-sensor-modules-are-known-to-work)). Concept2 already has 3 magnets inside — you can replace the pickup with a hall sensor.
  2. **Signal conditioning (advanced):** Amplify the VR signal and convert to digital with a comparator. Simpler to add new sensor unless you need compatibility with factory monitor.

PM2 sensor circuit (for reference):

- Coil changes resistance when magnet passes (millivolt range)
- Amplifier boosts signal to usable voltage
- Voltage comparator triggers high/low digital output

Recommended approach:

1. Remove existing pickup (see [C2 Pickup Replacement Guide](http://www.concept2.co.uk/files/pdf/us/indoor-rowers/C_MonitorPickupReplacement.pdf))
2. Install hall sensor board (e.g., [KY-003](https://www.joy-it.net/en/products/SEN-KY003HMS))
3. Position hall close to existing magnets (3 magnets already present)
4. Configure: `IMPULSES_PER_REVOLUTION 3`, `FLYWHEEL_INERTIA 0.101`

Paraphrases

- "Does Concept2 Model C work with ESP Rowing Monitor?"
- "How to use factory sensor on old Concept2?"
- "Concept2 VR sensor compatibility"

Tags: concept2, sensors, hardware, compatibility, VR-sensor

References:

- [C2 Pickup Replacement Guide](http://www.concept2.co.uk/files/pdf/us/indoor-rowers/C_MonitorPickupReplacement.pdf)
- [C2 Forum - PM2 sensor discussion](https://www.c2forum.com/viewtopic.php?t=94893)
- GitHub discussions: [Concept 2 Analog in](https://github.com/Abasz/ESPRowingMonitor/discussions/12)

---

## 17) Differences between ESP Rowing Monitor and OpenRowingMonitor

**Short answer:** ESP Rowing Monitor is a port of OpenRowingMonitor optimized for ESP32 microcontrollers. They share core algorithms but differ in platform, features, and resource requirements.

What / Why / How

- What: Both projects calculate rowing metrics from flywheel rotation. ESP Rowing Monitor runs on ESP32; OpenRowingMonitor runs on Raspberry Pi.
- Why: ESP32 is cheaper, more portable, battery-friendly. Raspberry Pi offers more features and processing power.
- How: Choose based on your needs — portability vs features.

| Feature | ESP Rowing Monitor | OpenRowingMonitor |
| --- | --- | --- |
| **Platform** | ESP32 microcontroller | Raspberry Pi |
| **Cost** | ~$5-15 for ESP32 | ~$35-75 for Pi |
| **Power** | Battery or USB (μA sleep) | Requires constant power |
| **Display** | WebGUI only (phone/tablet) | WebGUI + physical display |
| **Installation** | Flash firmware, configure | Full OS, npm install |
| **Features** | Core rowing metrics | Full features + recording |
| **Drag factor** | Always recalculates | Can be fixed or auto |
| **Resources** | Limited (256KB RAM) | Abundant (1-8GB RAM) |

Shared between projects:

- Core physics algorithms
- Stroke detection methods
- Many configuration parameters
- Compatible with same rowers
- Similar calibration approaches

ESP Rowing Monitor limitations:

- No fixed drag factor option (always recalculates)
- Simplified algorithms in some areas due to resource constraints
- No local session storage (exports via BLE)
- No direct display support

When to choose ESP Rowing Monitor:

- Battery-powered portable setup
- Budget-conscious build
- Simple installation (flash and go)
- Existing Raspberry Pi not available

When to choose OpenRowingMonitor:

- Want physical display on rower
- Need full feature set
- Local session storage required
- Don't need portability

Paraphrases

- "What's the difference between ESP Rowing Monitor and ORM?"
- "Should I use ESP32 or Raspberry Pi?"
- "Is ESP Rowing Monitor compatible with ORM settings?"

Tags: comparison, openrowingmonitor, platforms

References:

- [ESP Rowing Monitor](https://github.com/Abasz/ESPRowingMonitor)
- [OpenRowingMonitor](https://github.com/JaapvanEkris/openrowingmonitor)
- [ORM Rower Settings Guide](https://github.com/JaapvanEkris/openrowingmonitor/tree/main/docs/rower_settings.md)

---

## Quick Reference: Common Error Messages

| Error/Symptom | Likely Cause | Fix |
| --- | --- | --- |
| No strokes detected, deltaTime shows values | Configuration issue | Check and tweak settings |
| Strokes detected but no distance | `GOODNESS_OF_FIT_THRESHOLD` too high or `UPPER_DRAG_FACTOR_THRESHOLD` too low | Lower GoF to 0.6-0.8, increase drag factor threshold 1'000-5'000 |
| Ghost strokes at session end | Signal noise at low RPM | Increase `MINIMUM_RECOVERY_TIME`, check sensor mounting |
| Power values too low (e.g., 50W feels wrong) | Wrong `FLYWHEEL_INERTIA` or recovery includes drive and drag factor is too low | Check if drag factor is too low, adjust inertia until power feels right |
| Device doesn't wake from sleep | Non-RTC pin used for wakeup, or dead reed | Verify RTC-capable pin, test reed by shorting pins |
| Erratic deltaTime values | Sensor bounce or inconsitent triggering | Position sensor < 1mm from magnets, use hall, Enable `ENABLE_DEBOUNCE_FILTER` as last result |
| Boot loop on startup | Invalid GPIO pin configuration | Check pin numbers are valid for your ESP32 variant |
| BLE connection fails on iOS | iOS doesn't support Web Bluetooth natively | Use special browser app (supported in v7.0.0+) |
| Deep sleep current too high | Sensor power not switched off | Configure `SENSOR_ON_SWITCH_PIN_NUMBER` properly |
| Force curve has tail or wrong shape | Stroke detection is not tuned properly | Adjust settings |
| DeltaTime has periodic spikes | Uneven magnet spacing | Reposition magnets |
