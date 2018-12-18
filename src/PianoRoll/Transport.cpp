#include "rack.hpp"
#include "../../include/PianoRoll/Transport.hpp"
#include "../../include/PianoRoll/PatternData.hpp"

using namespace rack;

Transport::Transport(PatternData* patternData) {
  this->patternData = patternData;

  locked = false;
  running = true;
  recording = false;
  pendingRecording = false;
}

int Transport::currentPattern() {
  return pattern;
}

int Transport::currentMeasure() {
  return stepInPattern / patternData->getStepsPerMeasure(pattern);
}

int Transport::currentStepInMeasure() {
  return stepInPattern % patternData->getStepsPerMeasure(pattern);
}

int Transport::currentStepInPattern() {
  return stepInPattern;
}

bool Transport::isLastStepOfPattern() {
  return stepInPattern == (patternData->getStepsInPattern(pattern) - 1);
}

void Transport::setPattern(int pattern) {
  pattern = clamp(pattern, 0, 63);
  if (pattern != this->pattern) {
    this->pattern = pattern;
    this->stepInPattern = -1;
  }
}

void Transport::setMeasure(int measure) {
  stepInPattern = (patternData->getStepsPerMeasure(pattern) * measure) + (patternData->getStepsPerMeasure(pattern) - 1);
}

void Transport::setStepInMeasure(int stepInMeasure) {
  stepInPattern = (currentMeasure() * patternData->getStepsPerMeasure(pattern)) + (stepInMeasure % patternData->getStepsPerMeasure(pattern));
}

void Transport::setStepInPattern(int stepInPattern) {
  this->stepInPattern = stepInPattern;
}

void Transport::advancePattern(int offset) {
  setPattern(pattern + offset);
}

void Transport::advanceStep() {
  if (!running) {
    return;
  }

  int firstStepInLoop = 0;
  int measure = currentMeasure();
  stepInPattern = (stepInPattern + 1) % patternData->getStepsInPattern(pattern);
  int newMeasure = currentMeasure();

  if (locked) {
    firstStepInLoop = measure * patternData->getStepsPerMeasure(pattern);

    if (measure != newMeasure) {
      // We moved on to another measure in the pattern, adjust back to the start of our locked measure
      stepInPattern = firstStepInLoop;
    }
  }

  if (recording && stepInPattern == firstStepInLoop) {
    pendingRecording = false;
    recording = false;
  }

  if (pendingRecording && stepInPattern == firstStepInLoop) {
    pendingRecording = false;
    recording = true;
  }
}

void Transport::lockMeasure() {
  locked = true;
}

void Transport::unlockMeasure() {
  locked = false;
}

bool Transport::isLocked() {
  return locked;
}

void Transport::toggleRun() {
  running = !running;
}

void Transport::setRun(bool running) {
  this->running = running;
}

bool Transport::isRunning() {
  return running;
}

void Transport::toggleRecording() {
  if (!recording) {
    pendingRecording = !pendingRecording;
  } else {
    recording = false;
    pendingRecording = false;
  }
}

bool Transport::isRecording() {
  return recording;
}

bool Transport::isPendingRecording() {
  return pendingRecording;
}

void Transport::reset() {
  pattern = 0;
  stepInPattern = -1;
}
