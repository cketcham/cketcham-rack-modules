#include "GVerbWidget.hpp"


Plugin *plugin;


void init(Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	// Add all Models defined throughout the plugin
	p->addModel(modelGVerbModule);
	p->addModel(modelDuckModule);
	p->addModel(modelCV0to10Module);
	p->addModel(modelCVS0to10Module);
	p->addModel(modelCV5to5Module);
	p->addModel(modelCVMmtModule);
	p->addModel(modelCVTglModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
