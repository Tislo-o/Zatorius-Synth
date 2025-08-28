This synth features the very basics of digital synthesizers. You can control:
-waveform shape and volume
-unison parameters
-sound envelope(ADSR)

The synth doesn't suffer from aliasing because it doesn't generate "naïve" waveforms but load at runtime prebuilt bandlimited ones. 
The latter were generated in order to match each note to a waveform that contains only harmonics below Nyquist frequency.
