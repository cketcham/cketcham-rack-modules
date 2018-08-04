
# RCM plugins

## Reverb

Based on the GVerb GPL'd reverb code.


This is a dirty, messy and unstable reverb. Makes for some interesting, if not repeatable effects.
The reset button is there because it often gets itself into a state that just needs you to stop everything and start again.
Reset only resets the engine, it doesn't reset any of the knob values.

GVerb is a mono in, stereo out reverb. I'm emulating stereo in by running 2 engines and mixing the output.
The Spread feature cannot be modified without recreating the engine, so it's fixed to 90 degrees and uses mixing to simulate a spread effect.

The output is normalised using an internal envelope following, because the output gets crazy high voltages otherwise. This follower can generate its own effects at times when things are out of hand.

## Duck

A basic audio ducking module.

Takes 2 stereo inputs:

* over input is the audio that gets overlaid.
* under input is the base audio stream that gets ducked.

There is a single stereo output that contains the result of the ducking operation.

The tunables aren't CV'd since they don't seem to be generic - once you get your sound they tend to stay fixed. I may change this, but it would result in a wider module to accomadate the patching bay.

