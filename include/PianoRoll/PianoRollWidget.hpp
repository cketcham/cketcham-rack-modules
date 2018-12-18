#include <tuple>
#include <limits>

#include "rack.hpp"

using namespace rack;

struct PianoRollModule;
struct PianoRollWidget;
struct ModuleDragType;

enum CopyPasteState {
	COPYREADY,
	PATTERNLOADED,
	MEASURELOADED
};

struct Key {
	Vec pos;
	Vec size;
	bool sharp;
	int num;
	int oct;

	Key() : pos(Vec(0,0)), size(Vec(0,0)), sharp(false), num(0), oct(0) {}
	Key(Vec p, Vec s, bool sh, int n, int o) : pos(p), size(s), sharp(sh), num(n), oct(o) {}

	int pitch() {
		return num + (12 * oct);
	}
};

struct BeatDiv {
	Vec pos;
	Vec size;
	int num;
	bool beat;
	bool triplet;

	BeatDiv() : pos(Vec(0,0)), size(Vec(0,0)), num(0), beat(false), triplet(false) {}
};

struct PianoRollWidget : ModuleWidget {
	PianoRollModule* module;
	CopyPasteState state;
	float backgroundHue = 0.5f;
	float backgroundSaturation = 1.f;
	float backgroundLuminosity = 0.25f;

	int notesToShow = 18;
	int lowestDisplayNote = 4 * 12;
	int currentMeasure = 0;
	float topMargins = 15;
	int lastDrawnStep = -1;
	float displayVelocityHigh = -1;
	float displayVelocityLow = -1;

	double measureLockPressTime = 0.f;

	ModuleDragType *currentDragType = NULL;

	PianoRollWidget(PianoRollModule *module);

	Rect getRollArea();
	std::vector<Key> getKeys(const Rect& keysArea);
	std::vector<BeatDiv> getBeatDivs(const Rect &roll);
	Rect reserveKeysArea(Rect& roll);
	std::tuple<bool, int> findMeasure(Vec pos);
	std::tuple<bool, bool> findOctaveSwitch(Vec pos);
	std::tuple<bool, BeatDiv, Key> findCell(Vec pos);

	void drawKeys(NVGcontext *ctx, const std::vector<Key> &keys);
	void drawSwimLanes(NVGcontext *ctx, const Rect &roll, const std::vector<Key> &keys);
	void drawBeats(NVGcontext *ctx, const std::vector<BeatDiv> &beatDivs);
	void drawNotes(NVGcontext *ctx, const std::vector<Key> &keys, const std::vector<BeatDiv> &beatDivs);
	void drawMeasures(NVGcontext *ctx);
	void drawPlayPosition(NVGcontext *ctx);
	void drawVelocityInfo(NVGcontext *ctx);
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
