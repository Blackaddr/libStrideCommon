#include "MainComponent.h"
#include "DevelTable.h"

namespace stride {

JUCE_IMPLEMENT_SINGLETON(DevelTable)

int DevelTable::createTableFile() {

    // create a new file
    XmlElement root("DEVICES");
    XmlElement* columnsPtr = root.createNewChildElement("COLUMNS");
    root.createNewChildElement("DATA");

    XmlElement* elementPtr;
    elementPtr = columnsPtr->createNewChildElement("COLUMN");
    elementPtr->setAttribute(Identifier("columnId"), NAME_COL_ID);
    elementPtr->setAttribute(Identifier("name"), "Name");
    elementPtr->setAttribute(Identifier("width"), NAME_COL_WIDTH);

    elementPtr = columnsPtr->createNewChildElement("COLUMN");
    elementPtr->setAttribute(Identifier("columnId"), UID_COL_ID);
    elementPtr->setAttribute(Identifier("name"), "UID");
    elementPtr->setAttribute(Identifier("width"), UID_COL_WIDTH);

    elementPtr = columnsPtr->createNewChildElement("COLUMN");
    elementPtr->setAttribute(Identifier("columnId"), PROD_COL_ID);
    elementPtr->setAttribute(Identifier("name"), "Product");
    elementPtr->setAttribute(Identifier("width"), PROD_COL_WIDTH);

    elementPtr = columnsPtr->createNewChildElement("COLUMN");
    elementPtr->setAttribute(Identifier("columnId"), REV_COL_ID);
    elementPtr->setAttribute(Identifier("name"), "Rev");
    elementPtr->setAttribute(Identifier("width"), REV_COL_WIDTH);

    elementPtr = columnsPtr->createNewChildElement("COLUMN");
    elementPtr->setAttribute(Identifier("columnId"), PBK_COL_ID);
    elementPtr->setAttribute(Identifier("name"), "pbk");
    elementPtr->setAttribute(Identifier("width"), PBK_COL_WIDTH);

    elementPtr = columnsPtr->createNewChildElement("COLUMN");
    elementPtr->setAttribute(Identifier("columnId"), PVK_COL_ID);
    elementPtr->setAttribute(Identifier("name"), "pvk");
    elementPtr->setAttribute(Identifier("width"), PVK_COL_WIDTH);

    elementPtr = columnsPtr->createNewChildElement("COLUMN");
    elementPtr->setAttribute(Identifier("columnId"), DELETE_BUTTON_COL_ID);
    elementPtr->setAttribute(Identifier("name"), "Delete");
    elementPtr->setAttribute(Identifier("width"), DELETE_BUTTON_COL_WIDTH);

    return writeXmlFile(xmlFilePath, &root);
}

// This is overloaded from TableListBoxModel, and must update any custom components that we're using
// It can customize certain columns for certain behaviors like editable cells and buttons
Component* DevelTable::refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                    Component* existingComponentToUpdate)
{
    if (columnId == NAME_COL_ID) {
        // First column has editable name
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        if (textLabel == nullptr) { textLabel = new EditableTextCustomComponent (*this); }

        textLabel->setRowAndColumn (rowNumber, columnId);
        return textLabel;

    } else if (columnId == DELETE_BUTTON_COL_ID) {
        // Button column
        auto* buttonBox = static_cast<DeleteButtonCustomComponent*> (existingComponentToUpdate);

        // If an existing component is being passed-in for updating, we'll re-use it, but
        // if not, we'll have to create one.
        if (buttonBox == nullptr)
            buttonBox = new DeleteButtonCustomComponent (*this);

        buttonBox->setRowAndColumn (rowNumber, columnId);
        return buttonBox;
    } else {
        jassert (existingComponentToUpdate == nullptr);
        return nullptr;
    }
}

int DevelTable::addDevelKey(DevelKey& develKey) {
    std::string develDevicesFilenameStr = getDevelDevicesFilePath();
    if (develDevicesFilenameStr.empty()) { displayErrorMessage("Unable to get the Devel Table file location"); return FAILURE; }

    if (!FileUtil::fileExists(develDevicesFilenameStr)) {
        // create a new file
        int result = createTableFile();
        if (result != SUCCESS) { displayErrorMessage("Unable to create Device Table file"); return result; }
    }

    std::string uidAlphaKey   = develKey.uid.getAlphaKeyPretty();
    if (uidAlphaKey.empty()) { displayErrorMessage("Unable to decode the Devel Key"); return FAILURE; }

    // Check if this devel already has a key
    bool removeEntries = false;
    if (checkIfDeviceExists(uidAlphaKey)) {
        int row = findDeviceRow(uidAlphaKey);

        if (row >= 0) {  // non-negative rows means it was found
            String deviceIdentifier = getText(NAME_COL_ID,row);
            if (deviceIdentifier.isEmpty()) {
                deviceIdentifier = getText(UID_COL_ID,row);
            }
            if (AlertWindow::showOkCancelBox(MessageBoxIconType::WarningIcon,
                String("Replace Developer Key \"" + deviceIdentifier + "\""),
                String("Do you want to replace the table key with the key from the file?"),
                String("Yes"), String("No"))) {
                    removeEntries = true;
                } else {
                    return SUCCESS;
                }
        }
    }

    // If necessary, loop until all existing entries are removed
    if (removeEntries) {
        bool entryFound;
        int loopLimit = 50;
        do {
            entryFound = false;
            int row = findDeviceRow(uidAlphaKey);
             if (row >= 0) {  // non-negative rows means it was found
                 entryFound = true;
                 removeEntry(row);
                 break;
             }
        } while (entryFound && (loopLimit-- > 0));
    }

    // If we didn't find the devel key, add it.
    XmlElement* elementPtr;

    elementPtr = dataList->createNewChildElement("ITEM");
    if (!elementPtr) { displayErrorMessage("Unable to create new Device Entry"); }
    elementPtr->setAttribute(Identifier("ID"), numRows+1);
    elementPtr->setAttribute(Identifier("UID"), String(uidAlphaKey));
    elementPtr->setAttribute(Identifier("Product"), String(develKey.uid.getProductId()));
    elementPtr->setAttribute(Identifier("Rev"), String(develKey.uid.getRev()));

    // Make a Key string
    std::string dataStr;
    int result = StringUtil::uint8ToStr(develKey.pbk, PUBLIC_KEY_SIZE_BYTES, dataStr);
    if (result != SUCCESS || dataStr.empty()) { displayErrorMessage("An error occured while decoding the Devel keyfile. Cannot add key file."); return FAILURE; }
    elementPtr->setAttribute(Identifier("pbk"), String(dataStr));

    dataStr.clear();
    result = StringUtil::uint8ToStr(develKey.pvk, PRIVATE_KEY_SIZE_BYTES, dataStr);
    if (result != SUCCESS || dataStr.empty()) { displayErrorMessage("An error occured while decoding the Devel keyfile. Cannot add key file."); return FAILURE; }
    elementPtr->setAttribute(Identifier("pvk"), String(dataStr));

    result = writeXmlFile(develDevicesFilenameStr, tableData.get());

    return result;
}

int DevelTable::findDeviceRow(const std::string& uidStrPretty)
{
    int row = -1;
    for (int i=0; i < getNumRows(); i++) {
        if (uidStrPretty == getText(UID_COL_ID, i)) {
            return i;
        }
    }
    return row;
}

bool DevelTable::checkIfDeviceExists(const std::string& uidStrPretty) {
    bool isFound = false;
    for (int i=0; i < getNumRows(); i++) {
        if (uidStrPretty == getText(UID_COL_ID, i)) {
            isFound = true;
        }
    }
    return isFound;
}

///////////////////////////////////////////////////////////////////////////////
//  DevelTable::DeleteButtonCustomComponent
///////////////////////////////////////////////////////////////////////////////
DevelTable::DeleteButtonCustomComponent::DeleteButtonCustomComponent(XmlTable& td, BlockingWindow* blockingWindowPtr)
: ButtonCustomComponent(td, blockingWindowPtr)
{
    button.setImages(false, true, true,
        ImageCache::getFromMemory(BinaryData::delete_png, BinaryData::delete_pngSize), 1.0f,  Colours::transparentBlack,
        ImageCache::getFromMemory(BinaryData::delete_png, BinaryData::delete_pngSize), 0.80f, Colours::transparentBlack,
        ImageCache::getFromMemory(BinaryData::delete_png, BinaryData::delete_pngSize), 0.6f,  Colours::transparentBlack,
        0.0f
    );
}

void DevelTable::DeleteButtonCustomComponent::buttonClicked(Button* buttonThatWasClicked) {

    if (AlertWindow::showOkCancelBox(MessageBoxIconType::WarningIcon,
    String("Delete device \"" + owner.getText(NAME_COL_ID, row) + "\""),  /* name is in the first col */
    String("Do you want to delete this device from the database?"),
    String("Yes"), String("No"))) {
        owner.removeEntry(row);
    }
}

///////////////////////////////////////////////////////////////////////////////
//  DevelTableWindowComponent
///////////////////////////////////////////////////////////////////////////////
DevelTableWindowComponent::DevelTableWindowComponent(DevelTable* DevelTable)
: m_DevelTable(*DevelTable)
    {
    addAndMakeVisible(m_DevelTable);

    addAndMakeVisible(m_addDevelDeviceKeyButton);
    m_addDevelDeviceKeyButton.addListener(this);
    m_addDevelDeviceKeyButton.setImages(false, // do not resize button to fit image
        true, // rescale image when button changes
        true, // maintain aspect ratio
        ImageCache::getFromMemory(BinaryData::add_green_png, BinaryData::add_green_pngSize), 1.0f, Colours::transparentBlack,
        Image(), 1.0f, Colour(),
        Image(), 0.5f, Colour(),
        0);

    addAndMakeVisible(m_clearDevelDeviceKeyButton);
    m_clearDevelDeviceKeyButton.addListener(this);
    m_clearDevelDeviceKeyButton.setImages(false, // do not resize button to fit image
        true, // rescale image when button changes
        true, // maintain aspect ratio
        ImageCache::getFromMemory(BinaryData::clear_png, BinaryData::clear_pngSize), 1.0f, Colours::transparentBlack,
        Image(), 1.0f, Colour(),
        Image(), 0.5f, Colour(),
        0);

    setBlockingWindow(&m_blockingWindow);
    addChildComponent(m_blockingWindow);

    m_DevelTable.loadData();
}

DevelTableWindowComponent::~DevelTableWindowComponent() {
    m_DevelTable.setBlockingWindowPtr(nullptr);
}

void DevelTableWindowComponent::resized() {
    constexpr int MARGIN = 10;
    const int textRowHeight = 36;
    int nextRowStart = MARGIN;
    m_addDevelDeviceKeyButton.setBounds(getWidth() - MARGIN - textRowHeight, nextRowStart, textRowHeight, textRowHeight);
    m_clearDevelDeviceKeyButton.setBounds(m_addDevelDeviceKeyButton.getX() - MARGIN - textRowHeight, nextRowStart, textRowHeight, textRowHeight);
    nextRowStart += textRowHeight + MARGIN;
    m_DevelTable.setBounds(0, nextRowStart, getWidth(), getHeight() - nextRowStart - MARGIN);
    m_blockingWindow.setBounds(0, 0, getWidth(), getHeight());
}

void DevelTableWindowComponent::paint(Graphics &g)
{
    paintBasicWindow(g, *this, Colours::black, AABlue);
}

void DevelTableWindowComponent::buttonClicked (Button* buttonThatWasClicked) {

    if (buttonThatWasClicked == &m_addDevelDeviceKeyButton) {
        setWindowIsBlocking(true);

        std::string fileMask = "*" + std::string(DEVEL_KEY_SUFFIX);
        FileChooser myChooser ("Please select the Devel key files to import...",
                            File::getSpecialLocation (File::userHomeDirectory),
                            fileMask);

        String fileChooseDir;
        unsigned keysAdded = 0;
        if (myChooser.browseForMultipleFilesToOpen())
        {
            Array<File> chooseFileArray(myChooser.getResults());
            for (auto selectedFile : chooseFileArray) {
                String filename = selectedFile.getFullPathName();

                DevelKey develKey = DevelKey::readFile(filename.toStdString());

                if (!develKey.isValid) {
                    std::string msg = filename.toStdString() + " is not a valid Devel Key file";
                    displayErrorMessage(msg);
                    return;
                }

                int result = m_DevelTable.addDevelKey(develKey);
                if (result != SUCCESS) {
                    displayErrorMessage("Invalid Devel key");
                } else {
                    keysAdded++;
                    m_DevelTable.loadData();
                }
            }
        }

        if (keysAdded > 0) {
            std::string pluralStr = (keysAdded > 1) ? " keys" : " key";
            std::string msg = "Successfully added " + std::to_string(keysAdded) + pluralStr + " to the table.";
            displayInfoMessage(msg);
        }

        setWindowIsBlocking(false);
        return;
    }

    else if (buttonThatWasClicked == &m_clearDevelDeviceKeyButton) {
        if (AlertWindow::showOkCancelBox(MessageBoxIconType::WarningIcon,
            String("Delete all Developer keys"),
            String("Are you sure you want to remove all Developer keys from the table?"),
            String("YES"), String("CANCEL"))) {
                m_DevelTable.deleteTableFile();
                m_DevelTable.createTableFile();
                m_DevelTable.loadData();
        }
    }
}

void DevelTableWindow::closeButtonPressed() {
    exitModalState (0);
    if (handlePtr) {
        handlePtr->selfPtr = nullptr;
    }
}

}
