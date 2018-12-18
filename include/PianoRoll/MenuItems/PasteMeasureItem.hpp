#include "rack.hpp"

using namespace rack;

struct PasteMeasureItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(EventAction &e) override {
    module->patternData.pasteMeasure(module->transport.currentPattern(), widget->currentMeasure);
  }
};
