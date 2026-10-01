#include "PresetManager.h"
#include <juce_core/juce_core.h>

namespace LadderMono
{
    PresetManager::PresetManager(juce::AudioProcessorValueTreeState& state)
        : apvts(state)
    {
    }

    void PresetManager::loadFactoryPresets(const juce::String& jsonContent)
    {
        presets.clear();
        auto parsed = juce::JSON::parse(jsonContent);
        if (auto* array = parsed.getArray())
        {
            for (const auto& item : *array)
            {
                Preset p;
                p.name = item.getProperty("name", "Unnamed").toString();
                p.category = item.getProperty("category", "General").toString();
                p.soundType = item.getProperty("type", "Lead").toString();
                p.author = item.getProperty("author", "Factory").toString();

                if (auto* tagsArr = item.getProperty("tags", juce::var()).getArray())
                {
                    for (const auto& t : *tagsArr)
                        p.tags.push_back(t.toString());
                }

                if (auto* paramObj = item.getProperty("params", juce::var()).getDynamicObject())
                {
                    for (const auto& namedProp : paramObj->getProperties())
                    {
                        p.params[namedProp.name.toString()] = static_cast<float>(namedProp.value);
                    }
                }
                presets.push_back(p);
            }
        }

        if (!presets.empty())
        {
            currentPresetIndex = 0;
            applyPreset(presets[0]);
        }
    }

    juce::String PresetManager::getCurrentPresetName() const noexcept
    {
        if (currentPresetIndex >= 0 && currentPresetIndex < static_cast<int>(presets.size()))
            return presets[static_cast<size_t>(currentPresetIndex)].name;
        return "Custom";
    }

    void PresetManager::loadPreset(int index)
    {
        if (index >= 0 && index < static_cast<int>(presets.size()))
        {
            currentPresetIndex = index;
            applyPreset(presets[static_cast<size_t>(index)]);
        }
    }

    void PresetManager::loadNextPreset()
    {
        if (presets.empty()) return;
        int next = (currentPresetIndex + 1) % static_cast<int>(presets.size());
        loadPreset(next);
    }

    void PresetManager::loadPrevPreset()
    {
        if (presets.empty()) return;
        int prev = (currentPresetIndex - 1 + static_cast<int>(presets.size())) % static_cast<int>(presets.size());
        loadPreset(prev);
    }

    void PresetManager::applyPreset(const Preset& p)
    {
        if (onPresetChanged)
            onPresetChanged();

        // First reset all parameters to their defaults
        for (auto* param : apvts.processor.getParameters())
        {
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                ranged->setValueNotifyingHost(ranged->getDefaultValue());
            }
        }

        // Apply preset overrides
        for (const auto& [paramId, val] : p.params)
        {
            if (auto* param = apvts.getParameter(paramId))
            {
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
                {
                    float normVal = ranged->convertTo0to1(val);
                    ranged->setValueNotifyingHost(normVal);
                }
            }
        }

        if (onPresetChanged)
            onPresetChanged();
    }

    void PresetManager::initPatch()
    {
        for (size_t i = 0; i < presets.size(); ++i)
        {
            if (presets[i].name == "Init Patch")
            {
                loadPreset(static_cast<int>(i));
                return;
            }
        }

        // Default reset if not found
        for (auto* param : apvts.processor.getParameters())
        {
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
                ranged->setValueNotifyingHost(ranged->getDefaultValue());
        }
        currentPresetIndex = -1;
    }

    void PresetManager::saveUserPreset(const juce::String& name, const juce::String& category)
    {
        Preset p;
        p.name = name;
        p.category = category;
        p.author = "User";

        for (auto* param : apvts.processor.getParameters())
        {
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                float actualVal = ranged->convertFrom0to1(ranged->getValue());
                p.params[ranged->paramID] = actualVal;
            }
        }

        presets.push_back(p);
        currentPresetIndex = static_cast<int>(presets.size()) - 1;
    }
}
