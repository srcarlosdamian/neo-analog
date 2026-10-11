#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace LadderMono
{
    struct ThemeColors
    {
        // 1. Surfaces & Backgrounds
        juce::Colour bgChassis       { 0xff08090a }; // Outer main chassis
        juce::Colour panelBg         { 0xff141518 }; // Section faceplate panels
        juce::Colour wellDark        { 0xff090a0d }; // Cutout recessed wells
        juce::Colour panelLight      { 0xff1a1b20 }; // Auxiliary panel lighting
        juce::Colour comboBg         { 0xff181a20 }; // Dropdown combo box & popup background
        juce::Colour buttonBg        { 0xff1c1e24 }; // Button faceplates

        // 2. Lines & Borders (Normalized per user request)
        juce::Colour lineBorder      { 0xff1e2026 }; // Outer frame rims & well borders
        juce::Colour lineAccent      { 0xff2f333e }; // Section header plates & dividing accents

        // 3. Knobs & Controls
        juce::Colour knobBody        { 0xff1f2127 }; // Matte cylinder rotor
        juce::Colour knobCap         { 0xff141519 }; // Center recessed disc
        juce::Colour knobRing        { 0xff2a2c34 }; // Cylinder perimeter ring
        juce::Colour knobIndicator   { 0xffd8dce6 }; // Indicator dot / notch

        // 4. Switches & Performance Wheels
        juce::Colour switchBody      { 0xff1c1d22 }; // Rocker switch & wheel body
        juce::Colour switchActive    { 0xffeaedf4 }; // Active switch LED pip
        juce::Colour switchInactive  { 0xff0e0f12 }; // Inactive dark socket

        // 5. Typography & Geometry
        juce::Colour textPrimary     { 0xffd4d8e2 }; // Main titles, active values, button labels
        juce::Colour textSecondary   { 0xff989ea8 }; // Subtitles, section names, standard body
        juce::Colour textDim         { 0xff555964 }; // Dim hints, inactive switch labels
        float fontScale              { 1.0f };       // Global font scaling (0.85 to 1.30)
        float cornerRadius           { 4.0f };       // Global rounded corner radius (1.0 to 12.0)

        // 6. Screen (OLED)
        juce::Colour screenBg        { 0xff000000 }; // Screen inner glass
        juce::Colour screenBorder    { 0xff1e2026 }; // Screen bezel border
        juce::Colour screenText      { 0xff555964 }; // Standby badge text

        // 7. Virtual Keyboard (Noir All-Black Stealth Style)
        juce::Colour keyWhiteRest    { 0xff141519 }; // Matte black natural keys
        juce::Colour keyWhiteDown    { 0xff2a2d36 }; // Pressed slate
        juce::Colour keyBlackRest    { 0xff08080a }; // Glossy chiseled pitch black
        juce::Colour keyBlackDown    { 0xff1c1e24 };

        juce::String toJSONString() const
        {
            auto obj = std::make_unique<juce::DynamicObject>();
            obj->setProperty("bgChassis", bgChassis.toDisplayString(true));
            obj->setProperty("panelBg", panelBg.toDisplayString(true));
            obj->setProperty("wellDark", wellDark.toDisplayString(true));
            obj->setProperty("panelLight", panelLight.toDisplayString(true));
            obj->setProperty("comboBg", comboBg.toDisplayString(true));
            obj->setProperty("buttonBg", buttonBg.toDisplayString(true));
            obj->setProperty("lineBorder", lineBorder.toDisplayString(true));
            obj->setProperty("lineAccent", lineAccent.toDisplayString(true));
            obj->setProperty("knobBody", knobBody.toDisplayString(true));
            obj->setProperty("knobCap", knobCap.toDisplayString(true));
            obj->setProperty("knobRing", knobRing.toDisplayString(true));
            obj->setProperty("knobIndicator", knobIndicator.toDisplayString(true));
            obj->setProperty("switchBody", switchBody.toDisplayString(true));
            obj->setProperty("switchActive", switchActive.toDisplayString(true));
            obj->setProperty("switchInactive", switchInactive.toDisplayString(true));
            obj->setProperty("textPrimary", textPrimary.toDisplayString(true));
            obj->setProperty("textSecondary", textSecondary.toDisplayString(true));
            obj->setProperty("textDim", textDim.toDisplayString(true));
            obj->setProperty("fontScale", static_cast<double>(fontScale));
            obj->setProperty("cornerRadius", static_cast<double>(cornerRadius));
            obj->setProperty("screenBg", screenBg.toDisplayString(true));
            obj->setProperty("screenBorder", screenBorder.toDisplayString(true));
            obj->setProperty("screenText", screenText.toDisplayString(true));
            obj->setProperty("keyWhiteRest", keyWhiteRest.toDisplayString(true));
            obj->setProperty("keyWhiteDown", keyWhiteDown.toDisplayString(true));
            obj->setProperty("keyBlackRest", keyBlackRest.toDisplayString(true));
            obj->setProperty("keyBlackDown", keyBlackDown.toDisplayString(true));

            juce::var data(obj.release());
            return juce::JSON::toString(data, false);
        }

        bool fromJSONString(const juce::String& jsonText)
        {
            auto parsed = juce::JSON::parse(jsonText);
            auto* obj = parsed.getDynamicObject();
            if (obj == nullptr) return false;

            auto getCol = [&](const juce::Identifier& key, juce::Colour fallback) {
                if (obj->hasProperty(key))
                    return juce::Colour::fromString(obj->getProperty(key).toString());
                return fallback;
            };

            bgChassis     = getCol("bgChassis", bgChassis);
            panelBg       = getCol("panelBg", panelBg);
            wellDark      = getCol("wellDark", wellDark);
            panelLight    = getCol("panelLight", panelLight);
            comboBg       = getCol("comboBg", comboBg);
            buttonBg      = getCol("buttonBg", buttonBg);
            lineBorder    = getCol("lineBorder", lineBorder);
            lineAccent    = getCol("lineAccent", lineAccent);
            knobBody      = getCol("knobBody", knobBody);
            knobCap       = getCol("knobCap", knobCap);
            knobRing      = getCol("knobRing", knobRing);
            knobIndicator = getCol("knobIndicator", knobIndicator);
            switchBody    = getCol("switchBody", switchBody);
            switchActive  = getCol("switchActive", switchActive);
            switchInactive= getCol("switchInactive", switchInactive);
            textPrimary   = getCol("textPrimary", textPrimary);
            textSecondary = getCol("textSecondary", textSecondary);
            textDim       = getCol("textDim", textDim);
            if (obj->hasProperty("fontScale"))
                fontScale = std::clamp(static_cast<float>(obj->getProperty("fontScale")), 0.80f, 1.35f);
            if (obj->hasProperty("cornerRadius"))
                cornerRadius = std::clamp(static_cast<float>(obj->getProperty("cornerRadius")), 0.0f, 14.0f);
            screenBg      = getCol("screenBg", screenBg);
            screenBorder  = getCol("screenBorder", screenBorder);
            screenText    = getCol("screenText", screenText);
            keyWhiteRest  = getCol("keyWhiteRest", keyWhiteRest);
            keyWhiteDown  = getCol("keyWhiteDown", keyWhiteDown);
            keyBlackRest  = getCol("keyBlackRest", keyBlackRest);
            keyBlackDown  = getCol("keyBlackDown", keyBlackDown);
            return true;
        }

        void saveToFile(const juce::File& file) const
        {
            file.replaceWithText(toJSONString());
        }

        bool loadFromFile(const juce::File& file)
        {
            if (!file.existsAsFile()) return false;
            return fromJSONString(file.loadFileAsString());
        }

        static ThemeColors getDefaultNoir()
        {
            return ThemeColors();
        }

        static ThemeColors getCyberCyan()
        {
            ThemeColors t;
            t.bgChassis     = juce::Colour(0xff090c10);
            t.panelBg       = juce::Colour(0xff0f151e);
            t.wellDark      = juce::Colour(0xff06090d);
            t.panelLight    = juce::Colour(0xff141c28);
            t.comboBg       = juce::Colour(0xff101824);
            t.buttonBg      = juce::Colour(0xff141e2a);
            t.lineBorder    = juce::Colour(0xff00d0bc);
            t.lineAccent    = juce::Colour(0xff00a89a);
            t.knobBody      = juce::Colour(0xff121822);
            t.knobCap       = juce::Colour(0xff0b0f16);
            t.knobRing      = juce::Colour(0xff00a89a);
            t.knobIndicator = juce::Colour(0xff00ffe1);
            t.switchBody    = juce::Colour(0xff141c28);
            t.switchActive  = juce::Colour(0xff00ffe1);
            t.textPrimary   = juce::Colour(0xff00ffe1);
            t.textSecondary = juce::Colour(0xff6ee7b7);
            t.textDim       = juce::Colour(0xff134e4a);
            t.screenBg      = juce::Colour(0xff020d0f);
            t.screenBorder  = juce::Colour(0xff00d0bc);
            t.screenText    = juce::Colour(0xff00ffe1);
            return t;
        }

        static ThemeColors getAmberMatrix()
        {
            ThemeColors t;
            t.bgChassis     = juce::Colour(0xff0d0905);
            t.panelBg       = juce::Colour(0xff18120a);
            t.wellDark      = juce::Colour(0xff0a0703);
            t.panelLight    = juce::Colour(0xff22190e);
            t.comboBg       = juce::Colour(0xff1d140a);
            t.buttonBg      = juce::Colour(0xff241a0f);
            t.lineBorder    = juce::Colour(0xffe08b3c);
            t.lineAccent    = juce::Colour(0xffa65e20);
            t.knobBody      = juce::Colour(0xff1c140c);
            t.knobCap       = juce::Colour(0xff100b06);
            t.knobRing      = juce::Colour(0xffa65e20);
            t.knobIndicator = juce::Colour(0xffffa347);
            t.switchBody    = juce::Colour(0xff22190e);
            t.switchActive  = juce::Colour(0xffffa347);
            t.textPrimary   = juce::Colour(0xffffaf59);
            t.textSecondary = juce::Colour(0xffd4843b);
            t.textDim       = juce::Colour(0xff704218);
            t.screenBg      = juce::Colour(0xff120902);
            t.screenBorder  = juce::Colour(0xffe08b3c);
            t.screenText    = juce::Colour(0xffffaf59);
            return t;
        }

        static ThemeColors getMidnightBlue()
        {
            ThemeColors t;
            t.bgChassis     = juce::Colour(0xff080b12);
            t.panelBg       = juce::Colour(0xff0f1422);
            t.wellDark      = juce::Colour(0xff06080e);
            t.panelLight    = juce::Colour(0xff151b2e);
            t.comboBg       = juce::Colour(0xff111728);
            t.buttonBg      = juce::Colour(0xff171f34);
            t.lineBorder    = juce::Colour(0xff3b82f6);
            t.lineAccent    = juce::Colour(0xff2563eb);
            t.knobBody      = juce::Colour(0xff131828);
            t.knobCap       = juce::Colour(0xff0a0e1a);
            t.knobRing      = juce::Colour(0xff2563eb);
            t.knobIndicator = juce::Colour(0xff60a5fa);
            t.switchBody    = juce::Colour(0xff151b2e);
            t.switchActive  = juce::Colour(0xff93c5fd);
            t.textPrimary   = juce::Colour(0xffe0f2fe);
            t.textSecondary = juce::Colour(0xff93c5fd);
            t.textDim       = juce::Colour(0xff1e3a8a);
            t.screenBg      = juce::Colour(0xff030712);
            t.screenBorder  = juce::Colour(0xff3b82f6);
            t.screenText    = juce::Colour(0xff93c5fd);
            return t;
        }

        static ThemeColors getCrimsonStealth()
        {
            ThemeColors t;
            t.bgChassis     = juce::Colour(0xff0d0606);
            t.panelBg       = juce::Colour(0xff180d0d);
            t.wellDark      = juce::Colour(0xff0a0404);
            t.panelLight    = juce::Colour(0xff241313);
            t.comboBg       = juce::Colour(0xff1e0e0e);
            t.buttonBg      = juce::Colour(0xff271212);
            t.lineBorder    = juce::Colour(0xffe11d48);
            t.lineAccent    = juce::Colour(0xff9f1239);
            t.knobBody      = juce::Colour(0xff1a0e0e);
            t.knobCap       = juce::Colour(0xff100808);
            t.knobRing      = juce::Colour(0xff9f1239);
            t.knobIndicator = juce::Colour(0xfffb7185);
            t.switchBody    = juce::Colour(0xff241313);
            t.switchActive  = juce::Colour(0xfffb7185);
            t.textPrimary   = juce::Colour(0xffffe4e6);
            t.textSecondary = juce::Colour(0xfffb7185);
            t.textDim       = juce::Colour(0xff881337);
            t.screenBg      = juce::Colour(0xff0a0202);
            t.screenBorder  = juce::Colour(0xffe11d48);
            t.screenText    = juce::Colour(0xfffb7185);
            return t;
        }
    };
}
