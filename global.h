#pragma once

constexpr float PI = 3.14159265f;
constexpr unsigned int SAMPLE_RATE = 44100, SAMPLE_CHUNK_SIZE = 512;
constexpr unsigned int WIDTH = 1000, HEIGHT = 600; //window dimensions
constexpr float A4_FREQUENCY = 440.0f;

constexpr int WAVEFORM_SIZE = 1024;
constexpr float PHASE_INCREMENT = 2 * PI / WAVEFORM_SIZE;

constexpr int KNOB_COUNT = 10;

constexpr float octaveToMultiplier[7] = {0.125f, 0.25f, 0.5f, 1.f, 2.f, 4.f, 8.f};
constexpr float semitonesToMultiplier[12] = { 0.5946035575f, 0.6299605249f, 0.6674199271f, 0.7071067812f, 
                                              0.7491535384f, 0.793700526f,  0.8408964153f, 0.8908987181f, 
                                              0.9438743127f, 1.f,           1.059463094f,  1.122462048f };

struct Unison {
    int voiceCount = 1;
    int detune = 0.f;      // in cents
    float stereoWidth = 0.f; // 0: mono, 1: max width
    float blend = 0.f;       // 0: only main freq, 1: only detuned voices
    Unison() {}
    Unison(int v, int d, float s, float b)
        : voiceCount{v}, detune{d}, stereoWidth{s}, blend{b} {}
};