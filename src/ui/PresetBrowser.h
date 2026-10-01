#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../presets/PresetManager.h"

namespace LadderMono
{
    class PresetBrowserOverlay : public juce::Component, public juce::ListBoxModel
    {
    public:
        PresetBrowserOverlay(PresetManager& pm, std::function<void()> onPresetChangedCallback)
            : presetManager(pm), onPresetChanged(std::move(onPresetChangedCallback))
        {
            setAlwaysOnTop(true);

            // Search Box
            searchEditor.setTextToShowWhenEmpty("Search presets by name or tags...", juce::Colour(0xff7c8290));
            searchEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff16171b));
            searchEditor.setColour(juce::TextEditor::textColourId, juce::Colour(0xfff0f2f5));
            searchEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff3a3e49));
            searchEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xffe08b3c));
            searchEditor.setFont(juce::FontOptions(13.5f));
            searchEditor.onTextChange = [this] { updateFilteredList(); };
            searchEditor.onEscapeKey = [this] {
                if (searchEditor.getText().isNotEmpty())
                    searchEditor.setText("", juce::sendNotificationSync);
                clearSearchFocus();
            };
            searchEditor.onReturnKey = [this] {
                if (!filteredIndices.empty())
                {
                    listBox.selectRow(0);
                    listBox.scrollToEnsureRowIsOnscreen(0);
                }
                clearSearchFocus();
            };
            addAndMakeVisible(searchEditor);

            // Close button
            closeBtn.setButtonText(juce::CharPointer_UTF8("\xc3\x97"));
            closeBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22252c));
            closeBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd8dcde));
            closeBtn.onClick = [this] { setVisible(false); };
            addAndMakeVisible(closeBtn);

            // Type filter buttons
            typeButtons = {
                {"All Types", ""},
                {"Bass", "Bass"},
                {"Lead", "Lead"},
                {"Keys", "Keys"},
                {"Pad", "Pad"},
                {"Arp", "Arp"},
                {"Brass", "Brass"},
                {"FX", "FX"}
            };

            for (size_t i = 0; i < typeButtons.size(); ++i)
            {
                auto btn = std::make_unique<juce::TextButton>(typeButtons[i].first);
                btn->setClickingTogglesState(true);
                btn->setRadioGroupId(1001);
                btn->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e2026));
                btn->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffc87a2a));
                btn->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffabb1be));
                btn->setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));
                if (i == 0) btn->setToggleState(true, juce::dontSendNotification);
                btn->onClick = [this, i] {
                    selectedTypeFilter = typeButtons[i].second;
                    updateFilteredList();
                    clearSearchFocus();
                };
                addAndMakeVisible(*btn);
                typeFilterBtns.push_back(std::move(btn));
            }

            // Collection / Pack Filter ComboBox
            collectionBox.addItem("All Collections (Todo)", 1);
            collectionBox.addItem(juce::CharPointer_UTF8("\xe2\x98\x85 Basics (Sonidos B\xc3\xa1sicos)"), 2);
            collectionBox.addSeparator();
            collectionBox.addItem("Pop & Funk 1982", 3);
            collectionBox.addItem("Hip-Hop & Lo-Fi Beats", 4);
            collectionBox.addItem("Psych & Bedroom Pop", 5);
            collectionBox.addItem("French Touch & Electro", 6);
            collectionBox.addItem("Electroclash & Dark Wave", 7);
            collectionBox.addItem("Fantasy & Chiptune", 8);
            collectionBox.addItem("Neon Noir & Italo Disco", 9);
            collectionBox.addItem("80s Cinema & Disco", 10);
            collectionBox.addItem("Cinematic Cyber Noir", 11);
            collectionBox.addItem("1971 Baroque Electronic", 12);
            collectionBox.addItem("Digital Grid & Cyberpunk", 13);
            collectionBox.addItem("Space Pop & Downtempo", 14);
            collectionBox.setSelectedId(1, juce::dontSendNotification);
            collectionBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1e2026));
            collectionBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8eaee));
            collectionBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3a3e49));
            collectionBox.onChange = [this] {
                int id = collectionBox.getSelectedId();
                if (id == 1) selectedCollectionFilter = "";
                else if (id == 2) selectedCollectionFilter = "Basics";
                else selectedCollectionFilter = collectionBox.getText();
                updateFilteredList();
                clearSearchFocus();
            };
            addAndMakeVisible(collectionBox);

            // ListBox
            listBox.setModel(this);
            listBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff121316));
            listBox.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff2d3038));
            listBox.setRowHeight(30);
            addAndMakeVisible(listBox);

            updateFilteredList();
        }

        bool isSearchFocused() const
        {
            return searchEditor.hasKeyboardFocus(true);
        }

        void clearSearchFocus()
        {
            searchEditor.giveAwayKeyboardFocus();
            listBox.grabKeyboardFocus();
        }

        void selectNextPreset()
        {
            if (filteredIndices.empty()) return;
            int cur = listBox.getSelectedRow();
            if (cur < 0) cur = 0;
            else if (cur + 1 < static_cast<int>(filteredIndices.size())) cur++;
            listBox.selectRow(cur);
            listBox.scrollToEnsureRowIsOnscreen(cur);
        }

        void selectPrevPreset()
        {
            if (filteredIndices.empty()) return;
            int cur = listBox.getSelectedRow();
            if (cur <= 0) cur = 0;
            else cur--;
            listBox.selectRow(cur);
            listBox.scrollToEnsureRowIsOnscreen(cur);
        }

        void showBrowser()
        {
            setVisible(true);
            toFront(true);
            updateFilteredList();

            int currentPresetIdx = presetManager.getCurrentPresetIndex();
            int rowToSelect = -1;
            for (size_t i = 0; i < filteredIndices.size(); ++i)
            {
                if (filteredIndices[i] == currentPresetIdx)
                {
                    rowToSelect = static_cast<int>(i);
                    break;
                }
            }
            if (rowToSelect >= 0)
            {
                listBox.selectRow(rowToSelect);
                listBox.scrollToEnsureRowIsOnscreen(rowToSelect);
            }
            else if (!filteredIndices.empty())
            {
                listBox.selectRow(0);
                listBox.scrollToEnsureRowIsOnscreen(0);
            }

            clearSearchFocus();
        }

        void updateFilteredList()
        {
            filteredIndices.clear();
            const auto& all = presetManager.getPresets();
            juce::String query = searchEditor.getText().trim().toLowerCase();

            for (size_t i = 0; i < all.size(); ++i)
            {
                const auto& p = all[i];

                // Type filter
                if (selectedTypeFilter.isNotEmpty() && !p.soundType.equalsIgnoreCase(selectedTypeFilter))
                    continue;

                // Collection filter
                if (selectedCollectionFilter.isNotEmpty() && !p.category.equalsIgnoreCase(selectedCollectionFilter))
                    continue;

                // Search query match
                if (query.isNotEmpty())
                {
                    bool matchName = p.name.toLowerCase().contains(query);
                    bool matchCat = p.category.toLowerCase().contains(query);
                    bool matchType = p.soundType.toLowerCase().contains(query);
                    bool matchTag = false;
                    for (const auto& t : p.tags)
                    {
                        if (t.toLowerCase().contains(query))
                        {
                            matchTag = true;
                            break;
                        }
                    }
                    if (!matchName && !matchCat && !matchType && !matchTag)
                        continue;
                }

                filteredIndices.push_back(static_cast<int>(i));
            }

            listBox.updateContent();
            repaint();
        }

        // ListBoxModel methods
        int getNumRows() override { return static_cast<int>(filteredIndices.size()); }

        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
        {
            if (rowNumber < 0 || rowNumber >= static_cast<int>(filteredIndices.size()))
                return;

            size_t rowIdx = static_cast<size_t>(rowNumber);
            int actualIndex = filteredIndices[rowIdx];
            const auto& p = presetManager.getPresets()[static_cast<size_t>(actualIndex)];
            bool isCurrent = (actualIndex == presetManager.getCurrentPresetIndex());

            if (rowIsSelected)
                g.fillAll(juce::Colour(0xff2a2d36));
            else if (rowNumber % 2 == 1)
                g.fillAll(juce::Colour(0xff16171b));

            if (isCurrent)
            {
                // Amber highlight on active preset
                g.setColour(juce::Colour(0x28e08b3c));
                g.fillRect(0, 0, width, height);

                g.setColour(juce::Colour(0xffe08b3c));
                g.fillRect(0, 0, 4, height);
            }

            // Columns
            // 1. Indicator & Name (x=12, width=240)
            g.setFont(juce::FontOptions(13.0f, isCurrent ? juce::Font::bold : juce::Font::plain));
            g.setColour(isCurrent ? juce::Colour(0xfff5a642) : (rowIsSelected ? juce::Colour(0xffffffff) : juce::Colour(0xffd8dcde)));
            g.drawText(p.name, 14, 0, 230, height, juce::Justification::centredLeft, true);

            // 2. Type Badge (x=250, width=90)
            juce::Colour typeCol = getTypeColour(p.soundType);
            juce::Rectangle<float> typeBadge(252.0f, height * 0.5f - 9.0f, 65.0f, 18.0f);
            g.setColour(typeCol.withAlpha(0.2f));
            g.fillRoundedRectangle(typeBadge, 3.0f);
            g.setColour(typeCol.withAlpha(0.6f));
            g.drawRoundedRectangle(typeBadge, 3.0f, 1.0f);

            g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
            g.setColour(typeCol);
            g.drawText(p.soundType, typeBadge.toNearestInt(), juce::Justification::centred);

            // 3. Collection / Bank (x=330, width=220)
            g.setFont(juce::FontOptions(11.5f));
            if (p.category == "Basics")
            {
                g.setColour(juce::Colour(0xff52b8ff));
                g.drawText(juce::CharPointer_UTF8("\xe2\x98\x85 Basics"), 335, 0, 210, height, juce::Justification::centredLeft, true);
            }
            else
            {
                g.setColour(juce::Colour(0xff9ea4b2));
                g.drawText(p.category, 335, 0, 210, height, juce::Justification::centredLeft, true);
            }

            // 4. Tags (x=560, width = remainder)
            juce::String tagString;
            for (size_t t = 0; t < p.tags.size(); ++t)
            {
                if (t > 0) tagString += ", ";
                tagString += p.tags[t];
            }
            g.setFont(juce::FontOptions(11.0f));
            g.setColour(juce::Colour(0xff686d7a));
            g.drawText(tagString, 560, 0, width - 570, height, juce::Justification::centredLeft, true);
        }

        void selectedRowsChanged(int lastRowSelected) override
        {
            if (lastRowSelected >= 0 && lastRowSelected < static_cast<int>(filteredIndices.size()))
            {
                size_t selIdx = static_cast<size_t>(lastRowSelected);
                int actualIndex = filteredIndices[selIdx];
                presetManager.loadPreset(actualIndex);
                if (onPresetChanged)
                    onPresetChanged();
                listBox.repaint();
                clearSearchFocus();
            }
        }

        void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override
        {
            if (row >= 0 && row < static_cast<int>(filteredIndices.size()))
            {
                size_t rIdx = static_cast<size_t>(row);
                int actualIndex = filteredIndices[rIdx];
                presetManager.loadPreset(actualIndex);
                if (onPresetChanged)
                    onPresetChanged();
                setVisible(false);
            }
        }

        bool keyPressed(const juce::KeyPress& key) override
        {
            if (key == juce::KeyPress::escapeKey)
            {
                setVisible(false);
                return true;
            }
            return false;
        }

        void mouseDown(const juce::MouseEvent& e) override
        {
            // Click outside card closes browser
            auto cardArea = getCardBounds();
            if (!cardArea.contains(e.getPosition()))
            {
                setVisible(false);
            }
            else
            {
                clearSearchFocus();
            }
        }

        void paint(juce::Graphics& g) override
        {
            // Dimmed semi-transparent backdrop
            g.fillAll(juce::Colour(0xc80a0b0e));

            auto card = getCardBounds().toFloat();

            // Card walnut bevel & border
            g.setColour(juce::Colour(0xff3e2415));
            g.fillRoundedRectangle(card.expanded(6.0f), 8.0f);
            g.setColour(juce::Colour(0xffc89240));
            g.drawRoundedRectangle(card.expanded(6.0f), 8.0f, 1.5f);

            // Card body (brushed dark metal)
            juce::ColourGradient cardGrad(juce::Colour(0xff22242a), 0.0f, card.getY(),
                                          juce::Colour(0xff141518), 0.0f, card.getBottom(), false);
            g.setGradientFill(cardGrad);
            g.fillRoundedRectangle(card, 5.0f);
            g.setColour(juce::Colour(0xff444855));
            g.drawRoundedRectangle(card, 5.0f, 1.0f);

            // Brass Title Badge
            juce::Rectangle<float> titleBadge(card.getX() + 24.0f, card.getY() + 14.0f, 220.0f, 28.0f);
            juce::ColourGradient badgeGrad(juce::Colour(0xff242017), 0.0f, titleBadge.getY(),
                                           juce::Colour(0xff12100a), 0.0f, titleBadge.getBottom(), false);
            g.setGradientFill(badgeGrad);
            g.fillRoundedRectangle(titleBadge, 3.0f);
            g.setColour(juce::Colour(0xffc89240));
            g.drawRoundedRectangle(titleBadge, 3.0f, 1.2f);

            g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
            g.setColour(juce::Colour(0xffe89a38));
            g.drawText("P R E S E T   B R O W S E R", titleBadge.toNearestInt(), juce::Justification::centred);

            // Subtitle / Total Presets
            g.setFont(juce::FontOptions(11.5f));
            g.setColour(juce::Colour(0xff8c92a0));
            juce::String countText = "Showing " + juce::String(filteredIndices.size()) + " of "
                                   + juce::String(presetManager.getPresets().size()) + " presets";
            g.drawText(countText, static_cast<int>(card.getRight() - 250.0f), static_cast<int>(card.getY() + 18.0f), 180, 20, juce::Justification::centredRight);

            // Table Headers Banner
            auto listArea = listBox.getBounds();
            juce::Rectangle<int> headerArea(listArea.getX(), listArea.getY() - 22, listArea.getWidth(), 22);
            g.setColour(juce::Colour(0xff18191e));
            g.fillRect(headerArea);
            g.setColour(juce::Colour(0xff363a44));
            g.drawRect(headerArea, 1);

            g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
            g.setColour(juce::Colour(0xff8a90a0));
            g.drawText("PRESET NAME", headerArea.getX() + 14, headerArea.getY(), 230, 22, juce::Justification::centredLeft);
            g.drawText("TYPE", headerArea.getX() + 252, headerArea.getY(), 65, 22, juce::Justification::centred);
            g.drawText("COLLECTION / PACK", headerArea.getX() + 335, headerArea.getY(), 210, 22, juce::Justification::centredLeft);
            g.drawText("TAGS", headerArea.getX() + 560, headerArea.getY(), headerArea.getWidth() - 570, 22, juce::Justification::centredLeft);
        }

        void resized() override
        {
            auto card = getCardBounds();

            // Close button top right
            closeBtn.setBounds(card.getRight() - 36, card.getY() + 12, 24, 24);

            // Search box row
            searchEditor.setBounds(card.getX() + 24, card.getY() + 54, 380, 30);
            collectionBox.setBounds(card.getX() + 416, card.getY() + 54, card.getWidth() - 440, 30);

            // Type filter row
            int btnX = card.getX() + 24;
            int btnY = card.getY() + 94;
            int btnW = (card.getWidth() - 48 - (static_cast<int>(typeFilterBtns.size()) - 1) * 6) / static_cast<int>(typeFilterBtns.size());
            for (auto& btn : typeFilterBtns)
            {
                btn->setBounds(btnX, btnY, btnW, 26);
                btnX += btnW + 6;
            }

            // Results List
            int listY = card.getY() + 154;
            int listH = card.getBottom() - listY - 20;
            listBox.setBounds(card.getX() + 24, listY, card.getWidth() - 48, listH);
        }

    private:
        PresetManager& presetManager;
        std::function<void()> onPresetChanged;

        juce::TextEditor searchEditor;
        juce::TextButton closeBtn;
        juce::ComboBox collectionBox;
        std::vector<std::unique_ptr<juce::TextButton>> typeFilterBtns;
        std::vector<std::pair<juce::String, juce::String>> typeButtons;
        juce::ListBox listBox;

        std::vector<int> filteredIndices;
        juce::String selectedTypeFilter;
        juce::String selectedCollectionFilter;

        juce::Rectangle<int> getCardBounds() const
        {
            int w = std::min(880, getWidth() - 40);
            int h = std::min(550, getHeight() - 40);
            int x = (getWidth() - w) / 2;
            int y = (getHeight() - h) / 2;
            return {x, y, w, h};
        }

        static juce::Colour getTypeColour(const juce::String& type)
        {
            if (type == "Bass") return juce::Colour(0xff4aa3df);       // Blue
            if (type == "Lead") return juce::Colour(0xfff05d5e);       // Crimson / Coral
            if (type == "Keys") return juce::Colour(0xffe0a944);       // Warm Amber
            if (type == "Pad") return juce::Colour(0xffad72d6);        // Purple / Lavender
            if (type == "Arp") return juce::Colour(0xff2ec4b6);        // Teal / Cyan
            if (type == "Brass") return juce::Colour(0xffe68438);      // Orange / Copper
            if (type == "FX") return juce::Colour(0xff88d49e);         // Green
            return juce::Colour(0xffa0a4b0);
        }
    };
}
