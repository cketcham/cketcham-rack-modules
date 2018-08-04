#include "GVerbWidget.hpp"

struct Follower {
	float level = 0.f;

	float step(float* left, float* right, float recovery) {
		auto value = max(abs(*left), abs(*right));

		if (value >= level) {
			level = value;
		} else {
			level -= (level - value) / (engineGetSampleRate() * recovery);
		}

		return clamp2(level / 10.f, 0.f, 1.f);
	}
};

struct DuckModule : Module {
	enum ParamIds {
		AMOUNT_PARAM,
		RECOVERY_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LEFT_OVER_AUDIO,
		RIGHT_OVER_AUDIO,
		LEFT_UNDER_AUDIO,
		RIGHT_UNDER_AUDIO,
		NUM_INPUTS
	};
	enum OutputIds {
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	DuckModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	Follower follower;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void DuckModule::step() {
	auto amount = clamp2(follower.step(&inputs[LEFT_OVER_AUDIO].value, &inputs[RIGHT_OVER_AUDIO].value, params[RECOVERY_PARAM].value) * pow(params[AMOUNT_PARAM].value, 2.0), 0.f, 1.f);

	outputs[LEFT_OUTPUT].value = crossfade(inputs[LEFT_UNDER_AUDIO].value, inputs[LEFT_OVER_AUDIO].value, amount);
	outputs[RIGHT_OUTPUT].value = crossfade(inputs[RIGHT_UNDER_AUDIO].value, inputs[RIGHT_OVER_AUDIO].value, amount);
}

struct DuckModuleWidget : ModuleWidget {
	DuckModuleWidget(DuckModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Duck.svg")));

		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(10, 65), module, DuckModule::AMOUNT_PARAM, 0.0, 2.0, 1.0));
		addParam(ParamWidget::create<Davies1900hLargeWhiteKnob>(Vec(10, 166), module, DuckModule::RECOVERY_PARAM, 0.0, 1.0, 0.5));

		addInput(Port::create<PJ301MPort>(Vec(12, 257), Port::INPUT, module, DuckModule::LEFT_OVER_AUDIO));
		addInput(Port::create<PJ301MPort>(Vec(40, 257), Port::INPUT, module, DuckModule::RIGHT_OVER_AUDIO));

		addInput(Port::create<PJ301MPort>(Vec(12, 295), Port::INPUT, module, DuckModule::LEFT_UNDER_AUDIO));
		addInput(Port::create<PJ301MPort>(Vec(40, 295), Port::INPUT, module, DuckModule::RIGHT_UNDER_AUDIO));

		addOutput(Port::create<PJ301MPort>(Vec(12, 338), Port::OUTPUT, module, DuckModule::LEFT_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(40, 338), Port::OUTPUT, module, DuckModule::RIGHT_OUTPUT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelDuckModule = Model::create<DuckModule, DuckModuleWidget>("rcm", "rcm-duck", "Duck", ENVELOPE_FOLLOWER_TAG);
