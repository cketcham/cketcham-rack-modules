#include "rack.hpp"

using namespace rack;

struct CopyPatternItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  int type;
  void onAction(EventAction &e) override {
    module->patternData.copyPattern(module->transport.currentPattern());
    widget->state = PATTERNLOADED;
  }
};
