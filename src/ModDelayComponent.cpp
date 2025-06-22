#include "ModDelayComponent.h"

ModDelayComponent::ModDelayComponent(juce::AudioProcessorValueTreeState& state)
{
    addAndMakeVisible(group);
    group.setColour(juce::GroupComponent::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    group.setColour(juce::GroupComponent::textColourId, juce::Colours::white);

    configureKnob(delayTime, "delayTime", "Delay", state);
    configureKnob(feedbackL, "feedbackL", "FB Left", state);
    configureKnob(feedbackR, "feedbackR", "FB Right", state);
    configureKnob(mix, "modMix", "Mod Mix", state);
    configureKnob(modDepth, "modDepth", "Depth", state);
    configureKnob(modRate, "modRate", "Rate", state);

    // --- Waveform buttons ---
    const std::vector<std::pair<juce::String, ModDelay::ModulationType>> waveformData = {
        { "Sine", ModDelay::ModulationType::Sine },
        { "Tri", ModDelay::ModulationType::Triangle },
        { "Sq", ModDelay::ModulationType::Square },
        { "Saw+", ModDelay::ModulationType::SawtoothUp },
        { "Saw-", ModDelay::ModulationType::SawtoothDown }
    };

    int idx = 0;
    for (auto& [label, type] : waveformData) {
        auto* btn = waveformButtons.add(new juce::TextButton(label));
        btn->onClick = [this, idx] { updateWaveformSelection(idx); };
        addAndMakeVisible(btn);
        ++idx;
    }

    // Attach modulation type param invisibly for state save/load
    auto* hiddenCombo = new juce::ComboBox();
    modulationTypeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(state, "modulationType", *hiddenCombo));
    hiddenCombo->setComponentID("modTypeCombo");
    hiddenCombo->setVisible(false);
    addAndMakeVisible(hiddenCombo);

    // Sync toggle in advanced section
    syncToggle.setButtonText("Sync");
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "sync", syncToggle);
    advancedSection.addAndMakeVisible(syncToggle);

    advancedSection.addAndMakeVisible(feedbackL.slider);
    advancedSection.addAndMakeVisible(feedbackR.slider);
    advancedSection.addAndMakeVisible(feedbackL.label);
    advancedSection.addAndMakeVisible(feedbackR.label);
    addAndMakeVisible(advancedSection);
}

void ModDelayComponent::configureKnob(KnobWithLabel& kwl, const juce::String& paramID, const juce::String& labelText, juce::AudioProcessorValueTreeState& state)
{
    kwl.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    kwl.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    kwl.slider.setColour(juce::Slider::thumbColourId, juce::Colour(255, 111, 41));
    kwl.slider.setColour(juce::Slider::trackColourId, juce::Colours::deeppink);
    kwl.slider.setColour(juce::Slider::backgroundColourId, juce::Colour(123, 0, 70));
    kwl.slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::deeppink);
    kwl.slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(90, 0, 50));

    addAndMakeVisible(kwl.slider);

    kwl.label.setText(labelText);
    kwl.label.setReadOnly(true);
    kwl.label.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    kwl.label.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    kwl.label.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    kwl.label.setJustification(juce::Justification::centred);
    addAndMakeVisible(kwl.label);

    kwl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, paramID, kwl.slider);
}

void ModDelayComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(31, 31, 31));
}

void ModDelayComponent::resized()
{
    group.setBounds(getLocalBounds());

	int knobsAmount = 4; // Number of knobs in the main section

    auto area = getLocalBounds().reduced(margin);
    int textSpacing = 6;

    int y = area.getY() + 5;
    int x = area.getX();

    // --- Waveform buttons ---
    int waveX = x;
    for (auto* btn : waveformButtons)
    {
        btn->setBounds(waveX, y, 60, 24);
        waveX += 65;
    }
    y += 15;

    // --- Knobs ---
    KnobWithLabel* knobs[] = { &delayTime, &modDepth, &modRate, &mix };
    for (int i = 0; i < knobsAmount; ++i) {
        knobs[i]->slider.setBounds(x + i * (knobSize + spacing), y, knobSize, knobSize);
        knobs[i]->label.setBounds(x + i * (knobSize + spacing), y - labelHeight, knobSize, labelHeight);
    }

    // --- Advanced section ---
    int advX = y + x + knobsAmount * (knobSize + spacing);
    advancedSection.setBounds(advX, y, getWidth(), 100);
    syncToggle.setBounds(0, 0, 60, 30);
    feedbackL.slider.setBounds(80, 0, knobSize, knobSize);
    feedbackL.label.setBounds(80, -labelHeight, knobSize, labelHeight);
    feedbackR.slider.setBounds(200, 0, knobSize, knobSize);
    feedbackR.label.setBounds(200, -labelHeight, knobSize, labelHeight);
}

void ModDelayComponent::updateWaveformSelection(int index)
{
    if (index < 0 || index >= waveformButtons.size())
        return;

    selectedWaveform = index;
    for (int i = 0; i < waveformButtons.size(); ++i)
        waveformButtons[i]->setColour(juce::TextButton::buttonColourId,
            i == index ? juce::Colours::deeppink : juce::Colours::darkgrey);

    // Do NOT call setModulationType here.
    // Instead, set the invisible ComboBox that owns the state:
    if (auto* comp = findChildWithID("modTypeCombo"))
        if (auto* combo = dynamic_cast<juce::ComboBox*>(comp))
            combo->setSelectedId(index + 1, juce::NotificationType::sendNotification);
}

void ModDelayComponent::setModulationType(ModDelay::ModulationType type)
{
    int index = static_cast<int>(type) - 1;
    if (index == selectedWaveform)
        return; // No change

    updateWaveformSelection(index);
}

void ModDelayComponent::setDelayTime(float v) { delayTime.slider.setValue(v); }
void ModDelayComponent::setFeedbackLeft(float v) { feedbackL.slider.setValue(v); }
void ModDelayComponent::setFeedbackRight(float v) { feedbackR.slider.setValue(v); }
void ModDelayComponent::setMix(float v) { mix.slider.setValue(v); }
void ModDelayComponent::setModDepth(float v) { modDepth.slider.setValue(v); }
void ModDelayComponent::setModRate(float v) { modRate.slider.setValue(v); }