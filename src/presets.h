/*
 * Reflow Profile Presets
 * Allows saving and loading different temperature profiles
 * for different types of boards and components
 */

#ifndef PRESETS_H
#define PRESETS_H

#include <Arduino.h>

#define MAX_PRESETS 10
#define PRESET_NAME_LENGTH 32
#define PRESETS_FILE "/presets.json"

// Reflow profile structure
struct ReflowProfile {
  char name[PRESET_NAME_LENGTH];
  double preheatTemp;
  double soakTemp;
  double reflowTemp;
  double cooldownTemp;
  unsigned long preheatTime;
  unsigned long soakTime;
  unsigned long reflowTime;
  unsigned long cooldownTime;
  char description[64];
};

// Initialize preset system with default profiles
void initPresets();

// Get number of available presets
int getPresetCount();

// Get preset by index
ReflowProfile* getPreset(int index);

// Get preset by name
ReflowProfile* getPresetByName(const char* name);

// Save a new preset or update existing one
bool savePreset(const ReflowProfile& profile);

// Delete a preset by name
bool deletePreset(const char* name);

// Load a preset into current working parameters
void loadPresetIntoWorkingProfile(const char* name);

// Save current working parameters as a preset
bool saveWorkingProfileAsPreset(const char* name, const char* description);

// Get all presets as JSON string
String getPresetsJSON();

// Load presets from flash
void loadPresetsFromFlash();

// Save presets to flash
void savePresetsToFlash();

#endif // PRESETS_H

