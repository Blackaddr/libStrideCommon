/*
 * AlertWindowExtras.h
 *
 *  Created on: May. 27, 2023
 *      Author: blackaddr
 */

#ifndef GUIUTIL_ALERTWINDOWEXTRAS_H_
#define GUIUTIL_ALERTWINDOWEXTRAS_H_

#include <JuceHeader.h>

namespace stride {

class JUCE_API LookAndFeel_V4Ext   : public juce::LookAndFeel_V4
{
public:
    LookAndFeel_V4Ext() : juce::LookAndFeel_V4() {}
	~LookAndFeel_V4Ext() = default;

    using juce::LookAndFeel_V4::createAlertWindow;  // suppress hidden virtual warning
    juce::AlertWindow* createAlertWindow (const juce::String& title, const juce::String& message,
                                    const juce::String& button1,
                                    const juce::String& button2,
                                    const juce::String& button3,
                                    const juce::String& button4,
                                    juce::MessageBoxIconType iconType,
                                    int numButtons, juce::Component* associatedComponent);
};

int showYesNoMaybeCancel(juce::MessageBoxIconType iconType,
    const juce::String& title,
    const juce::String& message,
    const juce::String& button1Text,
    const juce::String& button2Text,
    const juce::String& button3Text,
    const juce::String& button4Text,
    juce::Component* associatedComponent=nullptr,
    juce::ModalComponentManager::Callback* callback=nullptr);

}

#endif