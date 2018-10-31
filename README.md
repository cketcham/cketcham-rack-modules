
# RCM plugins

## Reverb

Based on the GVerb GPL'd reverb code.


This is a dirty, messy and unstable reverb. Makes for some interesting, if not repeatable effects.
The reset button is there because it often gets itself into a state that just needs you to stop everything and start again.
Reset only resets the engine, it doesn't reset any of the knob values.

GVerb is a mono in, stereo out reverb. I'm emulating stereo in by running 2 engines and mixing the output.
The Spread feature cannot be modified without recreating the engine, so it's fixed to 90 degrees and uses mixing to simulate a spread effect.

The output is normalised using an internal envelope following, because the output gets crazy high voltages otherwise. This follower can generate its own effects at times when things are out of hand.

## Audio 16

Steve Baker's version of the Audio module that supports audio devices with up to 16 inputs and outputs.

