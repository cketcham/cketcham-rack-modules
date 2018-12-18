#include "rack.hpp"

using namespace rack;

struct PastePatternItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(EventAction &e) override {
    module->patternData.pastePattern(module->transport.currentPattern());
  }
};
