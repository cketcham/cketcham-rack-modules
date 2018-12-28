#include "GVerbWidget.hpp"
#include "../include/BaseWidget.hpp"

struct CVTglModule : Module {
	enum ParamIds {
		BUTTON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CVTglModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CVTglModule::step() {
	outputs[CV_OUTPUT].value = params[BUTTON_PARAM].value * 10.f;
}

struct CKSSWhite : SVGSwitch, ToggleSwitch {
	CKSSWhite() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_0_White.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_1_White.svg")));
	}
};

struct CVTglModuleWidget : BaseWidget {
    TextField *textField;

	CVTglModuleWidget(CVTglModule *module) : BaseWidget(module) {
		colourHotZone = Rect(Vec(10, 10), Vec(50, 13));
		backgroundHue = 0.754;
		backgroundSaturation = 1.f;
		backgroundLuminosity = 0.58f;

		setPanel(SVG::load(assetPlugin(plugin, "res/CVTgl.svg")));

		addParam(ParamWidget::create<CKSSWhite>(Vec(31, 172), module, CVTglModule::BUTTON_PARAM, 0.0, 1.0, 0.0));

		addOutput(Port::create<PJ301MPort>(Vec(26, 331), Port::OUTPUT, module, CVTglModule::CV_OUTPUT));

        textField = Widget::create<LedDisplayTextField>(Vec(7.5, 38.0));
		textField->box.size = Vec(60.0, 80.0);
		textField->multiline = true;
        ((LedDisplayTextField*)textField)->color = COLOR_WHITE;
		addChild(textField);

	}

	json_t *toJson() override {
		json_t *rootJ = BaseWidget::toJson();

		// text
		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		BaseWidget::fromJson(rootJ);

		// text
		json_t *textJ = json_object_get(rootJ, "text");
		if (textJ)
			textField->text = json_string_value(textJ);
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelCVTglModule = Model::create<CVTglModule, CVTglModuleWidget>("rcm", "rcm-CVTgl", "CVTgl", ENVELOPE_FOLLOWER_TAG);
