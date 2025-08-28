#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <fstream>
#include <global.h>
#include <mutex>

#include "graphics.h"


auto makeNoteTable() {
    std::array<int, sf::Keyboard::ScancodeCount> table{};
    table[static_cast<size_t>(sf::Keyboard::Scancode::A)] = 0;
    table[static_cast<size_t>(sf::Keyboard::Scancode::W)] = 1;
    table[static_cast<size_t>(sf::Keyboard::Scancode::S)] = 2;
    table[static_cast<size_t>(sf::Keyboard::Scancode::E)] = 3;
    table[static_cast<size_t>(sf::Keyboard::Scancode::D)] = 4;
    table[static_cast<size_t>(sf::Keyboard::Scancode::F)] = 5;
    table[static_cast<size_t>(sf::Keyboard::Scancode::T)] = 6;
    table[static_cast<size_t>(sf::Keyboard::Scancode::G)] = 7;
    table[static_cast<size_t>(sf::Keyboard::Scancode::Y)] = 8;
    table[static_cast<size_t>(sf::Keyboard::Scancode::H)] = 9;
    table[static_cast<size_t>(sf::Keyboard::Scancode::U)] = 10;
    table[static_cast<size_t>(sf::Keyboard::Scancode::J)] = 11;
    table[static_cast<size_t>(sf::Keyboard::Scancode::K)] = 12;
    table[static_cast<size_t>(sf::Keyboard::Scancode::O)] = 13;
    table[static_cast<size_t>(sf::Keyboard::Scancode::L)] = 14;
    table[static_cast<size_t>(sf::Keyboard::Scancode::P)] = 15;
    table[static_cast<size_t>(sf::Keyboard::Scancode::Semicolon)] = 16;
    return table;
}
auto makeFreqTable() {
    std::array<float, sf::Keyboard::ScancodeCount> table{};
    table[static_cast<size_t>(sf::Keyboard::Scancode::A)] = A4_FREQUENCY * semitonesToMultiplier[0];
    table[static_cast<size_t>(sf::Keyboard::Scancode::W)] = A4_FREQUENCY * semitonesToMultiplier[1];
    table[static_cast<size_t>(sf::Keyboard::Scancode::S)] = A4_FREQUENCY * semitonesToMultiplier[2];
    table[static_cast<size_t>(sf::Keyboard::Scancode::E)] = A4_FREQUENCY * semitonesToMultiplier[3];
    table[static_cast<size_t>(sf::Keyboard::Scancode::D)] = A4_FREQUENCY * semitonesToMultiplier[4];
    table[static_cast<size_t>(sf::Keyboard::Scancode::F)] = A4_FREQUENCY * semitonesToMultiplier[5];
    table[static_cast<size_t>(sf::Keyboard::Scancode::T)] = A4_FREQUENCY * semitonesToMultiplier[6];
    table[static_cast<size_t>(sf::Keyboard::Scancode::G)] = A4_FREQUENCY * semitonesToMultiplier[7];
    table[static_cast<size_t>(sf::Keyboard::Scancode::Y)] = A4_FREQUENCY * semitonesToMultiplier[8];
    table[static_cast<size_t>(sf::Keyboard::Scancode::H)] = A4_FREQUENCY;
    table[static_cast<size_t>(sf::Keyboard::Scancode::U)] = A4_FREQUENCY * semitonesToMultiplier[10];
    table[static_cast<size_t>(sf::Keyboard::Scancode::J)] = A4_FREQUENCY * semitonesToMultiplier[11];
    table[static_cast<size_t>(sf::Keyboard::Scancode::K)] = A4_FREQUENCY * semitonesToMultiplier[0] * 2.f;
    table[static_cast<size_t>(sf::Keyboard::Scancode::O)] = A4_FREQUENCY * semitonesToMultiplier[1] * 2.f;
    table[static_cast<size_t>(sf::Keyboard::Scancode::L)] = A4_FREQUENCY * semitonesToMultiplier[2] * 2.f;
    table[static_cast<size_t>(sf::Keyboard::Scancode::P)] = A4_FREQUENCY * semitonesToMultiplier[3] * 2.f;
    table[static_cast<size_t>(sf::Keyboard::Scancode::Semicolon)] = A4_FREQUENCY * semitonesToMultiplier[4] * 2.f;
    return table;
} 
auto keyToNote = makeNoteTable();
auto keyToFreq = makeFreqTable();
bool isRegisteredKey(sf::Keyboard::Scancode sc) {
    return keyToFreq[static_cast<size_t>(sc)] != 0.0f;
}

std::array<std::array<float, WAVEFORM_SIZE>, 84> sawWavetable;
std::array<std::array<float, WAVEFORM_SIZE>, 84> saw2Wavetable;
std::array<std::array<float, WAVEFORM_SIZE>, 84> squareWavetable;
std::array<std::array<float, WAVEFORM_SIZE>, 84> square2Wavetable;



struct voice {
    int noteIndex;
    float frequency;
    
    float amplitude;

    float morph, normalizedMorph; 

    float panning; // 0: left, 1: right
    float currentPhase = 0.0f; // between 0 and 1
    float sample, leftPanning, rightPanning;
    float dt;

    voice(int n, float f, float a, float m, float pan, float phase)
        : frequency{f}, amplitude{a}, morph{m}, panning{pan}, currentPhase{phase}, noteIndex{n} {
            dt = frequency / SAMPLE_RATE;
            leftPanning = cosf(panning * 0.5f * PI);
            rightPanning = sinf(panning * 0.5f * PI);

            normalizedMorph = morph - (int)morph;
        }

    void addSamples(float &left, float &right) {
        int sampleIndex = currentPhase * WAVEFORM_SIZE;
     
        if (morph < 1.f) {
            sample = ( sawWavetable[noteIndex][sampleIndex] * (1 - normalizedMorph) + 
                       saw2Wavetable[noteIndex][sampleIndex] * normalizedMorph ) * amplitude ;
        } else if (morph < 2.f) {
            sample = ( saw2Wavetable[noteIndex][sampleIndex] * (1 - normalizedMorph) + 
                       square2Wavetable[noteIndex][sampleIndex] * normalizedMorph ) * amplitude ;
        } else if (morph < 3.f) {
            sample = ( square2Wavetable[noteIndex][sampleIndex] * (1 - normalizedMorph) + 
                       squareWavetable[noteIndex][sampleIndex] * normalizedMorph ) * amplitude ;
        } else {
            sample = squareWavetable[noteIndex][sampleIndex] * amplitude ;
        }


        currentPhase += dt;
        if (currentPhase >= 1.0f) currentPhase -= 1.0f;

        left  += sample * leftPanning;
        right += sample * rightPanning;
    }
};

struct note {

    struct Envelope {
        int attackSampleCount = 0, decaySampleCount = 0, releaseSampleCount = 0;
        float sustainLevel = 1.f;
    };
    enum Stage {
        Attack, Decay, Sustain, Release
    };
    sf::Keyboard::Scancode sourceKey;
    std::vector<voice> voices;
    Unison unison;
    Envelope envelope;
    Stage currentStage = Attack;
    int elapsed = 0; //number of elapsed samples since the beginning of a stage (Attack, Decay or Release)
    float envelopeFactor = 0;
    float reachedAmplitude = 0;
    bool isFinished = false;
    float frequency;
    float leftSample, rightSample;

    note(sf::Keyboard::Scancode k, int n, float f, float a, float m, const Unison &u, const Envelope &e) : sourceKey{k}, unison{u}, envelope{e}, frequency{f} {
        int detunedVoices = (u.voiceCount % 2 == 1) ? u.voiceCount - 1 : u.voiceCount;

        if (detunedVoices > 0) {
            float detuneAmp = a / sqrtf(detunedVoices); 

            if (u.voiceCount % 2 == 1) {
                voices.emplace_back(n, f, a * (1 - u.blend), m, 0.5f, 0.0f);
                detuneAmp *= u.blend;
            }

            int sideVoiceCount = detunedVoices / 2;
            float ratioBetweenFreq = powf(2.f, (float)u.detune / (1200.f * sideVoiceCount) );
            float ratio = ratioBetweenFreq;

            for (int i = 1; i <= sideVoiceCount; ++i) {
                float freq = f * ratio;
                float pan = 0.5f + i * u.stereoWidth / detunedVoices;
                float phase = pseudoRandom(i);
                voices.emplace_back(n, freq, detuneAmp, m, pan, phase);

                freq = f / ratio;
                pan = 0.5f - i * u.stereoWidth / detunedVoices;
                phase = pseudoRandom(i + sideVoiceCount);
                voices.emplace_back(n, freq, detuneAmp, m, pan, phase);

                ratio *= ratioBetweenFreq;
            }
        } else {
            voices.emplace_back(n, f, a, m, 0.5f, 0.0f);
        }
    }

    static float pseudoRandom(int value) { // return float between 0 and 1
        float hash = sinf(value * 12.9898f + 78.233f) * 43758.5453f;
        return hash - floorf(hash);
    }

    void addSamples(float &mixLeft, float &mixRight) {
        switch (currentStage) {
            case Attack:
                if (elapsed >= envelope.attackSampleCount) {
                    currentStage = Decay; 
                    envelopeFactor = 1.f;
                    elapsed = 0;
                } else {
                    envelopeFactor = (float)elapsed / (float)envelope.attackSampleCount;
                }   
                break;
            case Decay:
                if (elapsed >= envelope.decaySampleCount) {
                    currentStage = Sustain;
                    envelopeFactor = envelope.sustainLevel;
                } else {
                    envelopeFactor = 1.f - ((float)elapsed / (float)envelope.decaySampleCount) * (1 - envelope.sustainLevel);
                }
                break;
            case Release:
                if (elapsed >= envelope.releaseSampleCount) {
                    envelopeFactor = 0.f;  
                    isFinished = true;
                } else {
                    envelopeFactor = (1.f - (float)elapsed / (float)envelope.releaseSampleCount) * reachedAmplitude;
                }
                break;
        }
        leftSample = 0;
        rightSample = 0;
        for (auto &v : voices) {
            v.addSamples(leftSample, rightSample);
        }
        leftSample *= envelopeFactor;
        rightSample *= envelopeFactor;

        mixLeft += leftSample;
        mixRight += rightSample;

        ++elapsed;
    }
    void initiateRelease() {
        elapsed = 0;
        reachedAmplitude = envelopeFactor;
        currentStage = Release;
    }
};  

class Synth : public sf::SoundStream {
public:
    std::array<bool, sf::Keyboard::ScancodeCount> currentNotesPlaying{};
    std::vector<std::int16_t> sampleBuffer;

    Synth() {
        sampleBuffer.resize(2 * SAMPLE_CHUNK_SIZE);
        initialize(2, SAMPLE_RATE, {sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight});
    }
    void addNote(const note &n) {
        std::scoped_lock lock(notesMutex);
        notes.push_back(n);
    }
    void removeNoteByKey(sf::Keyboard::Scancode key) {
        std::scoped_lock lock(notesMutex);
        notes.erase(std::remove_if(notes.begin(), notes.end(), 
                    [this, key](const note &n) {
                        return n.sourceKey == key;
                    }), notes.end());
    }
    void initiateNoteRelease(sf::Keyboard::Scancode key) {
        for (auto& note : notes) {
            if (note.sourceKey == key) {
                note.initiateRelease();
                break;
            }
        }
    }
    void checkFinishedNotes() {
        std::scoped_lock lock(notesMutex);
        notes.erase( std::remove_if(notes.begin(), notes.end(),
                    [this](const note& n) { 
                        if (n.isFinished) {
                            currentNotesPlaying[static_cast<size_t>(n.sourceKey)] = false;
                            return true;
                        }
                        return false; 
                    }),
                    notes.end());
    }

private:
    std::vector<note> notes{};
    std::mutex notesMutex; //serve synchronisation between the audio (which call onGetData) and the main thread

    bool onGetData(Chunk &data) override {

        for (size_t i = 0; i < SAMPLE_CHUNK_SIZE; ++i) {
            float left = 0.f;
            float right = 0.f;
            {
                std::scoped_lock lock(notesMutex);
                for (auto &n : notes) {
                    n.addSamples(left, right);
                }
            }
            
            if (left > 1.0f) left = 1.f;
            if (left < -1.0f) left = -1.f;
            if (right > 1.0f) right = 1.f;
            if (right < -1.0f) right = -1.f;

            sampleBuffer[i * 2]     = static_cast<std::int16_t>(left * 32767);
            sampleBuffer[i * 2 + 1] = static_cast<std::int16_t>(right * 32767);
        }

        data.samples = sampleBuffer.data();
        data.sampleCount = sampleBuffer.size();
        return true; 
    }
    void onSeek(sf::Time) override {}

};


struct GloabalState {
    int octave = 3;
    float masterVolume = 0.3f; 
    float morph;
    Unison unison;
    note::Envelope envelope;
};


std::array<bool, sf::Keyboard::ScancodeCount> keyPressed{};
bool isUpdatingKnob = false;

int main() {

    std::ifstream sawFile("assets/saw.bin", std::ios::binary);
    std::ifstream saw2File("assets/saw2.bin", std::ios::binary);
    std::ifstream squareFile("assets/square.bin", std::ios::binary);
    std::ifstream square2File("assets/square2.bin", std::ios::binary);

    if (!sawFile) throw std::runtime_error("failed to load saw file!");
    if (!saw2File) throw std::runtime_error("failed to load saw2 file!");
    if (!squareFile) throw std::runtime_error("failed to load square file!");
    if (!square2File) throw std::runtime_error("failed to load square2 file!");
    
    sawFile.read(reinterpret_cast<char*>(sawWavetable.data()), sizeof(sawWavetable));
    saw2File.read(reinterpret_cast<char*>(saw2Wavetable.data()), sizeof(saw2Wavetable));
    squareFile.read(reinterpret_cast<char*>(squareWavetable.data()), sizeof(squareWavetable));
    square2File.read(reinterpret_cast<char*>(square2Wavetable.data()), sizeof(square2Wavetable));

    GloabalState currentState;

    Synth synth;
    synth.play();

    UI::KnobConfig knobConfigs[KNOB_COUNT] = {
    {{600, 540}, 1.f, 51.f, &currentState.unison.voiceCount, "Voices"},
    {{700, 540}, 1.f, 50.f, &currentState.unison.detune, "Detune"},
    {{800, 540}, 0.f, 1.f, &currentState.unison.blend, "Blend"},
    {{900, 540}, 0.f, 1.f, &currentState.unison.stereoWidth, "Stereo"},

    {{100, 540}, 0.f, 30000.f, &currentState.envelope.attackSampleCount, "Attack"},
    {{200, 540}, 0.f, 30000.f, &currentState.envelope.decaySampleCount, "Decay"},
    {{300, 540}, 0.f, 30000.f, &currentState.envelope.releaseSampleCount, "Release"},
    {{400, 540}, 0.f, 1.f, &currentState.envelope.sustainLevel, "Sustain"},
    {{400, 390}, 0.f, 3.f, &currentState.morph, "Type"},
    {{300, 390}, 0.f, 1.f, &currentState.masterVolume, "Volume"},
    };
    UI ui(knobConfigs, currentState.unison);

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Zatorius");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
                sf::Keyboard::Scancode keySC = key->scancode;

                if (!keyPressed[static_cast<size_t>(keySC)]) {
                    keyPressed[static_cast<size_t>(keySC)] = true;

                    if (isRegisteredKey(keySC))  {
                        if (synth.currentNotesPlaying[static_cast<size_t>(keySC)]) {
                            synth.removeNoteByKey(keySC);
                        }
                        synth.currentNotesPlaying[static_cast<size_t>(keySC)] = true;

                        int noteIndex = keyToNote[static_cast<size_t>(keySC)] + currentState.octave * 12;
                        if (noteIndex > 83) break;

                        float frequency = A4_FREQUENCY * octaveToMultiplier[noteIndex / 12] * semitonesToMultiplier[noteIndex % 12];

                        synth.addNote({keySC, noteIndex, frequency, currentState.masterVolume, currentState.morph, currentState.unison, currentState.envelope});
                    }
                    else if (keySC == sf::Keyboard::Scancode::Up && currentState.octave < 6) {
                        ++currentState.octave;
                    }
                }    
            }
            else if (const auto *key = event->getIf<sf::Event::KeyReleased>()) {

                sf::Keyboard::Scancode keySC = key->scancode;

                keyPressed[static_cast<size_t>(keySC)] = false;

                if (isRegisteredKey(keySC) && synth.currentNotesPlaying[static_cast<size_t>(keySC)]) {
                    synth.initiateNoteRelease(keySC);
                }
                else if (keySC == sf::Keyboard::Scancode::Down && currentState.octave > 0) {
                    --currentState.octave;
                }
            }
            else if (const auto *mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {

                sf::Mouse::Button mouseButton = mouseEvent->button;
                if (mouseButton == sf::Mouse::Button::Left) {
                    isUpdatingKnob = ui.receiveMouseClick(mouseEvent->position);
                }
            }
            else if (const auto *mouseEvent = event->getIf<sf::Event::MouseButtonReleased>()) {

                sf::Mouse::Button mouseButton = mouseEvent->button;
                if (mouseButton == sf::Mouse::Button::Left && isUpdatingKnob) {
                    ui.stopHandlingKnob();
                    isUpdatingKnob = false;
                }
            }
        }
        synth.checkFinishedNotes();

        if (isUpdatingKnob) {
            ui.handleKnob(sf::Mouse::getPosition(window));
        }
        window.clear(sf::Color::Black);
        ui.drawUI(window, synth.sampleBuffer);
        window.display();
    }
    return 0;
}
