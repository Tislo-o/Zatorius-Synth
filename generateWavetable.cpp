#include "global.h"
#include <iostream>
#include <array>
#include <cmath>
#include <fstream>

auto makeSquareWavetable() {
    std::array<std::array<float, WAVEFORM_SIZE>, 84> wavetable;
    std::array<float, WAVEFORM_SIZE> waveform = {};
    
    int maxHarmonic = 0;

    for (int octave = 6; octave > -1; --octave) {
        for (int n = 11; n > -1; --n) {

            float frequency = semitonesToMultiplier[n] * octaveToMultiplier[octave] * A4_FREQUENCY;
            int newMaxHarmonic = (int)(SAMPLE_RATE / (2.f * frequency));

            for (int h = maxHarmonic + 1; h <= newMaxHarmonic; ++h) {
                if (h % 2 == 1) {
                    for (int s = 0; s < WAVEFORM_SIZE; ++s) {         
                        waveform[s] += std::sinf(s * PHASE_INCREMENT * h) / h;     
                    }
                }
            }
            maxHarmonic = newMaxHarmonic;

            std::array<float, WAVEFORM_SIZE> temp = waveform;

            float maxVal = 0.f;
            for(auto f : temp) maxVal = std::max(maxVal, std::abs(f));
            for(auto& f : temp) f /= maxVal;

            wavetable[octave * 12 + n] = temp;
        }
    }
    return wavetable;
}
auto makeSawWavetable() {
    std::array<std::array<float, WAVEFORM_SIZE>, 84> wavetable;
    std::array<float, WAVEFORM_SIZE> waveform = {};
    
    int maxHarmonic = 0;

    for (int octave = 6; octave > -1; --octave) {
        for (int n = 11; n > -1; --n) {

            float frequency = semitonesToMultiplier[n] * octaveToMultiplier[octave] * A4_FREQUENCY;
            int newMaxHarmonic = (int)(SAMPLE_RATE / (2.f * frequency));

            for (int h = maxHarmonic + 1; h <= newMaxHarmonic; ++h) {
                for (int s = 0; s < WAVEFORM_SIZE; ++s) {
                    waveform[s] += std::sinf(s * PHASE_INCREMENT * h) / h;
                }
            }
            maxHarmonic = newMaxHarmonic;

            std::array<float, WAVEFORM_SIZE> temp = waveform;

            float maxVal = 0.f;
            for(auto f : temp) maxVal = std::max(maxVal, std::abs(f));
            for(auto& f : temp) f /= maxVal;

            wavetable[octave * 12 + n] = temp;
        }
    }
    return wavetable;
}
auto makeWavetable() {
    std::array<std::array<float, WAVEFORM_SIZE>, 84> wavetable;
    std::array<float, WAVEFORM_SIZE> waveform = {};
    
    int maxHarmonic = 0;

    for (int octave = 6; octave > -1; --octave) {
        for (int n = 11; n > -1; --n) {

            float frequency = semitonesToMultiplier[n] * octaveToMultiplier[octave] * A4_FREQUENCY;
            int newMaxHarmonic = (int)(SAMPLE_RATE / (2.f * frequency));

            for (int h = maxHarmonic + 1; h <= newMaxHarmonic; ++h) {
                for (int s = 0; s < WAVEFORM_SIZE; ++s) {
                    waveform[s] += std::sinf(s * PHASE_INCREMENT * h) * std::abs(std::sinf(478.6f/h - h * sqrt(h))) / (0.1f * h);
                }
            }
            maxHarmonic = newMaxHarmonic;

            std::array<float, WAVEFORM_SIZE> temp = waveform;

            float maxVal = 0.f;
            for(auto f : temp) maxVal = std::max(maxVal, std::abs(f));
            for(auto& f : temp) f /= maxVal;

            wavetable[octave * 12 + n] = temp;
        }
    }
    return wavetable;
}
std::array<std::array<float, WAVEFORM_SIZE>, 84> wavetable = makeWavetable();

int main() {
    std::ofstream file("assets/saw2.bin", std::ios::binary);
    if (!file) return 1;

    file.write(reinterpret_cast<const char*>(wavetable.data()), sizeof(wavetable));
    return 0;
}