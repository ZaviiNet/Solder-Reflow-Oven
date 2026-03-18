/*
 * Reflow State Machine
 * Controls the reflow process stages
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"

// Initialize state machine
void initStateMachine();

// Get current state
ReflowState getCurrentState();

// Change state
void changeState(ReflowState newState);

// Update state machine (call in loop)
void updateReflowStateMachine();

// Start reflow process
void startReflow();

// Stop reflow process
void stopReflow();

// Emergency stop
void emergencyStopReflow();

// Get state name
String getStateName(ReflowState state);

// Check if emergency stop is active
bool isEmergencyStop();

// Data logging
void logDataPoint();
DataPoint* getDataLog();
int getDataLogIndex();
void resetDataLog();

#endif // STATE_MACHINE_H

