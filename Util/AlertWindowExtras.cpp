/*
 * AlertWindowExtras.cpp
 *
 *  Created on: May. 27, 2023
 *      Author: blackaddr
 */

#include "Util/AlertWindowExtras.h"

using namespace juce;

namespace stride {

static AlertWindow* createAlertWindow4Button (const String& title, const String& message,
                                                const String& button1, const String& button2, const String& button3, const String& button4,
                                                MessageBoxIconType iconType,
                                                int numButtons, Component* associatedComponent)
{
    AlertWindow* aw = new AlertWindow (title, message, iconType, associatedComponent);

    if (numButtons == 1)
    {
        aw->addButton (button1, 0,
                       KeyPress (KeyPress::escapeKey),
                       KeyPress (KeyPress::returnKey));
    }
    else
    {
        const KeyPress button1ShortCut ((int) CharacterFunctions::toLowerCase (button1[0]), 0, 0);
        KeyPress button2ShortCut ((int) CharacterFunctions::toLowerCase (button2[0]), 0, 0);
        if (button1ShortCut == button2ShortCut)
            button2ShortCut = KeyPress();

        if (numButtons == 2)
        {
            aw->addButton (button1, 1, KeyPress (KeyPress::returnKey), button1ShortCut);
            aw->addButton (button2, 0, KeyPress (KeyPress::escapeKey), button2ShortCut);
        }
        else if (numButtons == 3)
        {
            aw->addButton (button1, 1, button1ShortCut);
            aw->addButton (button2, 2, button2ShortCut);
            aw->addButton (button3, 0, KeyPress (KeyPress::escapeKey));
        }
        else if (numButtons == 4)
        {
            aw->addButton (button1, 1, button1ShortCut);
            aw->addButton (button2, 2, button2ShortCut);
            aw->addButton (button3, 3, KeyPress());
            aw->addButton (button4, 0, KeyPress (KeyPress::escapeKey));
        }
    }

    return aw;
}

AlertWindow* LookAndFeel_V4Ext::createAlertWindow (const String& title, const String& message,
                                                const String& button1, const String& button2, const String& button3, const String& button4,
                                                MessageBoxIconType iconType,
                                                int numButtons, Component* associatedComponent)
{
    auto boundsOffset = 50;

    auto* aw = createAlertWindow4Button (title, message, button1, button2, button3, button4,
                                                  iconType, numButtons, associatedComponent);

    auto bounds = aw->getBounds();
    bounds = bounds.withSizeKeepingCentre (bounds.getWidth() + boundsOffset, bounds.getHeight() + boundsOffset);
    aw->setBounds (bounds);

    for (auto* child : aw->getChildren())
        if (auto* button = dynamic_cast<TextButton*> (child))
            button->setBounds (button->getBounds() + Point<int> (25, 40));

    return aw;
}

enum class Async { no, yes };

namespace AlertWindowMappingsExtra
{
    using MapFn = int (*) (int);

    static inline int noMapping (int buttonIndex)    { return buttonIndex; }
    static inline int messageBox (int)               { return 0; }
    static inline int okCancel (int buttonIndex)     { return buttonIndex == 0 ? 1 : 0; }
    static inline int yesNoCancel (int buttonIndex)  { return buttonIndex == 2 ? 0 : buttonIndex + 1; }
    static inline int yesNoMaybeCancel (int buttonIndex)  { return buttonIndex == 3 ? 0 : buttonIndex + 1; }

    static std::unique_ptr<ModalComponentManager::Callback> getWrappedCallback (ModalComponentManager::Callback* callbackIn,
                                                                                MapFn mapFn)
    {
        jassert (mapFn != nullptr);

        if (callbackIn == nullptr)
            return nullptr;

        auto wrappedCallback = [innerCallback = rawToUniquePtr (callbackIn), mapFn] (int buttonIndex)
        {
            innerCallback->modalStateFinished (mapFn (buttonIndex));
        };

        return rawToUniquePtr (ModalCallbackFunction::create (std::move (wrappedCallback)));
    }

}

class AlertWindowInfo
{
public:
    AlertWindowInfo (const MessageBoxOptions& opts,
                     std::unique_ptr<ModalComponentManager::Callback>&& cb,
                     Async showAsync)
        : options (opts),
          callback (std::move (cb)),
          async (showAsync)
    {
    }

    int invoke() const
    {
        MessageManager::getInstance()->callFunctionOnMessageThread (showCallback, (void*) this);
        return returnValue;
    }

private:
    static void* showCallback (void* userData)
    {
        static_cast<AlertWindowInfo*> (userData)->show();
        return nullptr;
    }

    void show()
    {
        auto* component = options.getAssociatedComponent();

        // auto& lf = (component != nullptr ? component->getLookAndFeel()
        //                                  : LookAndFeel::getDefaultLookAndFeel());
        //auto lf = LookAndFeel_V4Ext();
		LookAndFeel_V4Ext lf;

        std::unique_ptr<AlertWindow> alertBox (lf.createAlertWindow (options.getTitle(), options.getMessage(),
                                                                     options.getButtonText (0), options.getButtonText (1), options.getButtonText (2), options.getButtonText (3),
                                                                     options.getIconType(), options.getNumButtons(), component));

        jassert (alertBox != nullptr); // you have to return one of these!

        //alertBox->setAlwaysOnTop (juce_areThereAnyAlwaysOnTopWindows());
        alertBox->setAlwaysOnTop (true);

       #if JUCE_MODAL_LOOPS_PERMITTED
        if (async == Async::no)
            returnValue = alertBox->runModalLoop();
        else
       #endif
        {
            ignoreUnused (async);

            alertBox->enterModalState (true, callback.release(), true);
            alertBox.release();
        }
    }

    MessageBoxOptions options;
    std::unique_ptr<ModalComponentManager::Callback> callback;
    const Async async;
    int returnValue = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlertWindowInfo)
};

static int showMaybeAsync (const MessageBoxOptions& options,
                           ModalComponentManager::Callback* callbackIn,
                           AlertWindowMappingsExtra::MapFn mapFn)
{
    const auto showAsync = (callbackIn != nullptr ? Async::yes
                                                  : Async::no);

    auto callback = AlertWindowMappingsExtra::getWrappedCallback (callbackIn, mapFn);

    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        if (showAsync == Async::no)
            return mapFn (NativeMessageBox::show (options));
       #endif

        NativeMessageBox::showAsync (options, callback.release());
        return false;
    }

    AlertWindowInfo info (options, std::move (callback), showAsync);
    return info.invoke();
}

int showYesNoMaybeCancel(MessageBoxIconType iconType,
                                       const String& title,
                                       const String& message,
                                       const String& button1Text,
                                       const String& button2Text,
                                       const String& button3Text,
                                       const String& button4Text,
                                       Component* associatedComponent,
                                       ModalComponentManager::Callback* callback)
{
    return showMaybeAsync (MessageBoxOptions()
                 .withIconType (iconType)
                 .withTitle (title)
                 .withMessage (message)
                 .withButton (button1Text.isEmpty() ? TRANS("YES")    : button1Text)
                 .withButton (button2Text.isEmpty() ? TRANS("NO")     : button2Text)
                 .withButton (button3Text.isEmpty() ? TRANS("MAYBE")  : button3Text)
                 .withButton (button4Text.isEmpty() ? TRANS("CANCEL") : button4Text)
                 .withAssociatedComponent (associatedComponent),
                 callback,
                 LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows()
                               ? AlertWindowMappingsExtra::yesNoCancel
                               : AlertWindowMappingsExtra::noMapping);
}

}
