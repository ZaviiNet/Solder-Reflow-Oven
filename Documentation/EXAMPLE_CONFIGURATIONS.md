# Example Configuration File

This file provides example configurations for different solder types and oven setups.

## How to Use

1. Choose the profile that matches your solder paste
2. Enter values in the web interface
3. Click "Save Configuration" to store in EEPROM
4. Run PID auto-tuning to optimize for your specific oven
5. Test with scrap PCBs before production

## Lead-Free Solder (SAC305) - Default Profile

**Best for:** Most modern lead-free applications

```
Preheat Temperature:  150°C
Preheat Time:        90000 ms (90 seconds)

Soak Temperature:    180°C
Soak Time:          90000 ms (90 seconds)

Reflow Temperature:  230°C
Reflow Time:        40000 ms (40 seconds)

Cooldown Target:     25°C
Cooldown Time:      60000 ms (60 seconds)
```

**Initial PID Values (tune after):**
```
Kp: 2.0
Ki: 5.0
Kd: 1.0
```

## Leaded Solder (Sn63/Pb37)

**Best for:** Hobby projects, through-hole, older designs

```
Preheat Temperature:  120°C
Preheat Time:        90000 ms (90 seconds)

Soak Temperature:    150°C
Soak Time:          90000 ms (90 seconds)

Reflow Temperature:  210°C
Reflow Time:        40000 ms (40 seconds)

Cooldown Target:     25°C
Cooldown Time:      60000 ms (60 seconds)
```

**Initial PID Values (tune after):**
```
Kp: 2.0
Ki: 5.0
Kd: 1.0
```

## Low-Temperature Lead-Free (LTS)

**Best for:** Temperature-sensitive components, bismuth-based pastes

```
Preheat Temperature:  100°C
Preheat Time:        90000 ms (90 seconds)

Soak Temperature:    130°C
Soak Time:          90000 ms (90 seconds)

Reflow Temperature:  180°C
Reflow Time:        40000 ms (40 seconds)

Cooldown Target:     25°C
Cooldown Time:      60000 ms (60 seconds)
```

**Initial PID Values (tune after):**
```
Kp: 1.8
Ki: 4.5
Kd: 0.9
```

## High-Temperature Lead-Free (SAC387)

**Best for:** High-reliability applications, automotive

```
Preheat Temperature:  160°C
Preheat Time:        90000 ms (90 seconds)

Soak Temperature:    190°C
Soak Time:          90000 ms (90 seconds)

Reflow Temperature:  245°C
Reflow Time:        40000 ms (40 seconds)

Cooldown Target:     25°C
Cooldown Time:      60000 ms (60 seconds)
```

**Initial PID Values (tune after):**
```
Kp: 2.2
Ki: 5.5
Kd: 1.1
```

## Quick Reflow (Small Boards)

**Best for:** Small PCBs, fast turnaround, testing

```
Preheat Temperature:  150°C
Preheat Time:        60000 ms (60 seconds)

Soak Temperature:    180°C
Soak Time:          60000 ms (60 seconds)

Reflow Temperature:  230°C
Reflow Time:        30000 ms (30 seconds)

Cooldown Target:     25°C
Cooldown Time:      45000 ms (45 seconds)
```

**Initial PID Values (tune after):**
```
Kp: 2.5
Ki: 6.0
Kd: 1.2
```

## Slow Reflow (Large/Thick Boards)

**Best for:** Large PCBs, thick copper, high thermal mass

```
Preheat Temperature:  150°C
Preheat Time:       120000 ms (120 seconds)

Soak Temperature:    180°C
Soak Time:         120000 ms (120 seconds)

Reflow Temperature:  230°C
Reflow Time:        60000 ms (60 seconds)

Cooldown Target:     25°C
Cooldown Time:      90000 ms (90 seconds)
```

**Initial PID Values (tune after):**
```
Kp: 1.5
Ki: 4.0
Kd: 0.8
```

## PID Tuning Guidelines

### After Auto-Tuning

The auto-tune feature will calculate optimal values. Typical results:

**Small/Fast Ovens:**
- Kp: 0.5 - 1.5
- Ki: 0.001 - 0.003
- Kd: 0.3 - 0.6

**Medium Ovens:**
- Kp: 1.5 - 3.0
- Ki: 0.003 - 0.007
- Kd: 0.6 - 1.2

**Large/Slow Ovens:**
- Kp: 3.0 - 5.0
- Ki: 0.007 - 0.015
- Kd: 1.2 - 2.0

### Manual Tuning Tips

If auto-tune doesn't work or you want to fine-tune:

**Temperature Oscillates (too aggressive):**
- Decrease Kp by 20%
- Decrease Kd by 20%

**Temperature Overshoots:**
- Increase Kd by 20%
- Decrease Kp by 10%

**Temperature Slow to Reach Target:**
- Increase Kp by 20%
- Increase Ki by 20%

**Temperature Steady-State Error:**
- Increase Ki by 50%

**Temperature Unstable:**
- Decrease all values by 30%
- Start with Kp=1, Ki=2, Kd=0.5

## Important Safety Notes

⚠️ **Always test new profiles with:**
1. Empty oven (no PCB) first
2. Watch for runaway heating
3. Verify emergency stop works
4. Monitor peak temperatures
5. Check for proper cooldown

⚠️ **Never exceed:**
- Your thermocouple's maximum rating (usually 350°C for K-type)
- Your PCB's maximum temperature rating
- Your component's maximum temperature rating
- Your oven's safe operating temperature

⚠️ **Temperature Limits:**
- Typical PCB substrate: 260°C max
- Most components: Check datasheet
- Solder paste: Follow manufacturer specs

## Troubleshooting Profiles

### Temperature Never Reaches Target

**Possible causes:**
- Heater too weak for oven size
- Poor thermal insulation
- Thermocouple placed incorrectly
- PID values too conservative

**Solutions:**
- Check heater power rating
- Add insulation to oven
- Position thermocouple near PCB
- Increase Kp and Ki values

### Temperature Overshoots Significantly

**Possible causes:**
- PID too aggressive
- Thermal mass mismatch
- Heater too powerful

**Solutions:**
- Run PID auto-tuning
- Decrease Kp and increase Kd
- Use slower ramp rates

### Uneven Heating

**Possible causes:**
- Thermocouple not representative
- Poor air circulation
- Hot spots in oven

**Solutions:**
- Place thermocouple in center
- Add circulation fan
- Use slower profiles
- Consider multiple zones (future feature)

## Configuration via API

You can also set configuration via HTTP POST to `/api/config`:

```json
{
  "preheatTemp": 150,
  "soakTemp": 180,
  "reflowTemp": 230,
  "cooldownTemp": 25,
  "preheatTime": 90000,
  "soakTime": 90000,
  "reflowTime": 40000,
  "cooldownTime": 60000,
  "Kp": 2.0,
  "Ki": 5.0,
  "Kd": 1.0,
  "saveToEEPROM": true
}
```

## Profile Storage

- Configurations are stored in EEPROM/Flash
- Survive power cycles
- One profile can be saved at a time
- Future versions may support multiple saved profiles

## Additional Resources

- **Solder Paste Datasheets**: Always consult your paste manufacturer
- **IPC-7530**: Industry standard for temperature profiling
- **Component Datasheets**: Check maximum reflow temperatures
- **[PID_TUNING_GUIDE.md](PID_TUNING_GUIDE.md)**: Detailed PID tuning instructions
- **[AUTO_TUNE_FEATURE.md](AUTO_TUNE_FEATURE.md)**: Auto-tuning guide

## Example Workflow

1. **Select Profile**: Choose based on your solder paste
2. **Enter via Web Interface**: Input all temperatures and times
3. **Save**: Click "Save Configuration"
4. **Auto-Tune**: Click "Auto-Tune PID" (10-15 min)
5. **Save Again**: Save the tuned PID values
6. **Test Run**: Empty oven, watch temperature curve
7. **Verify**: Check all stages reach target temps
8. **Production**: Use with actual PCBs

Enjoy your perfectly tuned reflow oven! 🔥
