#include "rack.hpp"

using namespace rack;

namespace SongRoll {

  struct SongRollData;

  class PatternControlWidget : public Widget {
  public:
    int pattern=0;

    PatternControlWidget();

    void draw(NVGcontext* ctx) override;
    void onMouseDown(EventMouseDown& e) override;
  };

}
