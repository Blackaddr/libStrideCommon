/*
 * GuiUtil.h
 *
 *  Created on: Jan. 16, 2021
 *      Author: blackaddr
 */

#ifndef GUIUTIL_GUIUTIL_H_
#define GUIUTIL_GUIUTIL_H_

#include <string>
#include <memory>
#include <JuceHeader.h>
#include "Util/CommonDefs.h"
#include "Util/AppLookAndFeel.h"

namespace stride {

juce::Rectangle<int> getDisplaySize();

unsigned getImageWidthFromNewHeight(juce::Image& imageIn, unsigned &targetHeight, unsigned maxWidth);
unsigned getImageWidthFromNewHeight(juce::Image& imageIn, unsigned targetHeight);
unsigned getImageHeightFromNewWidth(juce::Image& imageIn, unsigned &targetWidth, unsigned maxHeight);
unsigned getImageHeightFromNewWidth(juce::Image& imageIn, unsigned targetWidth);

std::string indexToString2(int index);

void setWidthToFitText(juce::Label& label);
unsigned getWidthToFitText(const juce::String& text, const juce::Font& font);

class HourGlass : public juce::Component
{
public:
    HourGlass();
    virtual ~HourGlass() = default;

    void paint(juce::Graphics& g) override;

    void advanceRotation(float angle);
    void reset();
    juce::Image image;
    float       rotation;
};

class TextEditWindow : public juce::Component,
                       public juce::Button::Listener
{
public:
    TextEditWindow();

    void setMessage(const juce::String& message) { m_message = juce::String(message); }
    //void setTextToEdit(const String& textToEdit) { m_label.setText(textToEdit, sendNotification); }

    void setLabelPtr(juce::Label* labelPtr);
    void setResultString(std::string* resultPtr) { m_resultStringPtr = resultPtr; }

    void resized() override;
    void paint(juce::Graphics& g) override;

    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    juce::TextButton okButton; // public in case someone wants to register a listener on this

private:
    juce::TextButton  m_cancelButton;
    juce::String      m_message;
    juce::Label*      m_labelPtr = nullptr;
    std::string*      m_resultStringPtr = nullptr;

    static constexpr int  m_buttonWidth  = 70;
    static constexpr int  m_buttonHeight = 24;
};

class LicenseAcceptWindow : public juce::Component,
                            public juce::Button::Listener
{
public:
    LicenseAcceptWindow();
    virtual ~LicenseAcceptWindow();

    void clearText();
    void setText(const juce::String& message);
    void appendText(const juce::String& message);
    bool isAccepted() { return m_isAccepted; }
    void setPosition(int newIndex) { m_textEditor.setCaretPosition(newIndex); }

    void resized() override;
    void paint(juce::Graphics& g) override;

    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    juce::TextButton acceptButton; // public in case someone wants to register a listener on this

private:
    juce::TextButton  m_exitButton;
    juce::TextEditor  m_textEditor;
    bool              m_isAccepted = false;

    static constexpr int  m_buttonWidth  = 70;
    static constexpr int  m_buttonHeight = 24;
};

class BlockingWindow : public juce::Component
{
public:
    BlockingWindow();
    virtual ~BlockingWindow() = default;

    virtual void paint(juce::Graphics &g) override;
    void mouseDown (const juce::MouseEvent& e) override {}
    void mouseUp   (const juce::MouseEvent &e) override {}
    void mouseDrag (const juce::MouseEvent& e) override {}
    virtual void visibilityChanged() override;
    
    void setParentComponent(juce::Component* parentPtr) { m_parentPtr = parentPtr; }
    void setImage(std::shared_ptr<juce::Image> imagePtr) { m_imagePtr = imagePtr; }
    void setBlurMode(bool isEnabled) { m_blurEnabled = isEnabled; }
    void setBlurRadius(int radius) { m_blurRadius = radius; }
    void setBackgroundColour(juce::Colour colour) { m_blockingColour = colour; }

private:
    juce::Colour m_blockingColour;
    int m_blurRadius = 5;
    bool m_blurEnabled = true;
    juce::Component* m_parentPtr = nullptr;
    std::shared_ptr<juce::Image> m_imagePtr = nullptr;
};

class BlockingWindowClickClearable : public BlockingWindow
{
public:
    void mouseUp (const juce::MouseEvent &e) override { setVisible(false); }
};

class BlockingWindowSingleton : public BlockingWindow
{
public:
    // use PresetManager::getInstance() to get a reference to this singleton
    JUCE_DECLARE_SINGLETON (BlockingWindowSingleton, true);
};

class ComponentWithBlocking : public juce::Component
{
public:
        ComponentWithBlocking() = default;
        virtual ~ComponentWithBlocking() = default;

        void setBlockingWindow(BlockingWindow* blockingWindowPtr) { m_blockingWindowPtr = blockingWindowPtr; }
        void setPositionRelative(Component* relativePtr, int xPos,  int yPos);

        void setWindowIsBlocking(bool isBlocking) {
           if (m_blockingWindowPtr) { m_blockingWindowPtr->setVisible(isBlocking);}
        }

protected:
    BlockingWindow* m_blockingWindowPtr = nullptr;
};

class InfoWindow : public ComponentWithBlocking,
                   public juce::Button::Listener
{
public:
    InfoWindow();
    virtual ~InfoWindow() = default;

    void setMessage(const juce::String& message);
    void setUseTextEditor(bool useTextEditor);

    void setOkButtonPosition(float ratioFromBottom) { m_ratioFromBottom = ratioFromBottom; resized(); }
    void setJustification(juce::Justification justify) { m_justify = justify; }

    void resized() override;
    void paint(juce::Graphics& g) override;

    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    juce::Colour backgroundColour = AAPurple;
    juce::Colour borderColour     = AABlue;
    juce::Colour textColour       = AABlue;

    juce::TextEditor textEditor;

    float    borderThicknessRatio = 0.02f;
    float    fontSizeHeightRatio  = 0.2f;
    unsigned numLines             = 1;

    juce::TextButton okButton; // public in case someone wants to register a listener on this
    juce::TextButton userButton;

protected:

    juce::String     m_message;
    juce::Justification m_justify = juce::Justification::centredTop;

    bool m_useTextEditor = false;

    static constexpr int  m_buttonWidth  = 70;
    static constexpr int  m_buttonHeight = 24;
    float m_ratioFromBottom = 0.2f; 

    juce::ComponentAnimator* m_animatorPtr = nullptr;   
};

class ComponentAnimatorDelay : public juce::Timer
{
public:
    ComponentAnimatorDelay() = default;
    void fadeOut(juce::Component* ptr, unsigned fadeTimeMs, unsigned fadeDelayMs = 0);
    void cancelAnimation(bool moveComponentToItsFinalPosition);
protected:
    void timerCallback() override;
    int m_fadeTimeMs;
    juce::ComponentAnimator m_componentAnimator;
    juce::Component*        m_componentPtr = nullptr;
    bool                    m_isFading     = false;
};

class InfoWindowAnimated : public InfoWindow
{
public:
    InfoWindowAnimated() = default;

    void fadeOut(unsigned fadeTimeMs, unsigned fadeDelayMs = 0);
    void cancelAnimation(bool moveComponentToItsFinalPosition) { m_animator.cancelAnimation(moveComponentToItsFinalPosition); }

    void buttonClicked (juce::Button* buttonThatWasClicked) override;
protected:
    ComponentAnimatorDelay m_animator;
};

// TODO: to make this work, we need it to block the caller until it finishes somehow?
// Maybe easier to change AlertWindow to match our style
// class OkCancelWindow : public InfoWindow
// {
// public:
//     OkCancelWindow();
//     OkCancelWindow(const juce::String& message);
//     juce::TextButton cancelButton; // public in case someone wants to register a listener on this
//     bool okSelected = false;

//     void resized() override;
//     void paint(juce::Graphics& g) override;
//     void buttonClicked (juce::Button* buttonThatWasClicked) override;
// };

class MoveableLabel : public juce::Label {
public:
    MoveableLabel(juce::Justification justification = juce::Justification::centredTop) :m_justification(justification) {}
    void setJustification(juce::Justification justification) { m_justification = justification; }
    void componentMovedOrResized (juce::Component& component, bool wasMoved, bool wasResized) override;
private:
    juce::Justification m_justification;
};

class UnclickableButton : public juce::TextButton
{
    void mouseDown (const juce::MouseEvent& e) override {};
    void mouseUp   (const juce::MouseEvent &e) override {}; // release click
    void mouseDrag (const juce::MouseEvent& e) override {}; // start dragging operation
};

class DisableStatusButton : public juce::TextButton
{
public:
    DisableStatusButton();
    void mouseDown (const juce::MouseEvent& e) override {};
    void mouseUp   (const juce::MouseEvent &e) override; // release click
    void mouseDrag (const juce::MouseEvent& e) override {}; // start dragging operation

    void setDisable(bool isDisabled, bool triggerClickEn = true);
    bool isDisabled() { return m_isDisabled; }
    void toggleDisabled(bool triggerClickEn = true);
private:
    bool m_isDisabled = false;
    void m_update();
};

/// This class provides a wrapper around juce::FileChooser to add support for
/// enabling a BlockingWindow
class BlockingFileChooser : public juce::FileChooser {
public:
    BlockingFileChooser() = delete;
    BlockingFileChooser(const juce::String& dialogText, const juce::String& fileFilter, BlockingWindow& blockingWindow,
            juce::File startingDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory), bool useNativeWindow = true );

    virtual ~BlockingFileChooser() = default;

    juce::String getFileOpenPath();
    juce::String getFileSavePath();
    juce::String browseForDir();

    juce::Array<juce::String> getMultiFileOpenPath();
    juce::Array<juce::String> getMultiFileDirExpandOpenPath(); // directories are expanded into list of child file paths
    juce::Array<juce::String> getMultiFileDirOpenPath();  // directories are returned as their original dir path
    juce::Array<juce::String> getMultiDirOpenPath();  // directories are returned as their original dir path

private:
    BlockingWindow& m_blockingWindow;
    juce::String    m_fileFilter;
};

bool stringIsNumber(const juce::String& s);

// // Usage example
// bool isCancelled = false;
// auto taskLambda = [this, &isCancelled]() {
//     while(true) {
//         if (Thread::currentThreadShouldExit()) { isCancelled = true; break; }
//         // do stuff
//     }
// };
// BackgroundTask task(taskLambda, "Building EFX package, please wait...", true);
// task.runThread();
// if (isCancelled) { return; }
class BackgroundTask : public juce::ThreadWithProgressWindow
{
public:
    BackgroundTask() = delete;
    BackgroundTask(std::function< void(BackgroundTask*)> functionToRun, juce::String windowMessage, bool showCancel = false, bool showProgress = false)
      : juce::ThreadWithProgressWindow (windowMessage, showProgress, showCancel), functionToRun(functionToRun) {}

    void run() override { functionToRun(this); }
    std::function< void(BackgroundTask*)> functionToRun;
};

class ThreadTask : public juce::Thread
{
public:
    ThreadTask() = delete;
    ThreadTask(std::function< void(void*)> functionToRun, const std::string& threadName, size_t stackSize=0, void* argPtr = nullptr)
      : juce::Thread (threadName, stackSize), functionToRun(functionToRun), m_argPtr(argPtr) {}
    virtual ~ThreadTask() = default;

    void run() { functionToRun(m_argPtr); }
    std::function< void(void* argPtr)> functionToRun;
private:
    void* m_argPtr = nullptr;
};

/////////////////////////////////////////////////////
// FileViewer Stuff
/////////////////////////////////////////////////////
class FileViewerWindow;  // forward delcare
class FileViewer;        // forward declare
class FileViewerParent
{
public:
    FileViewerParent()  = default;
    ~FileViewerParent() = default;

    void createPointer(const juce::String &title);
    void createPointer(const juce::String &title, juce::Colour backgroundColour, juce::Colour borderColour, juce::Colour textColour);
    void clearPointer();

    static juce::OptionalScopedPointer<FileViewer> createInfo(juce::Colour backgroundColour, juce::Colour borderColour, juce::Colour textColour);

    std::unique_ptr<FileViewerWindow> m_fileViewerPtr = nullptr;
};

class FileViewerWindow : public juce::DialogWindow
{
public:
    FileViewerWindow(FileViewerParent* parent, const juce::String &title);
    ~FileViewerWindow() = default;

    FileViewer* getFileViewer();

    void closeButtonPressed() override;

    FileViewerParent* m_fileViewerParent = nullptr;
};

class FileViewer : public juce::Component
{
public:
    FileViewer() {}
    FileViewer(juce::Colour backgroundColour, juce::Colour borderColour, juce::Colour textColour);
    ~FileViewer() = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void clear() { m_editor.clear(); }
    void loadFile(const std::string& filePath);
    void setEditorText(std::string& text);

    juce::Colour backgroundColour = stride::AAPurple;
    juce::Colour borderColour     = stride::AABlue;
    juce::Colour textColour       = stride::AABlue;

    float    borderThicknessRatio = 0.02f;
    float    fontSizeHeightRatio  = 0.2f;

private:

    juce::TextEditor  m_editor;
};

class TextBox : public juce::Component {
public:
    TextBox();
    virtual ~TextBox() = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void         setName (const juce::String &name) override { boxName. setText(name, juce::NotificationType::sendNotification); }
    juce::String getName()                         { return boxName.getText(); }

    void         setValue(const juce::String &name, juce::NotificationType notify = DONT_NOTIFY) {
        boxValue.setText(name, notify);
    }
    juce::String getValue()                         { return boxValue.getText(); }

    void setNameWidth(int width);
    int  getNameWidth() { return nameWidth; }

    void setIncrement(float inc) { increment = inc; }
    void setNumeric(bool isNumericIn) { isNumeric = isNumericIn; }

    void setTooltip(const juce::String& tooltip) { boxName.setTooltip(tooltip); boxValue.setTooltip(tooltip); }

    juce::Label boxName;
    juce::Label boxValue;
private:
    int   nameWidth = 120; // default initial width
    bool  isNumeric = false;
    float increment = 1.0f;

    void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
};

struct TooltipArea : public juce::Component, public juce::SettableTooltipClient {};

// Label text is anchored on the right and will draww elipsis if necessary at the front
class AnchoredLabel : public juce::Label
{
public:
    AnchoredLabel(const juce::String& componentName = juce::String(), const juce::String& labelText = juce::String());
    void paint(juce::Graphics& g) override;
};

class GetUserDataPrompt1 {
public:
    GetUserDataPrompt1();
    virtual ~GetUserDataPrompt1();

    void triggerPrompt();
    void setPrompt(const std::string& promptStr) { promptText = promptStr; }
    std::string getResponse() { return responseText; }
    bool isExited() { return exited; }

    std::shared_ptr<juce::AlertWindow> alertWindowPtr = nullptr;
    std::string windowTitle;
    std::string windowText;
    std::string promptText;
    std::string promptDefaultText;
    std::string responseText;
    bool exited = false;
};

}  // end of namespace stride

#endif /* GUIUTIL_GUIUTIL_H_ */
