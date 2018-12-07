#include "GVerbWidget.hpp"
#include "window.hpp"
#include "dsp/digital.hpp"
#include <tuple>

static const float VELOCITY_SENSITIVITY = 0.0015f;

struct PianoRollWidget;
struct PianoRollModule;

struct PatternWidget : LedDisplay {
	/** Not owned */
	PianoRollWidget *widget = NULL;
	PianoRollModule *module = NULL;

	LedDisplayChoice *patternChoice;
	LedDisplaySeparator *patternSeparator;
	LedDisplayChoice *octaveChoice;
	LedDisplaySeparator *octaveSeparator;
	LedDisplayChoice *measuresChoice;
	LedDisplaySeparator *measuresSeparator;
	LedDisplayChoice *beatsPerMeasureChoice;
	LedDisplaySeparator *beatsPerMeasureSeparator;
	LedDisplayChoice *divisionsPerBeatChoice;
	LedDisplaySeparator *divisionsPerBeatSeparator;
	LedDisplayChoice *tripletsChoice;
	PatternWidget();
	void step() override;
};


struct Note {
	int pitch;
	float velocity;
	bool retrigger;
	bool active;

	Note() : pitch(0), velocity(1.f), retrigger(false), active(false) {}
	Note& operator=(const Note& other) {
		pitch = other.pitch;
		velocity = other.velocity;
		retrigger = other.retrigger;
		active = other.active;
		return *this;
	}
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

	Pattern() : title(""), numberOfMeasures(1), beatsPerMeasure(4), divisionsPerBeat(4), triplets(false) { }
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
		END_OF_PATTERN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int patterns = 64;
	int currentPattern = 0;
	std::vector<Pattern> patternData;
	SchmittTrigger clockIn;
	SchmittTrigger resetIn;
	int currentStep = -1;
	PulseGenerator retriggerOut;
	PulseGenerator eopOut;

	PianoRollModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		patternData.resize(64);
	}

	void step() override;

	void onReset() override {
		currentPattern = 0;
		currentStep = -1;
		patternData.resize(0);
		patternData.resize(64);
	}

	json_t *toJson() override {
		json_t *rootJ = Module::toJson();
		if (rootJ == NULL) {
				rootJ = json_object();
		}

		json_t *patternsJ = json_array();
		for (const auto& pattern : patternData) {
			json_t *patternJ = json_object();
			json_object_set_new(patternJ, "title", json_string(pattern.title.c_str()));
			json_object_set_new(patternJ, "numberOfMeasures", json_integer(pattern.numberOfMeasures));
			json_object_set_new(patternJ, "beatsPerMeasure", json_integer(pattern.beatsPerMeasure));
			json_object_set_new(patternJ, "divisionsPerBeat", json_integer(pattern.divisionsPerBeat));
			json_object_set_new(patternJ, "triplets", json_boolean(pattern.triplets));

			json_t *measuresJ = json_array();
			for (const auto& measure : pattern.measures) {
				// struct Measure {
				// 	std::vector<Note> notes;
				json_t *measureJ = json_object();
				json_t *notesJ = json_array();

				for (const auto& note : measure.notes) {
					json_t *noteJ = json_object();

					json_object_set_new(noteJ, "pitch", json_integer(note.pitch));
					json_object_set_new(noteJ, "velocity", json_real(note.velocity));
					json_object_set_new(noteJ, "retrigger", json_boolean(note.retrigger));
					json_object_set_new(noteJ, "active", json_boolean(note.active));
		
					json_array_append_new(notesJ, noteJ);
				}

				json_object_set_new(measureJ, "notes", notesJ);
				json_array_append_new(measuresJ, measureJ);
			}

			json_object_set_new(patternJ, "measures", measuresJ);
			json_array_append_new(patternsJ, patternJ);
		}

		json_object_set_new(rootJ, "patterns", patternsJ);
		json_object_set_new(rootJ, "currentPattern", json_integer(currentPattern));
		json_object_set_new(rootJ, "currentStep", json_integer(currentStep));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		Module::fromJson(rootJ);

		json_t *currentStepJ = json_object_get(rootJ, "currentStep");
		if (currentStepJ) {
			currentStep = json_integer_value(currentStepJ);
		}

		json_t *currentPatternJ = json_object_get(rootJ, "currentPattern");
		if (currentPatternJ) {
			currentPattern = json_integer_value(currentPatternJ);
		}

		json_t *patternsJ = json_object_get(rootJ, "patterns");
		if (patternsJ) {
			patternData.resize(0);

			size_t i;
			json_t *patternJ;
			json_array_foreach(patternsJ, i, patternJ) {
				Pattern pattern;

				json_t *titleJ = json_object_get(patternJ, "title");
				if (titleJ) {
					pattern.title = json_string_value(titleJ);
				}

				json_t *numberOfMeasuresJ = json_object_get(patternJ, "numberOfMeasures");
				if (numberOfMeasuresJ) {
					pattern.numberOfMeasures = json_integer_value(numberOfMeasuresJ);
				}

				json_t *beatsPerMeasureJ = json_object_get(patternJ, "beatsPerMeasure");
				if (beatsPerMeasureJ) {
					pattern.beatsPerMeasure = json_integer_value(beatsPerMeasureJ);
				}

				json_t *divisionsPerBeatJ = json_object_get(patternJ, "divisionsPerBeat");
				if (divisionsPerBeatJ) {
					pattern.divisionsPerBeat = json_integer_value(divisionsPerBeatJ);
				}

				json_t *tripletsJ = json_object_get(patternJ, "triplets");
				if (tripletsJ) {
					pattern.triplets = json_boolean_value(tripletsJ);
				}

				json_t *measuresJ = json_object_get(patternJ, "measures");
				if (measuresJ) {
					pattern.measures.resize(0);

					size_t j;
					json_t *measureJ;
					json_array_foreach(measuresJ, j, measureJ) {
						Measure measure;

						json_t *notesJ = json_object_get(measureJ, "notes");
						if (notesJ) {
							size_t k;
							json_t *noteJ;
							json_array_foreach(notesJ, k, noteJ) {
								Note note;

								json_t *pitchJ = json_object_get(noteJ, "pitch");
								if (pitchJ) {
									note.pitch = json_integer_value(pitchJ);
								}

								json_t *velocityJ = json_object_get(noteJ, "velocity");
								if (velocityJ) {
									note.velocity = json_number_value(velocityJ);
								}

								json_t *retriggerJ = json_object_get(noteJ, "retrigger");
								if (retriggerJ) {
									note.retrigger = json_boolean_value(retriggerJ);
								}

								json_t *activeJ = json_object_get(noteJ, "active");
								if (activeJ) {
									note.active = json_boolean_value(activeJ);
								}

								measure.notes.push_back(note);
							}
						}

						pattern.measures.push_back(measure);
					}
				}

				patternData.push_back(pattern);
			}
		}

	}

	void setBeatsPerMeasure(int value) {
		patternData[currentPattern].beatsPerMeasure = value;
	}

	void reassignNotes(int fromDivisions, int toDivisions) {
		float scale = (float)toDivisions / (float)fromDivisions;
		int smear = rack::max(1, (int)floor(scale));
		for (auto& measure: patternData[currentPattern].measures) {

			std::vector<Note> scratch;
			scratch.resize(toDivisions);

			for (int i = 0; i < fromDivisions; i++) {
				int newPos = floor(i * scale);
				if ((int)measure.notes.size() <= i) { 
					continue;
				}
				if (measure.notes[i].active) {
					if ((int)measure.notes.size() > newPos) {
						measure.notes[i].retrigger |= measure.notes[i].active && measure.notes[i].retrigger;
					}

					// Copy note to new location
					// Keep it's gate length by smearing the note across multiple locations if necessary
					for (int n = 0; n < smear; n++) {
						scratch[newPos + n] = measure.notes[i];
						
						if (n > 0) {
							// don't retrigger smeared notes
							scratch[newPos + n].retrigger = false;
						}
					}
				}
			}

			measure.notes.resize(toDivisions);
			for (int i = 0; i < toDivisions; i++) {
				measure.notes[i] = scratch[i];
			}
		}
	}

	void setDivisionsPerBeat(int value) {
		int oldBPM = getDivisionsPerMeasure();
		patternData[currentPattern].divisionsPerBeat = value;
		int newBPM = getDivisionsPerMeasure();

		reassignNotes(oldBPM, newBPM);
	}

	void setTriplets(bool value) {
		int oldBPM = getDivisionsPerMeasure();
		patternData[currentPattern].triplets = value;
		int newBPM = getDivisionsPerMeasure();

		reassignNotes(oldBPM, newBPM);
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
			patternData[currentPattern].measures[measure].notes[note].velocity = 0.75f;
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

	int getDivisionsPerMeasure() const {
		return patternData[currentPattern].beatsPerMeasure * patternData[currentPattern].divisionsPerBeat * (patternData[currentPattern].triplets ? 1.5 : 1);
	}
};

void PianoRollModule::step() {
	if (resetIn.process(inputs[RESET_INPUT].value)) {
		currentStep = -1;
		outputs[GATE_OUTPUT].value = 0.f;
	}

	if (inputs[PATTERN_INPUT].active) {
		currentPattern = floor(rescale(clamp(inputs[PATTERN_INPUT].value, 0.f, 10.f), 0, 10, 0, 63));
	}

	if (clockIn.process(inputs[CLOCK_INPUT].value)) {
		currentStep = (currentStep + 1) % (getDivisionsPerMeasure() * patternData[currentPattern].numberOfMeasures);
		if (currentStep == (getDivisionsPerMeasure() * patternData[currentPattern].numberOfMeasures) - 1) {
			eopOut.trigger(1e-3f);
		}

		int measure = currentStep / getDivisionsPerMeasure();
		int noteInMeasure = currentStep % getDivisionsPerMeasure();

		if ((int)patternData[currentPattern].measures.size() > measure 
				&& (int)patternData[currentPattern].measures[measure].notes.size() > noteInMeasure
				&& patternData[currentPattern].measures[measure].notes[noteInMeasure].active) {

			if (outputs[GATE_OUTPUT].value == 0.f || patternData[currentPattern].measures[measure].notes[noteInMeasure].retrigger) {
				retriggerOut.trigger(1e-3f);
			}

			outputs[GATE_OUTPUT].value = 10.f;
			outputs[VELOCITY_OUTPUT].value = patternData[currentPattern].measures[measure].notes[noteInMeasure].velocity * 10.f;
			float octave = patternData[currentPattern].measures[measure].notes[noteInMeasure].pitch / 12;
			float semitone = patternData[currentPattern].measures[measure].notes[noteInMeasure].pitch % 12;
			outputs[VOCT_OUTPUT].value = (octave-4.f) + ((1.f/12.f) * semitone);
		} else {
			outputs[GATE_OUTPUT].value = 0.f;
		}
	}

	outputs[RETRIGGER_OUTPUT].value = retriggerOut.process(engineGetSampleTime()) ? 10.f : 0.f;
	outputs[END_OF_PATTERN_OUTPUT].value = eopOut.process(engineGetSampleTime()) ? 10.f : 0.f;
}

struct PianoRollWidget : ModuleWidget {
	PianoRollModule* module;
	TextField *patternNameField;
	TextField *patternInfoField;

	int octaves = 1;
	int currentOctave = 4;
	int currentMeasure = 0;
	float topMargins = 15;
	int lastDrawnStep = -1;

	ModuleDragType *currentDragType = NULL;

	PianoRollWidget(PianoRollModule *module) : ModuleWidget(module) {
		this->module = (PianoRollModule*)module;
		setPanel(SVG::load(assetPlugin(plugin, "res/PianoRoll.svg")));

		addInput(Port::create<PJ301MPort>(Vec(50.114, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::CLOCK_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(85.642, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RESET_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(121.170, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::PATTERN_INPUT));
		//addInput(Port::create<PJ301MPort>(Vec(156.697, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RECORD_INPUT));

		// addInput(Port::create<PJ301MPort>(Vec(456.921, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::VOCT_INPUT));
		// addInput(Port::create<PJ301MPort>(Vec(492.448, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::GATE_INPUT));
		// addInput(Port::create<PJ301MPort>(Vec(527.976, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RETRIGGER_INPUT));
		// addInput(Port::create<PJ301MPort>(Vec(563.503, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::VELOCITY_INPUT));

		// addOutput(Port::create<PJ301MPort>(Vec(50.114, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::CLOCK_OUTPUT));
		// addOutput(Port::create<PJ301MPort>(Vec(85.642, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RUN_OUTPUT));
		// addOutput(Port::create<PJ301MPort>(Vec(121.170, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RESET_OUTPUT));
		// addOutput(Port::create<PJ301MPort>(Vec(156.697, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::PATTERN_OUTPUT));
		// addOutput(Port::create<PJ301MPort>(Vec(192.224, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RECORD_OUTPUT));

		addOutput(Port::create<PJ301MPort>(Vec(421.394, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::VOCT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(456.921, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::GATE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(492.448, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RETRIGGER_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(527.976, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::VELOCITY_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(563.503, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::END_OF_PATTERN_OUTPUT));

		PatternWidget* patternWidget = Widget::create<PatternWidget>(Vec(505.257, 380.f-224.259-125.586));
		patternWidget->box.size = Vec(86.863, 80);
		patternWidget->module = module;
		patternWidget->widget = this;
		addChild(patternWidget);
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

	void appendContextMenu(Menu* menu) override {
		menu->addChild(MenuLabel::create(""));
		menu->addChild(MenuLabel::create("Octaves"));
			menu->addChild(new OctavesItem(this, 1));
			menu->addChild(new OctavesItem(this, 2));
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
					i % 12,
					currentOctave + (i / 12)
				)
			);
		}

		return keys;
	}

	std::vector<BeatDiv> getBeatDivs(const Rect &roll, int beatsPerMeasure, int divisionsPerBeat, bool triplets) {
		std::vector<BeatDiv> beatDivs;

		int totalDivisions = module->getDivisionsPerMeasure();
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

	std::tuple<bool, int> findMeasure(Vec pos) {
		Rect roll = getRollArea();
		Rect keysArea = reserveKeysArea(roll);

		float widthPerMeasure = roll.size.x / module->patternData[module->currentPattern].numberOfMeasures;
		float boxHeight = topMargins * 0.75;

		for (int i = 0; i < module->patternData[module->currentPattern].numberOfMeasures; i++) {
			if (Rect(Vec(roll.pos.x + i * widthPerMeasure, roll.pos.y + roll.size.y - boxHeight), Vec(widthPerMeasure, boxHeight)).contains(pos)) {
				return std::make_tuple(true, i);
			}
		}

		return std::make_tuple(false, 0);
	}


	std::tuple<bool, bool> findOctaveSwitch(Vec pos) {
		Rect roll = getRollArea();
		Rect keysArea = reserveKeysArea(roll);

		bool octaveUp = Rect(Vec(keysArea.pos.x, roll.pos.y), Vec(keysArea.size.x, keysArea.pos.y)).contains(pos);
		bool octaveDown = Rect(Vec(keysArea.pos.x, keysArea.pos.y + keysArea.size.y), Vec(keysArea.size.x, keysArea.pos.y)).contains(pos);
		
		return std::make_tuple(octaveUp, octaveDown);
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

		Rect roll = getRollArea();
		reserveKeysArea(roll);

		for (const auto &beatDiv : beatDivs) {
			if ((int)measures[currentMeasure].notes.size() <= beatDiv.num) { break; }
			if (measures[currentMeasure].notes[beatDiv.num].active == false ) { continue; }
			if (measures[currentMeasure].notes[beatDiv.num].pitch < lowPitch) {
				nvgBeginPath(ctx);
				nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
				nvgStrokeWidth(ctx, 1.f);
				nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
				nvgRect(ctx, beatDiv.pos.x, roll.pos.y + roll.size.y - topMargins, beatDiv.size.x, 1);
				nvgStroke(ctx);
				nvgFill(ctx);
				continue;
			}
			if (measures[currentMeasure].notes[beatDiv.num].pitch > highPitch) {
				nvgBeginPath(ctx);
				nvgStrokeColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
				nvgStrokeWidth(ctx, 1.f);
				nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, 1.f));
				nvgRect(ctx, beatDiv.pos.x, roll.pos.y + topMargins -1, beatDiv.size.x, 1);
				nvgStroke(ctx);
				nvgFill(ctx);
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

	void drawMeasures(NVGcontext *ctx, int numberOfMeasures, int currentMeasure) {
		Rect roll = getRollArea();
		reserveKeysArea(roll);

		float widthPerMeasure = roll.size.x / numberOfMeasures;
		float boxHeight = topMargins * 0.75;

		for (int i = 0; i < numberOfMeasures; i++) {
			nvgBeginPath(ctx);
			nvgStrokeColor(ctx, nvgRGBAf(0.f, 0.f, 0.f, 1.f));
			nvgStrokeWidth(ctx, 1.f);
			nvgFillColor(ctx, nvgRGBAf(1.f, 0.9f, 0.3f, i == currentMeasure ? 1.f : 0.25f));
			nvgRect(ctx, roll.pos.x + i * widthPerMeasure, roll.pos.y + roll.size.y - boxHeight, widthPerMeasure, boxHeight);
			nvgStroke(ctx);
			nvgFill(ctx);
		}
	}

	void drawPlayPosition(NVGcontext *ctx, int currentStep, int numberOfMeasures, int divisionsPerBeat, int currentMeasure) {
		Rect roll = getRollArea();
		reserveKeysArea(roll);

		if (currentStep >= 0) {
			int divisionsPerMeasure = module->getDivisionsPerMeasure();
			int playingMeasure = currentStep / divisionsPerMeasure;
			int noteInMeasure = currentStep % divisionsPerMeasure;


			if (playingMeasure == currentMeasure) {

				float divisionWidth = roll.size.x / divisionsPerMeasure;
				nvgBeginPath(ctx);
				nvgStrokeColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 0.5f));
				nvgStrokeWidth(ctx, 0.5f);
				nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 0.2f));
				nvgRect(ctx, roll.pos.x + (noteInMeasure * divisionWidth), roll.pos.y, divisionWidth, roll.size.y - topMargins);
				nvgStroke(ctx);
				nvgFill(ctx);
			}

			float widthPerMeasure = roll.size.x / numberOfMeasures;
			float stepWidthInMeasure = widthPerMeasure / divisionsPerMeasure;	
			nvgBeginPath(ctx);
			nvgStrokeColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 1.f));
			nvgStrokeWidth(ctx, 1.f);
			nvgFillColor(ctx, nvgRGBAf(1.f, 1.f, 1.f, 0.2f));
			nvgRect(ctx, roll.pos.x + (playingMeasure * widthPerMeasure) + (noteInMeasure * stepWidthInMeasure), roll.pos.y + roll.size.y - topMargins, stepWidthInMeasure, topMargins);
			nvgStroke(ctx);
			nvgFill(ctx);
		}
	}


	// Event Handlers

	void draw(NVGcontext* ctx) override {
		ModuleWidget::draw(ctx);

		Rect roll = getRollArea();

		int measure = module->currentStep / module->getDivisionsPerMeasure();
		if (measure != currentMeasure && lastDrawnStep != module->currentStep) {
			lastDrawnStep = module->currentStep;
			currentMeasure = measure;
		}

		Rect keysArea = reserveKeysArea(roll);
		auto keys = getKeys(keysArea, this->octaves);
		drawKeys(ctx, keys);
		drawSwimLanes(ctx, roll, keys);
		auto beatDivs = getBeatDivs(roll, module->patternData[module->currentPattern].beatsPerMeasure, module->patternData[module->currentPattern].divisionsPerBeat, module->patternData[module->currentPattern].triplets);
		drawBeats(ctx, beatDivs);
		drawNotes(ctx, keys, beatDivs, module->patternData[module->currentPattern].measures);
		drawMeasures(ctx, module->patternData[module->currentPattern].numberOfMeasures, this->currentMeasure);
		drawPlayPosition(ctx, module->currentStep, module->patternData[module->currentPattern].numberOfMeasures, module->patternData[module->currentPattern].divisionsPerBeat, this->currentMeasure);
	}	

	void onMouseDown(EventMouseDown& e) override {
		Vec pos = gRackWidget->lastMousePos.minus(box.pos);
		std::tuple<bool, bool> octaveSwitch = findOctaveSwitch(pos);
		std::tuple<bool, int> measureSwitch = findMeasure(pos);
		
		if (e.button == 1) {
			std::tuple<bool, BeatDiv, Key> cell = findCell(e.pos);
			if (!std::get<0>(cell)) { ModuleWidget::onMouseDown(e); return; }

			e.consumed = true;
			e.target = this;

			int beatDiv = std::get<1>(cell).num;
			int pitch = std::get<2>(cell).pitch();
			module->toggleCellRetrigger(currentMeasure, beatDiv, pitch);
		} else if (e.button == 0 && std::get<0>(octaveSwitch)) {
			this->currentOctave = clamp(currentOctave + 1, -1, 8);
		} else if (e.button == 0 && std::get<1>(octaveSwitch)) {
			this->currentOctave = clamp(currentOctave - 1, -1, 8);
		} else if (e.button == 0 && std::get<0>(measureSwitch)) {
			this->currentMeasure = std::get<1>(measureSwitch);
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

struct PatternItem : MenuItem {
	PatternWidget *widget = NULL;
	int pattern;
	void onAction(EventAction &e) override {
		widget->module->currentPattern = pattern;
	}
};

struct PatternChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Pattern"));

		for (int i = 0; i < 64; i++) {
			PatternItem *item = new PatternItem();
			item->widget = widget;
			item->pattern = i;
			item->text = stringf("%d/64", i+1);
			item->rightText = CHECKMARK(item->pattern == widget->module->currentPattern);
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("Pattern %d", widget->module->currentPattern + 1);
	}
};

struct MeasuresItem : MenuItem {
	PatternWidget *widget = NULL;
	int measures;
	void onAction(EventAction &e) override {
		widget->module->setMeasures(measures);
	}
};

struct MeasuresChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Measures"));

		for (int i = 1; i <= 16; i++) {
			MeasuresItem *item = new MeasuresItem();
			item->widget = widget;
			item->measures = i;
			item->text = stringf("%d measures", i);
			item->rightText = CHECKMARK(item->measures == widget->module->patternData[widget->module->currentPattern].numberOfMeasures);
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("Measures %d", widget->module->patternData[widget->module->currentPattern].numberOfMeasures);
	}
};

struct BeatsPerMeasureItem : MenuItem {
	PatternWidget *widget = NULL;
	int beatsPerMeasure;
	void onAction(EventAction &e) override {
		widget->module->setBeatsPerMeasure(beatsPerMeasure);
	}
};

struct BeatsPerMeasureChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Beats Per Measure"));

		for (int i = 1; i <= 16; i++) {
			BeatsPerMeasureItem *item = new BeatsPerMeasureItem();
			item->widget = widget;
			item->beatsPerMeasure = i;
			item->text = stringf("%d beats", i);
			item->rightText = CHECKMARK(item->beatsPerMeasure == widget->module->patternData[widget->module->currentPattern].beatsPerMeasure);
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("%d", widget->module->patternData[widget->module->currentPattern].beatsPerMeasure);
	}
};

struct DivisionsPerBeatItem : MenuItem {
	PatternWidget *widget = NULL;
	int divisionsPerBeat;
	void onAction(EventAction &e) override {
		widget->module->setDivisionsPerBeat(divisionsPerBeat);
	}
};

struct DivisionsPerBeatChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Divisions Per Beat"));

		for (int i = 1; i <= 16; i++) {
			DivisionsPerBeatItem *item = new DivisionsPerBeatItem();
			item->widget = widget;
			item->divisionsPerBeat = i;
			item->text = stringf("%d divisions", i);
			item->rightText = CHECKMARK(item->divisionsPerBeat == widget->module->patternData[widget->module->currentPattern].divisionsPerBeat);
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("%d", widget->module->patternData[widget->module->currentPattern].divisionsPerBeat);
	}
};

struct TripletsItem : MenuItem {
	PatternWidget *widget = NULL;
	bool triplets;
	void onAction(EventAction &e) override {
		widget->module->setTriplets(triplets);
	}
};

struct TripletsChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Triplets"));

		TripletsItem *itemyes = new TripletsItem();
		itemyes->widget = widget;
		itemyes->triplets = true;
		itemyes->text = stringf("yes");
		itemyes->rightText = CHECKMARK(itemyes->triplets == widget->module->patternData[widget->module->currentPattern].triplets);
		menu->addChild(itemyes);

		TripletsItem *itemno = new TripletsItem();
		itemno->widget = widget;
		itemno->triplets = false;
		itemno->text = stringf("no");
		itemno->rightText = CHECKMARK(itemno->triplets == widget->module->patternData[widget->module->currentPattern].triplets);
		menu->addChild(itemno);
	}
	void step() override {
		text = widget->module->patternData[widget->module->currentPattern].triplets ? "y" : "n";
	}
};


PatternWidget::PatternWidget() {

	// measuresChoice
	// measuresSeparator
	// beatsPerMeasureChoice
	// beatsPerMeasureSeparator
	// divisionsPerBeatChoice
	// divisionsPerBeatSeparator
	// tripletsChoice

	Vec pos = Vec();

	PatternChoice *patternChoice = Widget::create<PatternChoice>(pos);
	patternChoice->widget = this;
	addChild(patternChoice);
	pos = patternChoice->box.getBottomLeft();
	this->patternChoice = patternChoice;

	this->patternSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->patternSeparator);

	MeasuresChoice *measuresChoice = Widget::create<MeasuresChoice>(pos);
	measuresChoice->widget = this;
	addChild(measuresChoice);
	pos = measuresChoice->box.getBottomLeft();
	this->measuresChoice = measuresChoice;

	this->measuresSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->measuresSeparator);

	BeatsPerMeasureChoice *beatsPerMeasureChoice = Widget::create<BeatsPerMeasureChoice>(pos);
	beatsPerMeasureChoice->widget = this;
	addChild(beatsPerMeasureChoice);
	this->beatsPerMeasureChoice = beatsPerMeasureChoice;

	this->beatsPerMeasureSeparator = Widget::create<LedDisplaySeparator>(pos);
	this->beatsPerMeasureSeparator->box.size.y = this->beatsPerMeasureChoice->box.size.y;
	addChild(this->beatsPerMeasureSeparator);

	DivisionsPerBeatChoice *divisionsPerBeatChoice = Widget::create<DivisionsPerBeatChoice>(pos);
	divisionsPerBeatChoice->widget = this;
	addChild(divisionsPerBeatChoice);
	this->divisionsPerBeatChoice = divisionsPerBeatChoice;

	// this->divisionsPerBeatSeparator = Widget::create<LedDisplaySeparator>(pos);
	// this->divisionsPerBeatSeparator->box.size.y = this->beatsPerMeasureChoice->box.size.y;
	// addChild(this->divisionsPerBeatSeparator);

	// TripletsChoice *tripletsChoice = Widget::create<TripletsChoice>(pos);
	// tripletsChoice->widget = this;
	// addChild(tripletsChoice);
	// this->tripletsChoice = tripletsChoice;

}

void PatternWidget::step() {
	this->patternChoice->box.size.x = box.size.x;
	this->patternSeparator->box.size.x = box.size.x;
	this->measuresChoice->box.size.x = box.size.x;
	this->measuresSeparator->box.size.x = box.size.x;
	// this->beatsPerMeasureChoice->box.size.x = box.size.x / 3;
	// this->beatsPerMeasureSeparator->box.pos.x = box.size.x / 3;
	// this->divisionsPerBeatChoice->box.pos.x = box.size.x / 3;
	// this->divisionsPerBeatChoice->box.size.x = box.size.x / 3;
	// this->divisionsPerBeatSeparator->box.pos.x = box.size.x * (2.f/3.f);
	// this->tripletsChoice->box.pos.x = box.size.x * (2.f/3.f);
	// this->tripletsChoice->box.size.x = box.size.x / 3;
	this->beatsPerMeasureChoice->box.size.x = box.size.x / 2;
	this->beatsPerMeasureSeparator->box.pos.x = box.size.x / 2;
	this->divisionsPerBeatChoice->box.pos.x = box.size.x / 2;
	this->divisionsPerBeatChoice->box.size.x = box.size.x / 2;
	LedDisplay::step();
}
