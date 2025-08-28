#pragma once
#include <vector>
#include <optional>
#include <cmath>
#include <iostream>
#include <functional>
#include <SFML/Graphics.hpp>
#include "global.h"


class UI {
private:
    static constexpr float X_SAMPLE_GAP = (float)500 / (SAMPLE_CHUNK_SIZE - 1); //buffer waveform
    sf::Vertex leftWaveform[SAMPLE_CHUNK_SIZE], rightWaveform[SAMPLE_CHUNK_SIZE];

    Unison *pUnison;
    std::vector<sf::Vertex> unisonLines;
    void generateUnisonLines() {

        unisonLines.resize(0);
        int detunedVoices;
        float y;

        if (pUnison->voiceCount % 2 == 1) {
            unisonLines.emplace_back( sf::Vertex{sf::Vector2f{750, 480}, sf::Color::Cyan});
            if (pUnison->voiceCount == 1) {
                unisonLines.emplace_back( sf::Vertex{sf::Vector2f{750, 330.f }, sf::Color::Cyan});
            } else {
                unisonLines.emplace_back( sf::Vertex{sf::Vector2f{750, 480 - (1 - pUnison->blend) * 150.f}, sf::Color::Cyan});
            }
            
            detunedVoices = pUnison->voiceCount - 1;
            y = 480.f - pUnison->blend * 150.f;
        } else {
            detunedVoices = pUnison->voiceCount;
            y = 330.f;
        }
        if (detunedVoices > 0) {
            int sideVoiceCount = detunedVoices / 2;
            float xGap = pUnison->detune * 4.8f / (float)sideVoiceCount;

            for (int i = 1; i <= sideVoiceCount; ++i) {
                unisonLines.emplace_back( sf::Vertex{sf::Vector2f{750 + i * xGap, 480}, sf::Color::Green});
                unisonLines.emplace_back( sf::Vertex{sf::Vector2f{750 + i * xGap, y}, sf::Color::Green});

                unisonLines.emplace_back( sf::Vertex{sf::Vector2f{750 - i * xGap, 480}, sf::Color::Green});
                unisonLines.emplace_back( sf::Vertex{sf::Vector2f{750 - i * xGap, y}, sf::Color::Green});
            }
        }
    }

    sf::Texture knobTexture;
    sf::Texture mainTexture;
    std::optional<sf::Sprite> main;

    sf::Font font;

    std::optional<sf::Text> aidsText;

    struct knob {
        std::optional<sf::Text> nameText;
        std::optional<sf::Text> valueText;
        sf::FloatRect valueTextBounds;

        std::optional<sf::Sprite> sprite;
        sf::Angle angle;
        float maxDeltaRight;
        float minDeltaLeft;

        float minValue;
        float maxValue;

        enum class Type { Float, Int} type;
        union {
            float* fptr;
            int* iptr;
        } pParameter;
        
        void setParameterValue() { //change the corresponding parameter
            float inDegrees = sprite->getRotation().asDegrees();
            float normalized; //between 0 and 1

            if (inDegrees < 180) {
                normalized = inDegrees / 260.f + 0.5f;
            } else {
                normalized = (inDegrees - 230.f) / 260.f;
            }

            float value = minValue + normalized * (maxValue - minValue);

            if (type == Type::Int) {
                value = static_cast<int>(value);
                *pParameter.iptr = value;
                valueText->setString((std::to_string(static_cast<int>(value))));

            } else { 
                *pParameter.fptr = value;
                valueText->setString((std::to_string(value).substr(0, 3)));
            }
            valueTextBounds = valueText->getLocalBounds();
            valueText->setOrigin({std::round(valueTextBounds.size.x / 2.f), 0.f});
        }

        void setKnobDeltaLimits() { //set max and min scrolling deltas 
            float inDegrees = angle.asDegrees();

            if (inDegrees < 180.f) {
                maxDeltaRight = 130.f - inDegrees;
                minDeltaLeft = -130.f - inDegrees;
            } else {
                maxDeltaRight = 490.f - inDegrees;
                minDeltaLeft = 230.f - inDegrees;
            }
        }
    };
    knob knobs[KNOB_COUNT];

    int operatedKnobIndex; 
    sf::Vector2i clickStartingPos;

    void drawKnobs(sf::RenderWindow &window);
    void drawSoundBuffer(sf::RenderWindow &window, const std::vector<int16_t> &sampleBuffer);

public:
    struct KnobConfig {
        sf::Vector2f knobPos;
        float minValue;
        float maxValue;
        std::variant<float*, int*> pParameter;
        sf::String nameText;
    };

    UI(const KnobConfig knobConfigs[KNOB_COUNT], Unison &u) {

        if (!mainTexture.loadFromFile("assets/main.png")) {
            throw std::runtime_error("failed to load main texture!");
        }
        if (!knobTexture.loadFromFile("assets/knob.png")) {
            throw std::runtime_error("failed to load knob texture!");
        }
        if (!font.openFromFile("assets/Alegreya.ttf")) {
            throw std::runtime_error("failed to load font!");
        }
        
        for (int i = 0; i < KNOB_COUNT; ++i) {
            knobs[i].nameText.emplace(font);
            knobs[i].nameText->setString(knobConfigs[i].nameText);
            knobs[i].nameText->setCharacterSize(17); 
            knobs[i].nameText->setFillColor(sf::Color::White); 

            sf::FloatRect bounds = knobs[i].nameText->getLocalBounds();

            knobs[i].nameText->setOrigin({std::round(bounds.size.x / 2.f), 0.f});
            knobs[i].nameText->setPosition({knobConfigs[i].knobPos.x, knobConfigs[i].knobPos.y + 35.f});

            knobs[i].valueText.emplace(font);
            knobs[i].valueText->setCharacterSize(17); 
            knobs[i].valueText->setFillColor(sf::Color::White); 
            knobs[i].valueText->setPosition({knobConfigs[i].knobPos.x, knobConfigs[i].knobPos.y - 57.f});

            knobs[i].sprite.emplace(knobTexture);
            knobs[i].sprite->setOrigin({100.0f, 100.0f});
            knobs[i].sprite->setScale({0.3f, 0.3f});
            knobs[i].sprite->setPosition(knobConfigs[i].knobPos);

            if (i == 0 || i == 4 || i == 8) {
                knobs[i].angle = sf::degrees(230.0f);
            } else {
                knobs[i].angle = sf::degrees(0.0f);
            }
            knobs[i].sprite->setRotation(knobs[i].angle);

            knobs[i].minValue = knobConfigs[i].minValue; 
            knobs[i].maxValue = knobConfigs[i].maxValue; 

            if (auto pf = std::get_if<float*>(&knobConfigs[i].pParameter)) {
                knobs[i].type = knob::Type::Float;
                knobs[i].pParameter.fptr = *pf;  
            } else if (auto pi = std::get_if<int*>(&knobConfigs[i].pParameter)) {
                knobs[i].type = knob::Type::Int;
                knobs[i].pParameter.iptr = *pi; 
            } else {
                throw std::runtime_error("the variable passed is neither a float or an int !");
            }

            knobs[i].setParameterValue();
            knobs[i].setKnobDeltaLimits();
        }
        main.emplace(mainTexture);
        main->setPosition({0, 0});
        main->setScale({0.666f, 0.666f});

        aidsText.emplace(font);
        aidsText->setCharacterSize(20);
        aidsText->setFillColor(sf::Color::White);
        aidsText->setPosition({30, 340});
        aidsText->setString({"Press keyboard keys \nto make notes.\nUse up/down arrows to \nchange octave."});

        pUnison = &u;
        generateUnisonLines();
    }

    void drawUI(sf::RenderWindow &window, const std::vector<int16_t> &sampleBuffer) {
        window.draw(main.value());
        window.draw(aidsText.value());
        drawKnobs(window);

        drawSoundBuffer(window, sampleBuffer);
        window.draw(unisonLines.data(), unisonLines.size(), sf::PrimitiveType::Lines);
    }

    bool receiveMouseClick(const sf::Vector2i &mousePos); //return if the position corresponds to a knob
    void handleKnob(const sf::Vector2i &mousePos);
    void stopHandlingKnob();
};
