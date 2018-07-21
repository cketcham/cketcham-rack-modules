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
		BANDWIDTH_PARAM,
		EARLY_LEVEL_PARAM,
		TAIL_LEVEL_PARAM,
		MIX_PARAM,
		RESET_PARAM,
		ROOM_SIZE_POT_PARAM,
		DAMPING_POT_PARAM,
		REV_TIME_POT_PARAM,
		BANDWIDTH_POT_PARAM,
		EARLY_LEVEL_POT_PARAM,
		TAIL_LEVEL_POT_PARAM,
		MIX_POT_PARAM,
		SPREAD_POT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		AUDIO_INPUT,
		ROOM_SIZE_INPUT,
		DAMPING_INPUT,
		REV_TIME_INPUT,
		BANDWIDTH_INPUT,
		EARLY_LEVEL_INPUT,
		TAIL_LEVEL_INPUT,
		MIX_INPUT,
		SPREAD_INPUT,
		RESET_INPUT,
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
	float p_bandwidth = 0.f;
	float p_early_level = 0.f;
	float p_tail_level = 0.f;
	float p_reset = 0.f;

	Follower follower;

	float getParam(ParamIds param, InputIds mod, ParamIds trim, float min, float max);
	void handleParam(float value, float* store, void (*change)(ty_gverb*,float));

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

float MyModule::getParam(ParamIds param, InputIds mod, ParamIds trim, float min, float max) {
	return clamp2(params[param].value + (((clamp2(inputs[mod].value, -10.f, 10.f)/10) * max) * params[trim].value), min, max);
}

void MyModule::handleParam(float value, float* store, void (*change)(ty_gverb*,float)) {
	if (*store != value) {
		//info("Change old: %f, new: %f", *store, value);
		change(gverb, value);
		*store = value;
	}
}

void MyModule::step() {
	auto reset = max(params[RESET_PARAM].value, inputs[RESET_INPUT].value);
	auto mix = getParam(MIX_PARAM, MIX_INPUT, MIX_POT_PARAM, 0.f, 1.f);

	//info("Mix: %f, %f, %f, %f, %f, %f", mix, params[MIX_PARAM].value, clamp2(inputs[MIX_INPUT].value, -10.f, 10.f), params[MIX_POT_PARAM].value, 0.f, 1.f);

	if (p_frequency != engineGetSampleRate()) { disposeGverb(); }
	if (params[MIX_PARAM].value == 0.f) { disposeGverb(); }
	if (p_reset == 0.f && reset > 0.f) {
		disposeGverb();
	}

	p_reset = reset;

	if (gverb == NULL) {

		if (mix > 0.f) {
			info("Creating with mix %f", mix);
			gverb = gverb_new(
				engineGetSampleRate(), // freq
				300,    // max room size
				params[ROOM_SIZE_PARAM].value,    // room size
				params[REV_TIME_PARAM].value,     // revtime
				params[DAMPING_PARAM].value,   // damping
				90.0,   // spread
				params[BANDWIDTH_PARAM].value,     // input bandwidth
				params[EARLY_LEVEL_PARAM].value,   // early level
				params[TAIL_LEVEL_PARAM].value    // tail level
			);

			p_frequency = engineGetSampleRate();
			p_room_size = getParam(ROOM_SIZE_PARAM, ROOM_SIZE_INPUT, ROOM_SIZE_POT_PARAM, 2.f, 300.f);
			p_rev_time = getParam(REV_TIME_PARAM, REV_TIME_INPUT, REV_TIME_POT_PARAM, 0.f, 10000.f);
			p_damping = getParam(DAMPING_PARAM, DAMPING_INPUT, DAMPING_POT_PARAM, 0.f, 1.f);
			p_bandwidth = getParam(BANDWIDTH_PARAM, BANDWIDTH_INPUT, BANDWIDTH_POT_PARAM, 0.f, 1.f);
			p_early_level = getParam(EARLY_LEVEL_PARAM, EARLY_LEVEL_INPUT, EARLY_LEVEL_POT_PARAM, 0.f, 1.f);
			p_tail_level = getParam(TAIL_LEVEL_PARAM, TAIL_LEVEL_INPUT, TAIL_LEVEL_POT_PARAM, 0.f, 1.f);
		}
	} else {
		handleParam(getParam(ROOM_SIZE_PARAM, ROOM_SIZE_INPUT, ROOM_SIZE_POT_PARAM, 2.f, 300.f), &p_room_size, gverb_set_roomsize);
		handleParam(getParam(REV_TIME_PARAM, REV_TIME_INPUT, REV_TIME_POT_PARAM, 0.f, 10000.f), &p_rev_time, gverb_set_revtime);
		handleParam(getParam(DAMPING_PARAM, DAMPING_INPUT, DAMPING_POT_PARAM, 0.f, 1.f), &p_damping, gverb_set_damping);
		handleParam(getParam(BANDWIDTH_PARAM, BANDWIDTH_INPUT, BANDWIDTH_POT_PARAM, 0.f, 1.f), &p_bandwidth, gverb_set_inputbandwidth);
		handleParam(getParam(EARLY_LEVEL_PARAM, EARLY_LEVEL_INPUT, EARLY_LEVEL_POT_PARAM, 0.f, 1.f), &p_early_level, gverb_set_earlylevel);
		handleParam(getParam(TAIL_LEVEL_PARAM, TAIL_LEVEL_INPUT, TAIL_LEVEL_POT_PARAM, 0.f, 1.f), &p_tail_level, gverb_set_taillevel);
	}


	if (gverb != NULL) {
		gverb_do(gverb, inputs[AUDIO_INPUT].value, &outputs[LEFT_OUTPUT].value, &outputs[RIGHT_OUTPUT].value);

		isfinite(outputs[LEFT_OUTPUT].value) ? outputs[LEFT_OUTPUT].value : 0.f;
		isfinite(outputs[RIGHT_OUTPUT].value) ? outputs[RIGHT_OUTPUT].value : 0.f;

		auto spread = getParam(SPREAD_PARAM, SPREAD_INPUT, SPREAD_POT_PARAM, 0.f, 1.f);
		auto spreadleft = (outputs[LEFT_OUTPUT].value + ((1-spread) * outputs[RIGHT_OUTPUT].value)) / 2;
		auto spreadright = (outputs[RIGHT_OUTPUT].value + ((1-spread) * outputs[LEFT_OUTPUT].value)) / 2;
		outputs[LEFT_OUTPUT].value = spreadleft;
		outputs[RIGHT_OUTPUT].value = spreadright;

		follower.step(&outputs[LEFT_OUTPUT].value, &outputs[RIGHT_OUTPUT].value);

		outputs[RIGHT_OUTPUT].value = ((1 - mix) * inputs[AUDIO_INPUT].value) + (mix * outputs[RIGHT_OUTPUT].value);
		outputs[LEFT_OUTPUT].value = ((1 - mix) * inputs[AUDIO_INPUT].value) + (mix * outputs[LEFT_OUTPUT].value);

	} else {
		outputs[LEFT_OUTPUT].value = outputs[RIGHT_OUTPUT].value = inputs[AUDIO_INPUT].value;
	}
}

struct MyModuleWidget : ModuleWidget {
	MyModuleWidget(MyModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Reverb.svg")));

		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(50, 44), module, MyModule::ROOM_SIZE_PARAM, 2.0, 300.0, 2.0));
		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(50, 115), module, MyModule::DAMPING_PARAM, 0.0, 1.0, 0.95));

		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(127, 60), module, MyModule::REV_TIME_PARAM, 0.0, 10000.0, 5000.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(127, 120), module, MyModule::BANDWIDTH_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(185, 60), module, MyModule::EARLY_LEVEL_PARAM, 0.0, 1.0, 0.8));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(185, 120), module, MyModule::TAIL_LEVEL_PARAM, 0.0, 1.0, 0.5));

		addParam(ParamWidget::create<RoundBlackKnob>(Vec(84, 189), module, MyModule::MIX_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<RoundBlackKnob>(Vec(135, 189), module, MyModule::SPREAD_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<PB61303>(Vec(186, 189), module, MyModule::RESET_PARAM, 0.0, 1.0, 0.0));

		addParam(ParamWidget::create<Trimpot>(Vec(15, 263), module, MyModule::ROOM_SIZE_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(42, 263), module, MyModule::DAMPING_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(70, 263), module, MyModule::REV_TIME_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(97, 263), module, MyModule::BANDWIDTH_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(124, 263), module, MyModule::EARLY_LEVEL_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(151, 263), module, MyModule::TAIL_LEVEL_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(178, 263), module, MyModule::MIX_POT_PARAM, -1.f, 1.f, 0.f));
		addParam(ParamWidget::create<Trimpot>(Vec(205, 263), module, MyModule::SPREAD_POT_PARAM, -1.f, 1.f, 0.f));

		addInput(Port::create<PJ301MPort>(Vec(14, 286), Port::INPUT, module, MyModule::ROOM_SIZE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(41, 286), Port::INPUT, module, MyModule::DAMPING_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(68, 286), Port::INPUT, module, MyModule::REV_TIME_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(95, 286), Port::INPUT, module, MyModule::BANDWIDTH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(123, 286), Port::INPUT, module, MyModule::EARLY_LEVEL_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(150, 286), Port::INPUT, module, MyModule::TAIL_LEVEL_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(177, 286), Port::INPUT, module, MyModule::MIX_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(204, 286), Port::INPUT, module, MyModule::SPREAD_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(232, 286), Port::INPUT, module, MyModule::RESET_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(14, 332), Port::INPUT, module, MyModule::AUDIO_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(204, 332), Port::OUTPUT, module, MyModule::LEFT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(232, 332), Port::OUTPUT, module, MyModule::RIGHT_OUTPUT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelMyModule = Model::create<MyModule, MyModuleWidget>("Template", "MyModule", "My Module", OSCILLATOR_TAG);
