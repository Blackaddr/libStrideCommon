/*
 * GuiUtil.cpp
 *
 *  Created on: Jan. 16, 2021
 *      Author: blackaddr
 */
#include <JuceHeader.h>
#include "Resources/BinaryIcons.h"
#include "Util/CommonDefs.h"
#include "Util/AppLookAndFeel.h"
#include "Util/FileUtil.h"
#include "Util/GuiUtil.h"
#include "Util/Gin/imageBlur.h"

using namespace juce;

namespace stride {

Rectangle<int> getDisplaySize() { return  Desktop::getInstance().getDisplays().getMainDisplay().userArea; }

std::string indexToString2(int index)
{
    char indexStr[3];
    snprintf(indexStr, 3, "%02d", index);
    return std::string(indexStr);
}

void setWidthToFitText(juce::Label& label)
{
    int newWidth = label.getFont().getStringWidth(label.getText());
    label.setSize(newWidth,label.getHeight());
}

unsigned getWidthToFitText(const juce::String& text, const juce::Font& font)
{
    return font.getStringWidth(text);
}

unsigned getImageWidthFromNewHeight(Image& imageIn, unsigned &targetHeight, unsigned maxWidth)
{
    if (!imageIn.isValid()) { return 0; }
    float ratio = (float)imageIn.getWidth() / (float)imageIn.getHeight();
    float outputWidth  = targetHeight * ratio;

    if ( (maxWidth != 0) && (outputWidth > maxWidth) ) {
        float reductionRatio = (float)outputWidth / (float)maxWidth;
        outputWidth = static_cast<float>(maxWidth);
        targetHeight = static_cast<unsigned>((float)targetHeight / reductionRatio);
    }
    return static_cast<int>(outputWidth);
}

unsigned getImageWidthFromNewHeight(Image& imageIn, unsigned targetHeight)
{
    if (!imageIn.isValid()) { return 0; }
    float ratio = (float)imageIn.getWidth() / (float)imageIn.getHeight();
    float outputWidth  = targetHeight * ratio;

    return static_cast<int>(outputWidth);
}

unsigned getImageHeightFromNewWidth(Image& imageIn, unsigned &targetWidth, unsigned maxHeight)
{
    if (!imageIn.isValid()) { return 0; }
    float ratio = (float)imageIn.getWidth() / (float)imageIn.getHeight();
    float outputHeight  = targetWidth / ratio;

    if ( (maxHeight != 0) && (outputHeight > maxHeight) ) {
        float reductionRatio = (float)outputHeight / (float)maxHeight;
        outputHeight = static_cast<float>(maxHeight);
        targetWidth = static_cast<unsigned>((float)targetWidth / reductionRatio);
    }
    return static_cast<int>(outputHeight);
}

unsigned getImageHeightFromNewWidth(Image& imageIn, unsigned targetWidth)
{
    if (!imageIn.isValid()) { return 0; }
    float ratio = (float)imageIn.getWidth() / (float)imageIn.getHeight();
    float outputHeight  = targetWidth / ratio;

    return static_cast<int>(outputHeight);
}

HourGlass::HourGlass()
{
    image = ImageCache::getFromMemory(BinaryIcons::AvalonHourGlass_png, BinaryIcons::AvalonHourGlass_pngSize);
    rotation = 0.0f;
}

void HourGlass::reset()
{
    rotation = 0.0f;
}

void HourGlass::advanceRotation(float angle)
{
    rotation += angle;
    if (rotation > 360.0f) {
        rotation = rotation - 360.0f;
    }
}

void HourGlass::paint(Graphics& g) {
    AffineTransform rotateTransform;
    rotateTransform = rotateTransform.rotated(rotation, getWidth()/2.0f, getHeight()/2.0f);
    g.addTransform(rotateTransform);
    g.drawImage(image, 0,0, getWidth(), getHeight(),
                0,0, image.getWidth(), image.getHeight());
}

///////////////////////////////////////
// TextEditWindow
///////////////////////////////////////
TextEditWindow::TextEditWindow()
{
    okButton.setButtonText("OK");
    okButton.setSize(m_buttonWidth, m_buttonHeight);
    okButton.setCentrePosition(static_cast<int>(getWidth() - 1.5f*m_buttonWidth), proportionOfHeight(0.8f));
    okButton.addListener(this);
    addAndMakeVisible(okButton);

    m_cancelButton.setButtonText("CANCEL");
    m_cancelButton.setSize(m_buttonWidth, m_buttonHeight);
    m_cancelButton.setCentrePosition(getWidth() - 3*m_buttonWidth, proportionOfHeight(0.8f));
    m_cancelButton.addListener(this);
    addAndMakeVisible(m_cancelButton);
}

void TextEditWindow::setLabelPtr(Label* labelPtr)
{
    if (labelPtr) {
        m_labelPtr = labelPtr;
        m_labelPtr->setBounds(static_cast<int>(getWidth() *0.1f),
                              static_cast<int>(getHeight()*0.5f),
                              static_cast<int>(getWidth()*0.8f),
                              static_cast<int>(getHeight()*0.2f));
        m_labelPtr->setEditable(true);
        m_labelPtr->setJustificationType(Justification::centred);
        addAndMakeVisible(m_labelPtr);
    }
}
void TextEditWindow::resized()
{
    if (m_labelPtr) {
        m_labelPtr->setBounds(static_cast<int>(getWidth() *0.1f),
                              static_cast<int>(getHeight()*0.5f),
                              static_cast<int>(getWidth()*0.8f),
                              static_cast<int>(getHeight()*0.2f));
    }

    okButton.setCentrePosition(getWidth()/2.0f - 0.6f*m_buttonWidth, proportionOfHeight(0.8f));
    m_cancelButton.setCentrePosition(getWidth()/2.0f + 0.6f*m_buttonWidth, proportionOfHeight(0.8f));
}

void TextEditWindow::paint(Graphics& g)
{
    paintPopoutWindow(g, *this);
    g.setFont(static_cast<int>(getHeight()* 0.2f));
    g.drawFittedText(m_message, 0, getHeight() * 0.2f, getWidth(), getHeight(), Justification::centred,
                     1, // one line of text
                     1.0f); // no horizontal scaling
}

void TextEditWindow::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &okButton) {
        if (m_resultStringPtr && m_labelPtr) {
            *m_resultStringPtr = m_labelPtr->getText().toStdString();
            m_resultStringPtr = nullptr; // reset
        }
        setVisible(false);
    } else if (buttonThatWasClicked == &m_cancelButton) {
        setVisible(false);
    }

    if (m_labelPtr) {
        m_labelPtr->setVisible(false);
        removeChildComponent(m_labelPtr);
        m_labelPtr = nullptr;
    }

}

///////////////////////////////////////
// LicenseAcceptWindow
///////////////////////////////////////
LicenseAcceptWindow::LicenseAcceptWindow()
//: TopLevelWindow("SOFTWARE LICENSE AGREEMENT", true)
{
    m_textEditor.setReadOnly(true);
    m_textEditor.setMultiLine(true, true);
    m_textEditor.setScrollbarsShown(true);
    addAndMakeVisible(m_textEditor);

    acceptButton.setButtonText("ACCEPT");
    acceptButton.setSize(m_buttonWidth, m_buttonHeight);
    acceptButton.setCentrePosition(static_cast<int>(getWidth() - 1.5f*m_buttonWidth), proportionOfHeight(0.8f));
    acceptButton.addListener(this);
    addAndMakeVisible(acceptButton);

    m_exitButton.setButtonText("EXIT");
    m_exitButton.setSize(m_buttonWidth, m_buttonHeight);
    m_exitButton.setCentrePosition(getWidth() - 3*m_buttonWidth, proportionOfHeight(0.8f));
    m_exitButton.addListener(this);
    addAndMakeVisible(m_exitButton);

}

LicenseAcceptWindow::~LicenseAcceptWindow() {

}

void LicenseAcceptWindow::clearText()
{
    m_textEditor.clear();
}

void LicenseAcceptWindow::setText(const juce::String& message)
{
    m_textEditor.setText(message, DONT_NOTIFY);
}

void LicenseAcceptWindow::appendText(const juce::String& message)
{
    m_textEditor.insertTextAtCaret(message);
}

void LicenseAcceptWindow::resized()
{
    constexpr int MARGIN = 10;
    acceptButton.setCentrePosition(getWidth()/2.0f - 0.6f*m_buttonWidth, proportionOfHeight(0.95f));
    m_exitButton.setCentrePosition(getWidth()/2.0f + 0.6f*m_buttonWidth, proportionOfHeight(0.95f));

    m_textEditor.setBounds(MARGIN, MARGIN, getWidth() - 2*MARGIN, acceptButton.getY() - 2*MARGIN);
}

void LicenseAcceptWindow::paint(Graphics& g)
{
    paintPopoutWindow(g, *this);
}

void LicenseAcceptWindow::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &acceptButton) {
        m_isAccepted = true;
    } else if (buttonThatWasClicked == &m_exitButton) {
        m_isAccepted = false;
    }
    setVisible(false);
    if (isCurrentlyModal()) { exitModalState (0); }
    getTopLevelComponent()->setVisible(false);
    //delete this;
}

///////////////////////////////////////
// BlockingWindow
///////////////////////////////////////
JUCE_IMPLEMENT_SINGLETON(BlockingWindowSingleton)
BlockingWindow::BlockingWindow() {
    m_blockingColour = Colour((uint8_t)128,(uint8_t)128,(uint8_t)128,0.40f);  // translucent grey
}

void BlockingWindow::paint(juce::Graphics &g) {

    if (m_parentPtr && m_imagePtr) {
        g.fillAll(Colour());
    } else if (!m_blurEnabled) {
        g.fillAll(m_blockingColour);
    }

    if (m_imagePtr) {
        if (!m_imagePtr->isValid()) {
            g.drawSingleLineText("Invalid image file", 20,20);
            return;
        }
        g.drawImageWithin(*m_imagePtr, 0,0, getWidth(), getHeight(), RectanglePlacement::centred);
    }
}

void BlockingWindow::visibilityChanged()
{
    if (isVisible() && m_parentPtr && !m_imagePtr && m_blurEnabled) {
        Rectangle<int> area = Rectangle<int>(0,0,getWidth(), getHeight());
        Image img = m_parentPtr->createComponentSnapshot(area);
        applyStackBlur(img, m_blurRadius);
        m_imagePtr = std::make_shared<juce::Image>(img);
    }
    else if (!isVisible() && m_parentPtr) {
         m_imagePtr = nullptr;
    }
    Component::visibilityChanged();
}

// ComponentWithBlocking
void ComponentWithBlocking::setPositionRelative(Component* relativePtr, int xPos,  int yPos)
{
    Rectangle<int> relativeLocation = relativePtr->getScreenBounds();
    Rectangle<int> parentLocation = getParentComponent()->getScreenBounds();
    int xOrigin = relativeLocation.getX() - parentLocation.getX();
    int yOrigin = relativeLocation.getY() - parentLocation.getY();

    setTopLeftPosition(xOrigin + xPos, yOrigin + yPos);
}

// InfoWindow
InfoWindow::InfoWindow()
{
    okButton.setButtonText("OK");
    okButton.setSize(m_buttonWidth, m_buttonHeight);
    okButton.setCentrePosition(proportionOfWidth(0.5f), proportionOfHeight(0.8f));
    okButton.addListener(this);

    userButton.setButtonText("OK");
    userButton.setSize(m_buttonWidth, m_buttonHeight);
    userButton.setCentrePosition(proportionOfWidth(0.75f), proportionOfHeight(0.8f));
    userButton.addListener(this);

    textEditor.setSize(getWidth(), okButton.getY());
    addChildComponent(textEditor);
    addAndMakeVisible(okButton);
    addChildComponent(userButton); // by default not visible
}

void InfoWindow::setMessage(const juce::String& message)
{
    if (m_useTextEditor) {
        textEditor.setText(message);
        textEditor.setFont(static_cast<int>(getHeight()* fontSizeHeightRatio));
        textEditor.setColour(TextEditor::textColourId, textColour);
    } else {
        m_message = juce::String(message);
    }
}

void InfoWindow::setUseTextEditor(bool useTextEditor)
{
    m_useTextEditor = useTextEditor;
    if (m_useTextEditor) {
        textEditor.setFont(static_cast<int>(getHeight()* fontSizeHeightRatio));
        textEditor.setColour(TextEditor::textColourId, textColour);
        textEditor.setMultiLine(true, true);
        textEditor.setVisible(true);
    } else {
        textEditor.setVisible(false);
    }
}

void InfoWindow::resized()
{
    if (userButton.isVisible()) {
        okButton.setCentrePosition(proportionOfWidth(0.65), proportionOfHeight(1.0f -m_ratioFromBottom));
        userButton.setCentrePosition(proportionOfWidth(0.35), proportionOfHeight(1.0f -m_ratioFromBottom));
    } else {
        okButton.setCentrePosition(proportionOfWidth(0.5f), proportionOfHeight(1.0f -m_ratioFromBottom));
    }

    textEditor.setSize(getWidth(), getHeight());
}

void InfoWindow::paint(Graphics& g)
{
    paintBasicWindowRounded(g, *this, backgroundColour, borderColour);
    if (m_useTextEditor) {

    } else {
        g.setFont(static_cast<int>(getHeight()* fontSizeHeightRatio));
        g.setColour(textColour);
        g.drawFittedText(m_message, 0, static_cast<int>(getHeight() * 0.2f), getWidth(), getHeight(), m_justify,
                        numLines, // one line of text
                        1.0f); // no horizontal scaling
    }
    g.resetToDefaultState();
}

void InfoWindow::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &okButton) {
        if (m_animatorPtr) { m_animatorPtr->cancelAnimation(this, false); }
        if (m_blockingWindowPtr) { m_blockingWindowPtr->setVisible(false); }
        m_message = "";
        setVisible(false);
    }
}

void ComponentAnimatorDelay::fadeOut(juce::Component* ptr, unsigned fadeTimeMs, unsigned fadeDelayMs)
{
    stopTimer();
    if (!ptr) { return; }
    startTimer(fadeDelayMs);
    m_fadeTimeMs = fadeTimeMs;
    m_componentPtr = ptr;
    m_componentPtr->setAlpha(1.0f);
    m_isFading = false;
}

void ComponentAnimatorDelay::cancelAnimation(bool moveComponentToItsFinalPosition)
{
    if(!m_componentPtr) { return; }
    stopTimer();
    m_isFading = false;
    m_componentAnimator.cancelAnimation(m_componentPtr, moveComponentToItsFinalPosition);
    m_componentPtr->setAlpha(1.0f);
}

void ComponentAnimatorDelay::timerCallback()
{
    stopTimer();
    if (!m_componentPtr) { return; }
    if (m_isFading) {
        // when the timer fires in the fading mode, the animation is done and set the component invisible
        m_componentPtr->setVisible(false);
        m_componentPtr->setAlpha(1.0f);
    } else {
        // the delay phase is finished, set a new timer for when the fade is done so we can set it invisible then.
        m_isFading = true;
        startTimer(m_fadeTimeMs);
        m_componentAnimator.animateComponent(m_componentPtr, m_componentPtr->getBounds(), 0.0f, m_fadeTimeMs, false, 1.0, 1.0);
    }
}

void InfoWindowAnimated::fadeOut(unsigned fadeTimeMs, unsigned fadeDelayMs)
{
    m_animator.fadeOut(this, fadeTimeMs, fadeDelayMs);
}

void InfoWindowAnimated::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &okButton) {
        m_animator.cancelAnimation(false);
        setAlpha(1.0f);
        InfoWindow::buttonClicked(buttonThatWasClicked);
    }
}

//////////////////////////////////////////////////////////////////
// OkCancelWindow
//////////////////////////////////////////////////////////////////
// OkCancelWindow::OkCancelWindow()
// : InfoWindow()
// {
//     cancelButton.setButtonText("CANCEL");
//     cancelButton.setSize(m_buttonWidth, m_buttonHeight);
//     cancelButton.setCentrePosition(proportionOfWidth(0.66f), proportionOfHeight(0.8f));
//     cancelButton.addListener(this);
//     addAndMakeVisible(cancelButton);

//     // reposition the okButton
//     okButton.setCentrePosition(proportionOfWidth(0.33f), proportionOfHeight(0.8f));

//     okSelected = false;
// }

// OkCancelWindow::OkCancelWindow(const juce::String& message)
// : OkCancelWindow()
// {
//     setMessage(message);
// }

// void OkCancelWindow::resized()
// {
//     okButton.setCentrePosition(proportionOfWidth(0.33f), proportionOfHeight(1.0f -m_ratioFromBottom));
//     okButton.setCentrePosition(proportionOfWidth(0.66f), proportionOfHeight(1.0f -m_ratioFromBottom));
//     textEditor.setSize(getWidth(), getHeight());
// }

// void OkCancelWindow::paint(Graphics& g)
// {
//     InfoWindow::paint(g);
//     // paintBasicWindowRounded(g, *this, backgroundColour, borderColour);
//     // if (m_useTextEditor) {

//     // } else {
//     //     g.setFont(static_cast<int>(getHeight()* fontSizeHeightRatio));
//     //     g.setColour(textColour);
//     //     g.drawFittedText(m_message, 0, static_cast<int>(getHeight() * 0.2f), getWidth(), getHeight(), m_justify,
//     //                     numLines, // one line of text
//     //                     1.0f); // no horizontal scaling
//     // }
//     // g.resetToDefaultState();
// }

// void OkCancelWindow::buttonClicked (Button* buttonThatWasClicked)
// {
//     if ((buttonThatWasClicked == &okButton) || (buttonThatWasClicked == &cancelButton)) {
//         if (m_blockingWindowPtr) { m_blockingWindowPtr->setVisible(false); }
//         m_message = "";
//         setVisible(false);
//         if (buttonThatWasClicked == &okButton) { okSelected = true; }
//         else { okSelected = false; }
//     }
// }

//////////////////////////////////////////////////////////////////
// FileViewer Stuff
//////////////////////////////////////////////////////////////////
OptionalScopedPointer<FileViewer> FileViewerParent::createInfo(juce::Colour backgroundColour, juce::Colour borderColour, juce::Colour textColour) {

    OptionalScopedPointer<FileViewer> infoComponentPtr =
        OptionalScopedPointer<FileViewer>(new FileViewer(backgroundColour, borderColour, textColour), false);
    infoComponentPtr->setSize(1, 1);  // initialize to some valid, non-zero size
    return infoComponentPtr;
}

void FileViewerParent::createPointer(const juce::String &title) {
    m_fileViewerPtr = std::make_unique<FileViewerWindow>(this, title);
}

void FileViewerParent::createPointer(const juce::String &title, juce::Colour backgroundColour, juce::Colour borderColour, juce::Colour textColour)
{
    m_fileViewerPtr = std::make_unique<FileViewerWindow>(this, title);
}

void FileViewerParent::clearPointer() { m_fileViewerPtr = nullptr; }

FileViewerWindow::FileViewerWindow(FileViewerParent* parent, const String &title)
: DialogWindow(title, Colour(), true) {
    m_fileViewerParent = parent;
}

FileViewer* FileViewerWindow::getFileViewer() {
    return dynamic_cast<FileViewer*>(getContentComponent());
}

void FileViewerWindow::closeButtonPressed() {
    exitModalState (0);
    if (m_fileViewerParent) { m_fileViewerParent->clearPointer(); }  // will cause the object to be deleted
}

FileViewer::FileViewer(juce::Colour backgroundColour, juce::Colour borderColour, juce::Colour textColour)
: backgroundColour(backgroundColour), borderColour(borderColour), textColour(textColour)
{
    m_editor.setMultiLine(true, false);
    m_editor.setScrollbarsShown(true);
    m_editor.setReadOnly(true);
    m_editor.setSize(getWidth(), getHeight()); // set to size of parent
    m_editor.setColour(TextEditor::backgroundColourId, backgroundColour);
    m_editor.setColour(TextEditor::textColourId, textColour);
    addAndMakeVisible(m_editor);
}

void FileViewer::loadFile(const std::string& filePath)
{
    if (FileUtil::fileExists(filePath)) {
        std::string fileContents;
        int status = FileUtil::readFileToString(filePath, fileContents);
        if (status == SUCCESS) {
            m_editor.setText(String(fileContents));
        }
    }
}

void FileViewer::setEditorText(std::string& text) { m_editor.insertTextAtCaret(juce::String(text)); }

void FileViewer::resized()
{
    m_editor.setSize(getWidth(), getHeight()); // set to size of parent
}

void FileViewer::paint(Graphics& g)
{
    paintBasicWindowRounded(g, *this, backgroundColour, borderColour);
}
///////////////////////////////////////////////////////////////////////////

void MoveableLabel::componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
{
    setSize(1.5*getFont().getStringWidth(getText()), getFont().getHeight());
    if (m_justification == Justification::centredBottom)
    {
        int xCentre = component.getX() + component.getWidth()/2;
        //int yCentre = component.getY() + component.getHeight()/2;
        setCentrePosition(xCentre , component.getBottom() + getFont().getHeight()/2);
    }

    else
    {
        int xCentre = component.getX() + component.getWidth()/2;
        setCentrePosition(xCentre , component.getY() - getFont().getHeight());
    }

}


DisableStatusButton::DisableStatusButton()
{
    setColour(TextButton::buttonColourId, Colours::grey);
    setColour(TextButton::ColourIds::buttonOnColourId, Colours::lightgreen);
}

void DisableStatusButton::setDisable(bool isDisabled, bool triggerClickEn)
{
    m_isDisabled = isDisabled;
    m_update();
    if (triggerClickEn) { triggerClick(); }
}

void DisableStatusButton::toggleDisabled(bool triggerClickEn)
{
    m_isDisabled = !m_isDisabled;
    m_update();
    if (triggerClickEn) { triggerClick(); }
}

void DisableStatusButton::m_update() {
    if (m_isDisabled) {
        setColour(TextButton::buttonColourId, Colours::red);

    } else {
        setColour(TextButton::buttonColourId, Colours::darkgrey);
        setColour(TextButton::ColourIds::buttonOnColourId, Colours::lightgreen);
    }

    setToggleState(false, dontSendNotification);
    //triggerClick();
}

void DisableStatusButton::mouseUp(const MouseEvent &e)
{
    if (e.mods.isLeftButtonDown()) {
        m_isDisabled = !m_isDisabled;
        m_update();
        triggerClick();
    }
}

////////////////////////////////////////////////////////////////////////////////
// BlockingFileChooser
////////////////////////////////////////////////////////////////////////////////
BlockingFileChooser::BlockingFileChooser(const String& dialogText, const String& fileFilter,
                                         BlockingWindow& blockingWindow, File startingDir,
                                         bool useNativeWindow)
: FileChooser(dialogText, startingDir, fileFilter, useNativeWindow), m_blockingWindow(blockingWindow),
  m_fileFilter(fileFilter)
{

}

juce::String BlockingFileChooser::getFileOpenPath()
{
    String returnString;

    m_blockingWindow.setVisible(true);

    if (this->browseForFileToOpen())
    {
        File inputFile(this->getResult());
        returnString = inputFile.getFullPathName();
    }

    m_blockingWindow.setVisible(false);
    return returnString;
}

Array<String> BlockingFileChooser::getMultiFileOpenPath()
{
    String returnString;

    m_blockingWindow.setVisible(true);

    Array<String> stringArray;

    if (this->browseForMultipleFilesToOpen())
    {
        Array<File> filesArray = this->getResults();

        for (auto& file : filesArray) {
            stringArray.add(file.getFullPathName());
        }
    }

    m_blockingWindow.setVisible(false);
    return stringArray;
}

// This function will expand directories and recursively return all their children paths
Array<String> BlockingFileChooser::getMultiFileDirExpandOpenPath()
{
    String returnString;

    m_blockingWindow.setVisible(true);

    Array<String> stringArray;

    if (this->browseForMultipleFilesOrDirectories())
    {
        Array<File> filesArray = this->getResults();

        for (auto& file : filesArray) {
            if (file.isDirectory()) {
                Array<File> childFilesArray = file.findChildFiles( File::findFiles, true /* recursive*/,
                    m_fileFilter);
                for( auto& subfile : childFilesArray) {
                    stringArray.add(subfile.getFullPathName());
                }
            } else {
                stringArray.add(file.getFullPathName());
            }

        }
    }

    m_blockingWindow.setVisible(false);
    return stringArray;
}

// This function will return directores as the full directory path
Array<String> BlockingFileChooser::getMultiFileDirOpenPath()
{
    String returnString;

    m_blockingWindow.setVisible(true);

    Array<String> stringArray;

    if (this->browseForMultipleFilesOrDirectories())
    {
        Array<File> filesArray = this->getResults();

        for (auto& file : filesArray) {
            stringArray.add(file.getFullPathName());
        }
    }

    m_blockingWindow.setVisible(false);
    return stringArray;
}

// This function will return directores as the full directory path
// it will not return files
Array<String> BlockingFileChooser::getMultiDirOpenPath()
{
    String returnString;

    m_blockingWindow.setVisible(true);

    Array<String> stringArray;

    //auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories
    //    FileBrowserComponent:: canSelectMultipleItems;

    if (this->showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectDirectories
                        | FileBrowserComponent::canSelectMultipleItems, nullptr))
    {
        Array<File> filesArray = this->getResults();

        for (auto& file : filesArray) {
            stringArray.add(file.getFullPathName());
        }
    }

    m_blockingWindow.setVisible(false);
    return stringArray;
}

juce::String BlockingFileChooser::getFileSavePath()
{
    String returnString;

    m_blockingWindow.setVisible(true);

    if (this->browseForFileToSave(true /* warn if overwrite */ ))
    {
        File inputFile(this->getResult());
        returnString = inputFile.getFullPathName();
    }

    m_blockingWindow.setVisible(false);
    return returnString;
}

juce::String BlockingFileChooser::browseForDir() {
    String returnString;

    m_blockingWindow.setVisible(true);

    if (this->browseForDirectory())
    {
        File inputFile(this->getResult());
        returnString = inputFile.getFullPathName();
    }

    m_blockingWindow.setVisible(false);
    return returnString;
}
////////////////////////////////////////////////////////////////////////////////

bool stringIsNumber(const juce::String& s)
{
    auto t = s.getCharPointer();
    while (*t)
    {
        if (! t.isDigit())
            return false;

        t++;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// TextBox
////////////////////////////////////////////////////////////////////////////////
const juce::Colour BOX_VALUES_BACKGROUND(64,  64,  64);
const juce::Colour BOX_VALUES_READ_ONLY_BACKGROUND(32,  32,  32);
const juce::Colour BOX_VALUES_READ_ONLY_TEXT_COLOUR(192,192,192);

TextBox::TextBox()
{
    addAndMakeVisible(boxName);
    addAndMakeVisible(boxValue);

    boxValue.setEditable(true, false, false); // edit on single click, loss of focus commits changes
}

void TextBox::paint (juce::Graphics& g)
{
    boxValue.setColour(Label::backgroundColourId, BOX_VALUES_BACKGROUND);
}

void TextBox::resized() {
    boxName.setBounds(0,0, nameWidth, getHeight());
    boxValue.setBounds(boxName.getRight(), 0, getWidth() - boxName.getWidth(), getHeight());
}

void TextBox::setNameWidth(int width) {
    if (width < getWidth()) {

    }
    nameWidth = width;
    resized();
}

void TextBox::mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel)
{
    if (!isNumeric) { return; }
    float incrementVal = 0.0f;
    if ( (wheel.deltaX > 0) || (wheel.deltaY > 0) ) {
        incrementVal = increment;
    } else if ( (wheel.deltaX < 0) || (wheel.deltaY < 0) ) {
        incrementVal = -increment;
    } else {
        return;
    }
    float value = boxValue.getText().getFloatValue() + incrementVal;
    boxValue.setText(String(value), NotificationType::sendNotification);
}

// AnchoredLabel
AnchoredLabel::AnchoredLabel(const String& componentName, const String& labelText)
    : Label(componentName, labelText)
{
}

void AnchoredLabel::paint(Graphics& g)
{
    // Set font and color for text
    g.setFont(getFont());
    g.setColour(findColour(Label::textColourId));

    auto text = getText();
    auto margin = 4;
    auto availableWidth = getWidth() - 2*margin; // Leave some margin if needed
    auto textWidth = g.getCurrentFont().getStringWidth(text);

    // Check if the text fits within the available width
    if (textWidth > availableWidth) {
        // Start trimming the text from the beginning and add an ellipsis
        String trimmedText = text;

        // Gradually shorten the substring from the beginning until it fits
        for (int i = 0; i < text.length(); ++i) {
            String candidateText = "..." + text.substring(i);
            if (g.getCurrentFont().getStringWidth(candidateText) <= availableWidth) {
                trimmedText = candidateText;
                break;
            }
        }

        // Draw the trimmed text with an ellipsis at the beginning
        g.drawText(trimmedText, margin, 0, availableWidth, getHeight(), Justification::centredLeft, true);
    } else {
        // If text fits, draw it normally
        g.drawText(text, margin, 0, availableWidth, getHeight(), Justification::centredLeft, true);
    }
}

///////////////////////
// GetUserDataPrompt1
///////////////////////
GetUserDataPrompt1::GetUserDataPrompt1()
{

}

GetUserDataPrompt1::~GetUserDataPrompt1()
{
    // if (alertWindowPtr) {
    //     delete alertWindowPtr;
    // }
}

static void GetUserDataPrompt1Callback(int modalResult, GetUserDataPrompt1* ptr)
{
    if (modalResult == 1) {
        if (!ptr) { return;}
        if (!ptr->alertWindowPtr) { return; }
        ptr->responseText = ptr->alertWindowPtr->getTextEditorContents("userPrompt").toStdString();
        ptr->alertWindowPtr->exitModalState(0);
        ptr->exited = true;
    }
}

void GetUserDataPrompt1::triggerPrompt()
{
    std::shared_ptr<AlertWindow> alertWindowPtr = std::make_shared<AlertWindow>(windowTitle, windowText, AlertWindow::AlertIconType::QuestionIcon);
    if (!alertWindowPtr) {
        std::cout << "Failed to create alert window" << std::endl;
        return;
    }

    // Add text input fields
    alertWindowPtr->addTextEditor("userPrompt", promptDefaultText);

    alertWindowPtr->addButton("OK", 1);
    alertWindowPtr->addButton("Cancel", 0);
    alertWindowPtr->setVisible(true);

    exited = false;
    alertWindowPtr->enterModalState(true, ModalCallbackFunction::create (GetUserDataPrompt1Callback, this));
}

}  // end of namespace stride
