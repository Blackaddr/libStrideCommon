#pragma once

#include "Util/CommonDefs.h"
#include "Misc/XmlTableWindow.h"

namespace stride {

class DevelTable : public XmlTable {
public:

    DevelTable() : XmlTable() {}
    ~DevelTable() = default;

    static constexpr int NAME_COL_ID             = 1;
    static constexpr int UID_COL_ID              = NAME_COL_ID+1;
    static constexpr int PROD_COL_ID             = UID_COL_ID+1;
    static constexpr int REV_COL_ID              = PROD_COL_ID+1;
    static constexpr int PBK_COL_ID              = REV_COL_ID+1;
    static constexpr int PVK_COL_ID              = PBK_COL_ID+1;
    static constexpr int DELETE_BUTTON_COL_ID    = PVK_COL_ID+1;

    static constexpr int NAME_COL_WIDTH          = 150;
    static constexpr int UID_COL_WIDTH           = 250;
    static constexpr int PROD_COL_WIDTH          = 50;
    static constexpr int REV_COL_WIDTH           = 40;
    static constexpr int PBK_COL_WIDTH           = 100;
    static constexpr int PVK_COL_WIDTH           = 100;
    static constexpr int DELETE_BUTTON_COL_WIDTH = 50;


    static constexpr int MINIMUM_WIDTH = NAME_COL_WIDTH + UID_COL_WIDTH + PROD_COL_WIDTH + REV_COL_WIDTH +
                                         PBK_COL_WIDTH + PVK_COL_WIDTH + DELETE_BUTTON_COL_WIDTH + 30;

    int createTableFile() override;
    int getMinimumWidth() override { return MINIMUM_WIDTH; }

    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                    Component* existingComponentToUpdate) override;

    std::string getSelectedUid() { return getText(UID_COL_ID, getSelectedRow()).toStdString(); }
    std::string getSelectedDevelPublicKey()   { return getText(PBK_COL_ID, getSelectedRow()).toStdString(); }
    std::string getSelectedDevelPrivateKey()  { return getText(PVK_COL_ID, getSelectedRow()).toStdString(); }

    int  addDevelKey(DevelKey& develKey) ;
    bool checkIfDeviceExists(const std::string& uidStrPretty);
    int  findDeviceRow(const std::string& uidStrPretty);

    JUCE_DECLARE_SINGLETON (DevelTable, true);

private:

    class DeleteButtonCustomComponent : public ButtonCustomComponent
    {
    public:
        DeleteButtonCustomComponent() = delete;
        DeleteButtonCustomComponent(XmlTable& td, BlockingWindow* blockingWindowPtr = nullptr);
        void buttonClicked(Button* buttonThatWasClicked) override;
    };
};

class DevelTableWindowComponent : public ComponentWithBlocking,
                                   public juce::ImageButton::Listener
{
public:
    DevelTableWindowComponent() = delete;
    DevelTableWindowComponent(DevelTable* DevelTable);

    virtual ~DevelTableWindowComponent();

    void resized() override;

    void paint(Graphics &g) override;

    void buttonClicked (Button* buttonThatWasClicked) override;

private:
    DevelTable&   m_DevelTable;
    ImageButton    m_addDevelDeviceKeyButton;
    ImageButton    m_clearDevelDeviceKeyButton;
    BlockingWindow m_blockingWindow;
};


class DevelTableWindowHandle; // forward declare
class DevelTableWindow : public DialogWindow
{
public:
    DevelTableWindow() = delete;
    DevelTableWindow(DevelTableWindowHandle* handlePtrIn, const String& name, Colour background, bool escClose, bool addToDesktop)
        : DialogWindow(name, background, escClose, addToDesktop), handlePtr(handlePtrIn) {}

    virtual ~DevelTableWindow() = default;

    void closeButtonPressed() override;

    DevelTableWindowHandle* handlePtr = nullptr;
};

class DevelTableWindowHandle {
public:
    std::unique_ptr<DevelTableWindow> selfPtr = nullptr;
};

}
