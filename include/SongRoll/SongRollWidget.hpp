#include <tuple>
#include <limits>

#include "rack.hpp"

using namespace rack;


namespace SongRoll {

  struct ModuleDragType;
  struct SongRollModule;

  struct SongRollWidget : ModuleWidget {
    SongRollModule* module;
    float backgroundHue = 0.25f;
    float backgroundSaturation = 1.f;
    float backgroundLuminosity = 0.25f;

    ModuleDragType *currentDragType = NULL;

    SongRollWidget(SongRollModule *module);

    Rect getRollArea();

    void drawBackgroundColour(NVGcontext* ctx);

    // Event Handlers

    void appendContextMenu(Menu* menu) override;
    void draw(NVGcontext* ctx) override;
    void onMouseDown(EventMouseDown& e) override;
    void onDragStart(EventDragStart& e) override;
    void baseDragMove(EventDragMove& e);
    void onDragMove(EventDragMove& e) override;
    void onDragEnd(EventDragEnd& e) override;

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

  };
}
