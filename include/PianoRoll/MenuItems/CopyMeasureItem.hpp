#include "rack.hpp"

using namespace rack;

struct CopyMeasureItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(EventAction &e) override {
    module->patternData.copyMeasure(module->transport.currentPattern(), widget->currentMeasure);
    widget->state = MEASURELOADED;
  }
};
