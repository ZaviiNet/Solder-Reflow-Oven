/*
 * PID Controller and Auto-Tuning
 */

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>
#include <PID_v1.h>

// Initialize PID controller
void initPIDController();

// Get PID parameters
double getPIDKp();
double getPIDKi();
double getPIDKd();

// Set PID parameters
void setPIDTunings(double kp, double ki, double kd);

// Get PID setpoint
double getPIDSetpoint();

// Set PID setpoint
void setPIDSetpoint(double setpoint);

// Get PID output
double getPIDOutput();

// Set PID output (for manual control)
void setPIDOutput(double output);

// Get PID input
double getPIDInput();

// Set PID input
void setPIDInput(double input);

// Compute PID output
bool computePID();

// Set PID mode
void setPIDMode(int mode);

// Get PID instance (for direct access if needed)
PID* getPIDInstance();

// PID Auto-Tuning Functions
void startPIDTuning();
void stopPIDTuning();
void updatePIDTuningStateMachine();
bool isPIDTuningActive();

// Console logging for tuning
void addConsoleLog(String message);
void clearConsoleLog();
String* getConsoleLog();
int getConsoleLogIndex();

#endif // PID_CONTROLLER_H

