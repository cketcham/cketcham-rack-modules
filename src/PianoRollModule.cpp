#include "GVerbWidget.hpp"
#include "window.hpp"
#include <tuple>

static const float VELOCITY_SENSITIVITY = 0.0015f;

struct PianoRollWidget;
struct PianoRollModule;

struct Note {
	int pitch;
	float velocity;
	bool retrigger;
	bool active;

	Note() : pitch(0), velocity(1.f), retrigger(false), active(false) {}
};

struct Measure {
	std::vector<Note> notes;
};

struct Pattern {
	std::string title;
	std::vector<Measure> measures;
	int numberOfMeasures;
	int beatsPerMeasure;
	int divisionsPerBeat;
	bool triplets;

	Pattern() : title(""), numberOfMeasures(0), beatsPerMeasure(4), divisionsPerBeat(4), triplets(false) { }
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

struct ModuleDragType {
	PianoRollWidget* widget;
	PianoRollModule* module;

	ModuleDragType(PianoRollWidget* widget, PianoRollModule* module) : widget(widget), module(module) {}
	virtual ~ModuleDragType() {}

	virtual void onDragMove(EventDragMove& e) = 0;
};

struct NotePaintDragging : public ModuleDragType {
	int lastDragBeatDiv;
	int lastDragPitch;

	NotePaintDragging(PianoRollWidget* widget, PianoRollModule* module) : ModuleDragType(widget, module) {}
	~NotePaintDragging() {};

	void onDragMove(EventDragMove& e) override;
};

struct VelocityDragging : public ModuleDragType {
	int measure;
	int division;

	VelocityDragging(PianoRollWidget* widget, PianoRollModule* module, int measure, int division) : ModuleDragType(widget, module), measure(measure), division(division) {
		windowCursorLock();
	}

	~VelocityDragging() {
		windowCursorUnlock();
	}

	void onDragMove(EventDragMove& e) override;
};

struct StandardModuleDragging : public ModuleDragType {
	StandardModuleDragging(PianoRollWidget* widget, PianoRollModule* module) : ModuleDragType(widget, module) {}
	~StandardModuleDragging() {}

	void onDragMove(EventDragMove& e) override;
};


struct PianoRollModule : Module {
	enum ParamIds {
		RUN_BUTTON_PARAM,
		RESET_BUTTON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RUN_INPUT,
		RESET_INPUT,
		PATTERN_INPUT,
		RECORD_INPUT,
		VOCT_INPUT,
		GATE_INPUT,
		RETRIGGER_INPUT,
		VELOCITY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		RUN_OUTPUT,
		RESET_OUTPUT,
		PATTERN_OUTPUT,
		RECORD_OUTPUT,
		VOCT_OUTPUT,
		GATE_OUTPUT,
		RETRIGGER_OUTPUT,
		VELOCITY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int patterns = 64;
	int currentPattern = 0;
	std::vector<Pattern> patternData;

	PianoRollModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		patternData.resize(1);
	}
	void step() override;

	void setBeatsPerMeasure(int value) {
		patternData[currentPattern].beatsPerMeasure = value;
	}
	void setDivisionsPerBeat(int value) {
		patternData[currentPattern].divisionsPerBeat = value;
	}
	void setTriplets(bool value) {
		patternData[currentPattern].triplets = value;
	}
	void setMeasures(int value) {
		patternData[currentPattern].numberOfMeasures = value;
	}
	void setPattern(int pattern) {
		if (pattern < 1 || pattern > 64) {
			return;
		}

		currentPattern = pattern;
		if ((int)patternData.size() <= pattern) {
			patternData.resize(pattern + 1);
		}
	}

	void adjustVelocity(int measure, int note, float delta) {
		if ((int)patternData[currentPattern].measures.size() <= measure) {
			return;
		}

		if ((int)patternData[currentPattern].measures[measure].notes.size() <= note) {
			return;
		}

		// find first note in the current group
		int pitch = patternData[currentPattern].measures[measure].notes[note].pitch;
		while (patternData[currentPattern].measures[measure].notes[note-1].active && patternData[currentPattern].measures[measure].notes[note-1].pitch == pitch) {
			if (patternData[currentPattern].measures[measure].notes[note].retrigger) {
				break;
			}

			note--;
		}

		// Adjust the velocity of that note
		float velocity = clamp(patternData[currentPattern].measures[measure].notes[note].velocity + delta, 0.f, 1.f);

		// Apply new velocity to all notes in the group
		while (patternData[currentPattern].measures[measure].notes[note].active && patternData[currentPattern].measures[measure].notes[note].pitch == pitch) {
			patternData[currentPattern].measures[measure].notes[note].velocity = velocity;
			note++;

			if (patternData[currentPattern].measures[measure].notes[note].retrigger) {
				break;
			}
		}
	}

	void toggleCell(int measure, int note, int pitch) {
		if ((int)patternData[currentPattern].measures.size() <= measure) {
			patternData[currentPattern].measures.resize(measure + 1);
		}

		if ((int)patternData[currentPattern].measures[measure].notes.size() <= note) {
			patternData[currentPattern].measures[measure].notes.resize(note + 1);
		}

		if (!patternData[currentPattern].measures[measure].notes[note].active) {
			patternData[currentPattern].measures[measure].notes[note].active = true;
		} else if (patternData[currentPattern].measures[measure].notes[note].pitch == pitch) {
			patternData[currentPattern].measures[measure].notes[note].active = false;
			patternData[currentPattern].measures[measure].notes[note].retrigger = false;
		}

		patternData[currentPattern].measures[measure].notes[note].pitch = pitch;
		// match the velocity of the new note to any other notes in the group
		adjustVelocity(measure, note, 0.f);
	}

	void toggleCellRetrigger(int measure, int note, int pitch) {
		if ((int)patternData[currentPattern].measures.size() <= measure) {
			return;
		}

		if ((int)patternData[currentPattern].measures[measure].notes.size() <= note) {
			return;
		}

		if (!patternData[currentPattern].measures[measure].notes[note].active) {
			return;
		}

		if (patternData[currentPattern].measures[measure].notes[note].pitch != pitch) {
			return;
		}

		patternData[currentPattern].measures[measure].notes[note].retrigger = !patternData[currentPattern].measures[measure].notes[note].retrigger;
		// match the velocity of the new note to any other notes in the group
		adjustVelocity(measure, note, 0.f);
	}

};

void PianoRollModule::step() {
}



struct PianoRollWidget : ModuleWidget {
	PianoRollModule* module;
	TextField *patternNameField;
	TextField *patternInfoField;

	int octaves = 1;
	int currentOctave = 4;
	int currentMeasure = 0;
	float topMargins = 15;

	ModuleDragType *currentDragType = NULL;

	PianoRollWidget(PianoRollModule *module) : ModuleWidget(module) {
		this->module = (PianoRollModule*)module;
		setPanel(SVG::load(assetPlugin(plugin, "res/PianoRoll.svg")));

		addParam(ParamWidget::create<PB61303>(Vec(560.000, 380.f-185.839-28), module, PianoRollModule::RUN_BUTTON_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<PB61303>(Vec(560.000, 380.f-154.693-28), module, PianoRollModule::RESET_BUTTON_PARAM, 0.0, 1.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(50.114, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::CLOCK_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(85.642, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RUN_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(121.170, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RESET_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(156.697, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::PATTERN_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(192.224, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RECORD_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(456.921, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::VOCT_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(492.448, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::GATE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(527.976, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RETRIGGER_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(563.503, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::VELOCITY_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(50.114, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::CLOCK_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(85.642, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RUN_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(121.170, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RESET_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(156.697, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::PATTERN_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(192.224, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RECORD_OUTPUT));

		addOutput(Port::create<PJ301MPort>(Vec(456.921, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::VOCT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(492.448, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::GATE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(527.976, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RETRIGGER_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(563.503, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::VELOCITY_OUTPUT));

		patternNameField = Widget::create<LedDisplayTextField>(Vec(505.257, 380.f-224.259-125.586));
		patternNameField->box.size = Vec(86.863, 35);
		patternNameField->multiline = true;
		((LedDisplayTextField*)patternNameField)->color = COLOR_WHITE;
		addChild(patternNameField);

		patternInfoField = Widget::create<LedDisplayTextField>(Vec(505.257, 380.f-224.259-125.586+35));
		patternInfoField->box.size = Vec(86.863, 95.586);
		patternInfoField->multiline = true;
		((LedDisplayTextField*)patternInfoField)->color = COLOR_WHITE;
		addChild(patternInfoField);

	}

	struct OctavesItem : MenuItem {
		char buffer[100];
		PianoRollWidget* module;
		int value;
		OctavesItem(PianoRollWidget* module, int value) {
			this->module = module;
			this->value = value;

			snprintf(buffer, 10, "%d", value);
			text = buffer;
			if (value == module->octaves) {
				rightText = "✓";
			}
		}
		void onAction(EventAction &e) override {
			module->octaves = value;
		}
	};

	struct MeasuresItem : MenuItem {
		char buffer[100];
		PianoRollModule* module;
		int value;
		MeasuresItem(PianoRollModule* module, int value) {
			this->module = module;
			this->value = value;

			snprintf(buffer, 10, "%d", value);
			text = buffer;
			if (value == module->patternData[module->currentPattern].numberOfMeasures) {
				rightText = "✓";
			}
		}
		void onAction(EventAction &e) override {
			module->setMeasures(value);
		}
	};

	struct BeatsPerMeasureItem : MenuItem {
		char buffer[100];
		PianoRollModule* module;
		int value;
		BeatsPerMeasureItem(PianoRollModule* module, int value) {
			this->module = module;
			this->value = value;

			snprintf(buffer, 10, "%d", value);
			text = buffer;
			if (value == module->patternData[module->currentPattern].beatsPerMeasure) {
				rightText = "✓";
			}
		}
		void onAction(EventAction &e) override {
			module->setBeatsPerMeasure(value);
		}
	};

	struct DivisionsPerBeatItem : MenuItem {
		char buffer[100];
		PianoRollModule* module;
		int value;
		DivisionsPerBeatItem(PianoRollModule* module, int value) {
			this->module = module;
			this->value = value;

			snprintf(buffer, 10, "%d", value);
			text = buffer;
			if (value == module->patternData[module->currentPattern].divisionsPerBeat) {
				rightText = "✓";
			}
		}
		void onAction(EventAction &e) override {
			module->setDivisionsPerBeat(value);
		}
	};

	struct TripletsItem : MenuItem {
		char buffer[100];
		PianoRollModule* module;
		TripletsItem(PianoRollModule* module) {
			this->module = module;

			text = "Triplets";
			if (module->patternData[module->currentPattern].triplets) {
				rightText = "✓";
			}
		}
		void onAction(EventAction &e) override {
			module->setTriplets(!module->patternData[module->currentPattern].triplets);
		}
	};

	void appendContextMenu(Menu* menu) override {
		menu->addChild(MenuLabel::create(""));
		menu->addChild(MenuLabel::create("Octaves"));
			menu->addChild(new OctavesItem(this, 1));
			menu->addChild(new OctavesItem(this, 2));
		menu->addChild(MenuLabel::create(""));
		menu->addChild(MenuLabel::create("Measures"));
			menu->addChild(new MeasuresItem(module, 1));
			menu->addChild(new MeasuresItem(module, 2));
			menu->addChild(new MeasuresItem(module, 4));
			menu->addChild(new MeasuresItem(module, 8));
			menu->addChild(new MeasuresItem(module, 16));
		menu->addChild(MenuLabel::create(""));
		menu->addChild(MenuLabel::create("Beats per measure"));
			menu->addChild(new BeatsPerMeasureItem(module, 2));
			menu->addChild(new BeatsPerMeasureItem(module, 3));
			menu->addChild(new BeatsPerMeasureItem(module, 4));
			menu->addChild(new BeatsPerMeasureItem(module, 5));
			menu->addChild(new BeatsPerMeasureItem(module, 6));
			menu->addChild(new BeatsPerMeasureItem(module, 7));
			menu->addChild(new BeatsPerMeasureItem(module, 8));
		menu->addChild(MenuLabel::create(""));
		menu->addChild(MenuLabel::create("Divisions per beat"));
			menu->addChild(new DivisionsPerBeatItem(module, 2));
			menu->addChild(new DivisionsPerBeatItem(module, 4));
			menu->addChild(new DivisionsPerBeatItem(module, 8));
		menu->addChild(MenuLabel::create(""));
		menu->addChild(new TripletsItem(module));
	}

	Rect getRollArea() {
		Rect roll;
		roll.pos.x = 15.f;
		roll.pos.y = 380-365.f;
		roll.size.x = 480.f;
		roll.size.y = 220.f;
		return roll;
	}

	std::vector<Key> getKeys(const Rect& keysArea, int octaves) {
		std::vector<Key> keys;

		int keyCount = (octaves * 12) + 1;
		float keyHeight = keysArea.size.y / keyCount;

		for (int i = 0; i < keyCount; i++) {
			int n = i % 12;
			keys.push_back(
				Key(
					Vec(keysArea.pos.x, (keysArea.pos.y + keysArea.size.y) - ( (1 + i) * keyHeight) ),
					Vec(keysArea.size.x, keyHeight ),
					n == 1 || n == 3 || n == 6 || n == 8 || n == 10,
					(i + 3) % 12,
					currentOctave + ((i+3) / 12)
				)
			);
		}

		return keys;
	}

	std::vector<BeatDiv> getBeatDivs(const Rect &roll, int beatsPerMeasure, int divisionsPerBeat, bool triplets) {
		std::vector<BeatDiv> beatDivs;

		int totalDivisions = beatsPerMeasure * divisionsPerBeat * (triplets ? 1.5 : 1);
		divisionsPerBeat *= (triplets ? 1.5 : 1);

		float divisionWidth = roll.size.x / totalDivisions;

		float top = roll.pos.y + topMargins;

		for (int i = 0; i < totalDivisions; i ++) {
			float x = roll.pos.x + (i * divisionWidth);

			BeatDiv beatDiv;
			beatDiv.pos.x = x;
			beatDiv.size.x = divisionWidth;
			beatDiv.pos.y = top;
			beatDiv.size.y = roll.size.y - (2 * topMargins);

			beatDiv.num = i;
			beatDiv.beat = (i % divisionsPerBeat == 0);
			beatDiv.triplet = (triplets && i % 3 == 0);

			beatDivs.push_back(beatDiv);
		}

		return beatDivs;
	}

	Rect reserveKeysArea(Rect& roll) {
		Rect keysArea;
		keysArea.pos.x = roll.pos.x;
		keysArea.pos.y = roll.pos.y + topMargins;
		keysArea.size.x = 25.f;
		keysArea.size.y = roll.size.y - (2*topMargins);

		roll.pos.x = keysArea.pos.x + keysArea.size.x;
		roll.size.x = roll.size.x - keysArea.size.x;

		return keysArea;
	}

	std::tuple<bool, BeatDiv, Key> findCell(Vec pos) {
		Rect roll = getRollArea();
		Rect keysArea = reserveKeysArea(roll);

		if (!roll.contains(pos)) {
			return std::make_tuple(false, BeatDiv(), Key());
		}

		auto keys = getKeys(keysArea, this->octaves);
		bool keyFound = false;
		Key cellKey;

		for (auto const& key: keys) {
			if (Rect(Vec(key.pos.x + key.size.x, key.pos.y), Vec(roll.size.x, key.size.y)).contains(pos)) {
				cellKey = key;
				keyFound = true;
				break;
			}
		}

		auto beatDivs = getBeatDivs(roll, module->patternData[module->currentPattern].beatsPerMeasure, module->patternData[module->currentPattern].divisionsPerBeat, module->patternData[module->currentPattern].triplets);
		bool beatDivFound = false;
		BeatDiv cellBeatDiv;

		for (auto const& beatDiv: beatDivs) {
			if (Rect(beatDiv.pos, beatDiv.size).contains(pos)) {
				cellBeatDiv = beatDiv;
				beatDivFound = true;
				break;
			}
		}

	  return std::make_tuple(keyFound && beatDivFound, cellBeatDiv, cellKey);
	}


	void drawKeys(NVGcontext *ctx, const std::vector<Key> &keys) {
		int n = 0;
		char buffer[100];
		int displayOctave = currentOctave;

		for (auto const& key: keys) {
			nvgBeginPath(ctx);
			nvgStrokeWidth(ctx, 0.5f);
			nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0));

			if (key.sharp) {
				nvgFillColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0));
			} else {
				nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 1.0));
			}
			nvgRect(ctx, key.pos.x, key.pos.y, key.size.x, key.size.y);

			nvgStroke(ctx);
			nvgFill(ctx);

			if (n == 0) {
				nvgBeginPath(ctx);
				snprintf(buffer, 10, "C%d", displayOctave);
				nvgFontSize(ctx,key.size.y);
				nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0));
				nvgFillColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.0));
				nvgText(ctx, key.pos.x + 4, key.pos.y + key.size.y - 2, buffer, NULL);
				nvgStroke(ctx);
				nvgFill(ctx);
			}

			n = (n + 1) % 12;
			if (n == 0) {
				displayOctave += 1;
			}
		}
	}

	void drawSwimLanes(NVGcontext *ctx, const Rect &roll, const std::vector<Key> &keys) {

		for (auto const& key: keys) {
			nvgBeginPath(ctx);
			nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5f));
			nvgStrokeWidth(ctx, 0.5f);
			nvgMoveTo(ctx, roll.pos.x, key.pos.y);
			nvgLineTo(ctx, roll.pos.x + roll.size.x, key.pos.y);
			nvgStroke(ctx);
		}

		nvgBeginPath(ctx);
		nvgStrokeWidth(ctx, 1.f);
		nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0f));
		nvgMoveTo(ctx, roll.pos.x, keys.back().pos.y);
		nvgLineTo(ctx, roll.pos.x + roll.size.x, keys.back().pos.y);
		nvgStroke(ctx);

		nvgBeginPath(ctx);
		nvgStrokeWidth(ctx, 1.f);
		nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0f));
		nvgMoveTo(ctx, roll.pos.x, keys[0].pos.y + keys[0].size.y);
		nvgLineTo(ctx, roll.pos.x + roll.size.x, keys[0].pos.y + keys[0].size.y);
		nvgStroke(ctx);
	}

	void drawBeats(NVGcontext *ctx, const std::vector<BeatDiv> &beatDivs) {
		bool first = true;
		for (const auto &beatDiv : beatDivs) {

			nvgBeginPath(ctx);

			if (beatDiv.beat && !first) {
				nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0));
				nvgStrokeWidth(ctx, 1.0f);
			} else if (beatDiv.triplet) {
				nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.0));
				nvgStrokeWidth(ctx, 0.5f);
			} else {
				nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5));
				nvgStrokeWidth(ctx, 0.5f);
			}

			nvgMoveTo(ctx, beatDiv.pos.x, beatDiv.pos.y);
			nvgLineTo(ctx, beatDiv.pos.x, beatDiv.pos.y + beatDiv.size.y);

			nvgStroke(ctx);

			first = false;
		}
	}

	void drawNotes(NVGcontext *ctx, const std::vector<Key> &keys, const std::vector<BeatDiv> &beatDivs, const std::vector<Measure> &measures) {
		int lowPitch = keys.front().num + (keys.front().oct * 12);
		int highPitch = keys.back().num + (keys.back().oct * 12);
		if ((int)measures.size() <= currentMeasure) { return; }

		for (const auto &beatDiv : beatDivs) {
			if ((int)measures[currentMeasure].notes.size() <= beatDiv.num) { break; }
			if (   measures[currentMeasure].notes[beatDiv.num].active == false
					|| measures[currentMeasure].notes[beatDiv.num].pitch < lowPitch
					|| measures[currentMeasure].notes[beatDiv.num].pitch > highPitch) { 
				continue;
			}

			for (auto const& key: keys) {
				if (key.num + (key.oct * 12) == measures[currentMeasure].notes[beatDiv.num].pitch) {

					float velocitySize = (measures[currentMeasure].notes[beatDiv.num].velocity * key.size.y * 0.9f) + (key.size.y * 0.1f);

					nvgBeginPath(ctx);
					nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.25f));
					nvgStrokeWidth(ctx, 0.5f);
					nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.25f));
					nvgRect(ctx, beatDiv.pos.x, key.pos.y, beatDiv.size.x, (key.size.y - velocitySize));
					nvgStroke(ctx);
					nvgFill(ctx);

					nvgBeginPath(ctx);
					nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 0.5f));
					nvgStrokeWidth(ctx, 0.5f);
					nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
					nvgRect(ctx, beatDiv.pos.x, key.pos.y + (key.size.y - velocitySize), beatDiv.size.x, velocitySize);
					nvgStroke(ctx);
					nvgFill(ctx);


					if (measures[currentMeasure].notes[beatDiv.num].retrigger) {
						nvgBeginPath(ctx);

						nvgStrokeWidth(ctx, 0.f);
						nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 1.f));

						nvgRect(ctx, beatDiv.pos.x, key.pos.y, beatDiv.size.x / 4.f, key.size.y);
						nvgStroke(ctx);
						nvgFill(ctx);
					}

					break;
				}
			}

		}
	}

	// Event Handlers

	void draw(NVGcontext* ctx) override {
		ModuleWidget::draw(ctx);

		Rect roll = getRollArea();

		Rect keysArea = reserveKeysArea(roll);
		auto keys = getKeys(keysArea, this->octaves);
		drawKeys(ctx, keys);
		drawSwimLanes(ctx, roll, keys);
		auto beatDivs = getBeatDivs(roll, module->patternData[module->currentPattern].beatsPerMeasure, module->patternData[module->currentPattern].divisionsPerBeat, module->patternData[module->currentPattern].triplets);
		drawBeats(ctx, beatDivs);
		drawNotes(ctx, keys, beatDivs, module->patternData[module->currentPattern].measures);
	}

	void onMouseDown(EventMouseDown& e) override {
		if (e.button == 1) {
			std::tuple<bool, BeatDiv, Key> cell = findCell(e.pos);
			if (!std::get<0>(cell)) { ModuleWidget::onMouseDown(e); return; }

			e.consumed = true;
			e.target = this;

			int beatDiv = std::get<1>(cell).num;
			int pitch = std::get<2>(cell).pitch();
			module->toggleCellRetrigger(currentMeasure, beatDiv, pitch);
		} else {
			ModuleWidget::onMouseDown(e);
		}
	}

	void onDragStart(EventDragStart& e) override {
		Vec pos = gRackWidget->lastMousePos.minus(box.pos);
		std::tuple<bool, BeatDiv, Key> cell = findCell(pos);

		if (std::get<0>(cell) && windowIsShiftPressed()) {
			currentDragType = new VelocityDragging(this, module, this->currentMeasure, std::get<1>(cell).num);
		} else if (std::get<0>(cell)) {
			currentDragType = new NotePaintDragging(this, module);
		} else {
			currentDragType = new StandardModuleDragging(this, module);
		}

		ModuleWidget::onDragStart(e);
	}

	void baseDragMove(EventDragMove& e) {
		ModuleWidget::onDragMove(e);
	}

	void onDragMove(EventDragMove& e) override {
		currentDragType->onDragMove(e);
	}

	void onDragEnd(EventDragEnd& e) override {
		delete currentDragType;
		currentDragType = NULL;

		ModuleWidget::onDragEnd(e);
	}



};


void NotePaintDragging::onDragMove(EventDragMove& e) {
	Vec pos = gRackWidget->lastMousePos.minus(widget->box.pos);

	std::tuple<bool, BeatDiv, Key> cell = widget->findCell(pos);
	if (!std::get<0>(cell)) {
		return;
	}

	int beatDiv = std::get<1>(cell).num;
	int pitch = std::get<2>(cell).pitch();

	if (lastDragBeatDiv != beatDiv || lastDragPitch != pitch) {
		lastDragBeatDiv = beatDiv;
		lastDragPitch = pitch;
		module->toggleCell(widget->currentMeasure, beatDiv, pitch);
	};
}

void VelocityDragging::onDragMove(EventDragMove& e) {
	float speed = 1.f;
	float range = 1.f;
	float delta = VELOCITY_SENSITIVITY * -e.mouseRel.y * speed * range;
	if (windowIsModPressed()) {
		delta /= 16.f;
	}

	module->adjustVelocity(measure, division, delta);
}

void StandardModuleDragging::onDragMove(EventDragMove& e) {
	widget->baseDragMove(e);
}


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelPianoRollModule = Model::create<PianoRollModule, PianoRollWidget>("rcm", "rcm-pianoroll", "Piano Roll", SEQUENCER_TAG);
