
# RCM plugins

## Piano Roll

*early version - forward compatibility not 100% guaranteed*

A monophonic quantized sequencer. With `1v/oct`, `gate`, `retrigger` & `velocity` outputs. 64 patterns with up to 16 measures per pattern, up to 16 beats per measure and up to 16 divisions per beat. `EOP` will trigger when the last note in the pattern has been triggered. Patterns always loop (currently).

Controls:

* Left click a cell to activate that note.
* Right click an active note to cause it to retrigger.
* Hold down shift then drag a note up & down to alter the velocity, when dragging, press ctrl for more fine-grained control.
* Use the panel on the right to alter the current pattern and its make-up.
* Send a pulse to clock-in to advance each step (no internal clock).
* Send a pulse to reset to prepare to play the current pattern from the start (plays 1st note on receiving first clock).

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

