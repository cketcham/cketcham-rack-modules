#include "Template.hpp"
extern "C" 
{
	#include "../include/gverb.h"
}

struct Follower {
	float level = 0.f;

	void step(float* left, float* right) {
		auto value = max(abs(*left), abs(*right));

		if (value >= level) {
			level = value;
		} else {
			level -= (level - value) * 0.001;
		}

		if (level > 10.f) {
			*left /= (level / 10.f);
			*right /= (level / 10.f);
		}
	}
};

struct MyModule : Module {
	enum ParamIds {
		ROOM_SIZE_PARAM,
		REV_TIME_PARAM,
		DAMPING_PARAM,
		SPREAD_PARAM,
		EARLY_LEVEL_PARAM,
		TAIL_LEVEL_PARAM,
		BYPASS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	ty_gverb* gverb = NULL;

	MyModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;
	void disposeGverb();

	float p_frequency = 0.f;
	float p_room_size = 0.f;
	float p_rev_time = 0.f;
	float p_damping = 0.f;
	float p_spread = 0.f;
	float p_early_level = 0.f;
	float p_tail_level = 0.f;
	float p_bypass = 0.f;

	Follower follower;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void MyModule::disposeGverb() {
	if (gverb != NULL) {
		gverb_free(gverb);
		gverb = NULL;
	}
}

void MyModule::step() {

	if (p_frequency != engineGetSampleRate()) { disposeGverb(); }
	if (p_bypass != params[BYPASS_PARAM].value) { disposeGverb(); }

	if (gverb == NULL) {

		if (params[BYPASS_PARAM].value == 1.f) {
			gverb = gverb_new(
				engineGetSampleRate(), // freq
				50,    // max room size
				params[ROOM_SIZE_PARAM].value,    // room size
				params[REV_TIME_PARAM].value,     // revtime
				params[DAMPING_PARAM].value,   // damping
				120.0,   // spread
				0.005,     // input bandwidth
				params[EARLY_LEVEL_PARAM].value,   // early level
				params[TAIL_LEVEL_PARAM].value    // tail level
			);

			p_frequency = engineGetSampleRate();
			p_room_size = params[ROOM_SIZE_PARAM].value;
			p_rev_time = params[REV_TIME_PARAM].value;
			p_damping = params[DAMPING_PARAM].value;
			p_spread = params[SPREAD_PARAM].value;
			p_early_level = params[EARLY_LEVEL_PARAM].value;
			p_tail_level = params[TAIL_LEVEL_PARAM].value;
			p_bypass = params[BYPASS_PARAM].value;
		}
	} else {
		if (p_room_size != params[ROOM_SIZE_PARAM].value) {
			gverb_set_roomsize(gverb, params[ROOM_SIZE_PARAM].value);
			p_room_size = params[ROOM_SIZE_PARAM].value;
		}
		if (p_rev_time != params[REV_TIME_PARAM].value) {
			gverb_set_revtime(gverb, params[REV_TIME_PARAM].value);
			p_rev_time = params[REV_TIME_PARAM].value;
		}
		if (p_damping != params[DAMPING_PARAM].value) {
			gverb_set_damping(gverb, params[DAMPING_PARAM].value);
			p_damping = params[DAMPING_PARAM].value;
		}
		if (p_early_level != params[EARLY_LEVEL_PARAM].value) {
			gverb_set_earlylevel(gverb, params[EARLY_LEVEL_PARAM].value);
			p_early_level = params[EARLY_LEVEL_PARAM].value;
		}
		if (p_tail_level != params[TAIL_LEVEL_PARAM].value) {
			gverb_set_taillevel(gverb, params[TAIL_LEVEL_PARAM].value);
			p_tail_level = params[TAIL_LEVEL_PARAM].value;
		}
	}


	if (gverb != NULL) {
		gverb_do(gverb, inputs[AUDIO_INPUT].value, &outputs[LEFT_OUTPUT].value, &outputs[RIGHT_OUTPUT].value);

		outputs[RIGHT_OUTPUT].value = ((1 - params[SPREAD_PARAM].value) * outputs[LEFT_OUTPUT].value) + (params[SPREAD_PARAM].value * outputs[RIGHT_OUTPUT].value);

		follower.step(&outputs[LEFT_OUTPUT].value, &outputs[RIGHT_OUTPUT].value);
	} else {
		outputs[LEFT_OUTPUT].value = outputs[RIGHT_OUTPUT].value = inputs[AUDIO_INPUT].value;
	}


	// Implement a simple sine oscillator
	// float deltaTime = engineGetSampleTime();

	// // Compute the frequency from the pitch parameter and input
	// float pitch = params[PITCH_PARAM].value;
	// pitch += inputs[AUDIO_INPUT].value;
	// pitch = clamp(pitch, -4.0f, 4.0f);
	// // The default pitch is C4
	// float freq = 261.626f * powf(2.0f, pitch);

	// // Accumulate the phase
	// phase += freq * deltaTime;
	// if (phase >= 1.0f)
	// 	phase -= 1.0f;

	// // Compute the sine output
	// float sine = sinf(2.0f * M_PI * phase);
	// outputs[LEFT_OUTPUT].value = 5.0f * sine;
	// outputs[RIGHT_OUTPUT].value = 5.0f * sine;

	// // Blink light at 1Hz
	// blinkPhase += deltaTime;
	// if (blinkPhase >= 1.0f)
	// 	blinkPhase -= 1.0f;
	// lights[BLINK_LIGHT].value = (blinkPhase < 0.5f) ? 1.0f : 0.0f;
}

struct MyModuleWidget : ModuleWidget {
	MyModuleWidget(MyModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/MyModule.svg")));

		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(17, 30), module, MyModule::ROOM_SIZE_PARAM, 0.5, 50.0, 1.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(6, 105), module, MyModule::REV_TIME_PARAM, 0.0, 10000.0, 0.5));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(48, 105), module, MyModule::DAMPING_PARAM, 0.9, 1.0, 1.0));
		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10, 200), module, MyModule::SPREAD_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 157), module, MyModule::EARLY_LEVEL_PARAM, 0.0, 1.0, 0.8));
		addParam(ParamWidget::create<RoundBlackKnob>(Vec(51, 157), module, MyModule::TAIL_LEVEL_PARAM, 0.0, 1.0, 0.5));

		addParam(ParamWidget::create<CKSS>(Vec(64, 202), module, MyModule::BYPASS_PARAM, 0.0, 1.0, 1.0));

		addInput(Port::create<PJ301MPort>(Vec(33, 275), Port::INPUT, module, MyModule::AUDIO_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(15, 330), Port::OUTPUT, module, MyModule::LEFT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(51, 330), Port::OUTPUT, module, MyModule::RIGHT_OUTPUT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelMyModule = Model::create<MyModule, MyModuleWidget>("Template", "MyModule", "My Module", OSCILLATOR_TAG);
