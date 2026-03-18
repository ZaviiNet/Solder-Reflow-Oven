/*
 * Reflow Profile Presets Implementation
 */

#include "presets.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// Forward declarations for external variables
extern double preheatTemp, soakTemp, reflowTemp, cooldownTemp;
extern unsigned long preheatTime, soakTime, reflowTime, cooldownTime;

// Preset storage
static ReflowProfile presets[MAX_PRESETS];
static int presetCount = 0;

// Default presets
// Based on JEDEC J-STD-020 standards and industry best practices
static const ReflowProfile defaultPresets[] = {
  {
    "Lead-Free Standard",
    150, 180, 245, 25,
    90000, 90000, 40000, 60000,
    "SAC305 lead-free, 245C peak (JEDEC compliant)"
  },
  {
    "Lead-Free Safe",
    145, 175, 235, 25,
    90000, 90000, 40000, 60000,
    "Conservative lead-free for standard plastic headers"
  },
  {
    "Leaded Solder",
    120, 150, 220, 25,
    90000, 90000, 40000, 60000,
    "Sn63/Pb37 leaded solder (210-230C range)"
  },
  {
    "Sensitive Components",
    130, 160, 210, 25,
    90000, 90000, 35000, 60000,
    "Non-rated plastics, cheap connectors (<210C)"
  },
  {
    "High Mass Boards",
    160, 190, 250, 25,
    120000, 120000, 50000, 90000,
    "Thick PCBs, large thermal mass (max 250C)"
  }
};

void initPresets() {
  Serial.println("Initializing reflow presets...");

  // Try to load from flash first
  loadPresetsFromFlash();

  // If no presets loaded, use defaults
  if (presetCount == 0) {
    Serial.println("No presets found, loading defaults...");
    int defaultCount = sizeof(defaultPresets) / sizeof(defaultPresets[0]);
    for (int i = 0; i < defaultCount && i < MAX_PRESETS; i++) {
      presets[i] = defaultPresets[i];
      presetCount++;
    }
    // Save defaults to flash
    savePresetsToFlash();
    Serial.print("Loaded ");
    Serial.print(presetCount);
    Serial.println(" default presets");
  }
}

int getPresetCount() {
  return presetCount;
}

ReflowProfile* getPreset(int index) {
  if (index >= 0 && index < presetCount) {
    return &presets[index];
  }
  return nullptr;
}

ReflowProfile* getPresetByName(const char* name) {
  for (int i = 0; i < presetCount; i++) {
    if (strcmp(presets[i].name, name) == 0) {
      return &presets[i];
    }
  }
  return nullptr;
}

bool savePreset(const ReflowProfile& profile) {
  // Check if preset with this name already exists
  for (int i = 0; i < presetCount; i++) {
    if (strcmp(presets[i].name, profile.name) == 0) {
      // Update existing preset
      presets[i] = profile;
      savePresetsToFlash();
      Serial.print("Updated preset: ");
      Serial.println(profile.name);
      return true;
    }
  }

  // Add new preset
  if (presetCount < MAX_PRESETS) {
    presets[presetCount] = profile;
    presetCount++;
    savePresetsToFlash();
    Serial.print("Added new preset: ");
    Serial.println(profile.name);
    return true;
  }

  Serial.println("ERROR: Maximum presets reached!");
  return false;
}

bool deletePreset(const char* name) {
  for (int i = 0; i < presetCount; i++) {
    if (strcmp(presets[i].name, name) == 0) {
      // Shift remaining presets down
      for (int j = i; j < presetCount - 1; j++) {
        presets[j] = presets[j + 1];
      }
      presetCount--;
      savePresetsToFlash();
      Serial.print("Deleted preset: ");
      Serial.println(name);
      return true;
    }
  }
  Serial.print("Preset not found: ");
  Serial.println(name);
  return false;
}

void loadPresetIntoWorkingProfile(const char* name) {
  ReflowProfile* preset = getPresetByName(name);
  if (preset) {
    preheatTemp = preset->preheatTemp;
    soakTemp = preset->soakTemp;
    reflowTemp = preset->reflowTemp;
    cooldownTemp = preset->cooldownTemp;
    preheatTime = preset->preheatTime;
    soakTime = preset->soakTime;
    reflowTime = preset->reflowTime;
    cooldownTime = preset->cooldownTime;

    Serial.print("Loaded preset: ");
    Serial.println(name);
    Serial.print("  Preheat: "); Serial.print(preheatTemp); Serial.println("°C");
    Serial.print("  Soak: "); Serial.print(soakTemp); Serial.println("°C");
    Serial.print("  Reflow: "); Serial.print(reflowTemp); Serial.println("°C");
  } else {
    Serial.print("ERROR: Preset not found: ");
    Serial.println(name);
  }
}

bool saveWorkingProfileAsPreset(const char* name, const char* description) {
  ReflowProfile profile;
  strncpy(profile.name, name, PRESET_NAME_LENGTH - 1);
  profile.name[PRESET_NAME_LENGTH - 1] = '\0';
  strncpy(profile.description, description, 63);
  profile.description[63] = '\0';

  profile.preheatTemp = preheatTemp;
  profile.soakTemp = soakTemp;
  profile.reflowTemp = reflowTemp;
  profile.cooldownTemp = cooldownTemp;
  profile.preheatTime = preheatTime;
  profile.soakTime = soakTime;
  profile.reflowTime = reflowTime;
  profile.cooldownTime = cooldownTime;

  return savePreset(profile);
}

String getPresetsJSON() {
  JsonDocument doc;
  JsonArray presetsArray = doc.to<JsonArray>();

  for (int i = 0; i < presetCount; i++) {
    JsonObject preset = presetsArray.add<JsonObject>();
    preset["name"] = presets[i].name;
    preset["description"] = presets[i].description;
    preset["preheatTemp"] = presets[i].preheatTemp;
    preset["soakTemp"] = presets[i].soakTemp;
    preset["reflowTemp"] = presets[i].reflowTemp;
    preset["cooldownTemp"] = presets[i].cooldownTemp;
    preset["preheatTime"] = presets[i].preheatTime;
    preset["soakTime"] = presets[i].soakTime;
    preset["reflowTime"] = presets[i].reflowTime;
    preset["cooldownTime"] = presets[i].cooldownTime;
  }

  String result;
  serializeJson(doc, result);
  return result;
}

void loadPresetsFromFlash() {
  if (!LittleFS.exists(PRESETS_FILE)) {
    Serial.println("No presets file found");
    return;
  }

  File presetsFile = LittleFS.open(PRESETS_FILE, "r");
  if (!presetsFile) {
    Serial.println("Failed to open presets file");
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, presetsFile);
  presetsFile.close();

  if (error) {
    Serial.print("Failed to parse presets file: ");
    Serial.println(error.c_str());
    return;
  }

  JsonArray presetsArray = doc.as<JsonArray>();
  presetCount = 0;

  for (JsonObject preset : presetsArray) {
    if (presetCount >= MAX_PRESETS) break;

    strncpy(presets[presetCount].name, preset["name"] | "", PRESET_NAME_LENGTH - 1);
    presets[presetCount].name[PRESET_NAME_LENGTH - 1] = '\0';

    strncpy(presets[presetCount].description, preset["description"] | "", 63);
    presets[presetCount].description[63] = '\0';

    presets[presetCount].preheatTemp = preset["preheatTemp"] | 150;
    presets[presetCount].soakTemp = preset["soakTemp"] | 180;
    presets[presetCount].reflowTemp = preset["reflowTemp"] | 230;
    presets[presetCount].cooldownTemp = preset["cooldownTemp"] | 25;
    presets[presetCount].preheatTime = preset["preheatTime"] | 90000;
    presets[presetCount].soakTime = preset["soakTime"] | 90000;
    presets[presetCount].reflowTime = preset["reflowTime"] | 40000;
    presets[presetCount].cooldownTime = preset["cooldownTime"] | 60000;

    presetCount++;
  }

  Serial.print("Loaded ");
  Serial.print(presetCount);
  Serial.println(" presets from flash");
}

void savePresetsToFlash() {
  Serial.println("Saving presets to flash...");

  JsonDocument doc;
  JsonArray presetsArray = doc.to<JsonArray>();

  for (int i = 0; i < presetCount; i++) {
    JsonObject preset = presetsArray.add<JsonObject>();
    preset["name"] = presets[i].name;
    preset["description"] = presets[i].description;
    preset["preheatTemp"] = presets[i].preheatTemp;
    preset["soakTemp"] = presets[i].soakTemp;
    preset["reflowTemp"] = presets[i].reflowTemp;
    preset["cooldownTemp"] = presets[i].cooldownTemp;
    preset["preheatTime"] = presets[i].preheatTime;
    preset["soakTime"] = presets[i].soakTime;
    preset["reflowTime"] = presets[i].reflowTime;
    preset["cooldownTime"] = presets[i].cooldownTime;
  }

  File presetsFile = LittleFS.open(PRESETS_FILE, "w");
  if (!presetsFile) {
    Serial.println("Failed to open presets file for writing");
    return;
  }

  if (serializeJson(doc, presetsFile) == 0) {
    Serial.println("Failed to write presets");
  } else {
    Serial.println("Presets saved successfully!");
  }

  presetsFile.close();
}

