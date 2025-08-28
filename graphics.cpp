#include "graphics.h"
#include <iostream>

bool UI::receiveMouseClick(const sf::Vector2i &mousePos) { //return if the position corresponds to a knob
    for (int i = 0; i < KNOB_COUNT; ++i) {
        sf::Vector2f knobPos = knobs[i].sprite->getPosition();
        float deltaX = knobPos.x - mousePos.x;
        float deltaY = knobPos.y - mousePos.y;
        
        if (sqrt(deltaX*deltaX + deltaY*deltaY) < 30.f) {
            operatedKnobIndex = i;
            clickStartingPos = mousePos;
            return true;
        }
    }
    return false;
}

void UI::handleKnob(const sf::Vector2i &mousePos) {
    float delta = static_cast<float>(clickStartingPos.y - mousePos.y) / 1.5f;
    delta = std::clamp(delta, knobs[operatedKnobIndex].minDeltaLeft, knobs[operatedKnobIndex].maxDeltaRight);

    knobs[operatedKnobIndex].sprite->setRotation(knobs[operatedKnobIndex].angle + sf::degrees(delta));

    knobs[operatedKnobIndex].setParameterValue();

    if (operatedKnobIndex < 4) {//if it is a unison knob
        generateUnisonLines();
    }
}
void UI::stopHandlingKnob() {
    sf::Angle newAngle = knobs[operatedKnobIndex].sprite->getRotation();
    knobs[operatedKnobIndex].angle = newAngle;

    knobs[operatedKnobIndex].setKnobDeltaLimits();
    knobs[operatedKnobIndex].setParameterValue();

    if (operatedKnobIndex < 4) {//if it is a unison knob
        generateUnisonLines();
    }
}


void UI::drawKnobs(sf::RenderWindow &window) {
    for (int i = 0; i < KNOB_COUNT; ++i) {
        window.draw(knobs[i].sprite.value());
        window.draw(knobs[i].nameText.value());
        window.draw(knobs[i].valueText.value());
    }
}

void UI::drawSoundBuffer(sf::RenderWindow &window, const std::vector<int16_t> &sampleBuffer) {
    float maxSample = 0;
    for (size_t i = 0; i < SAMPLE_CHUNK_SIZE; ++i) {

        float x = i * X_SAMPLE_GAP + 502;

        float normalizedSample = sampleBuffer[i * 2] / 32767.f; 
        leftWaveform[i] = sf::Vertex{sf::Vector2f{ x, -146 * normalizedSample + 151}, sf::Color::Green}; 

        if (normalizedSample > maxSample) maxSample = normalizedSample;

        normalizedSample = sampleBuffer[i * 2 + 1] / 32767.f; 
        rightWaveform[i] = sf::Vertex{sf::Vector2f{ x, -146 * normalizedSample + 151}, sf::Color::Cyan};                      
    }
    window.draw(leftWaveform, SAMPLE_CHUNK_SIZE, sf::PrimitiveType::LineStrip);        
    window.draw(rightWaveform, SAMPLE_CHUNK_SIZE, sf::PrimitiveType::LineStrip); 
}