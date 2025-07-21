#pragma once

#include <JuceHeader.h>
#include "Util/CommonDefs.h"
#include "Util/FileUtil.h"
#include "Util/StringUtil.h"
#include "Util/ErrorMessage.h"
#include "Util/ErrorMessageWindow.h"
#include "Util/Keys.h"
#include "Util/GuiUtil.h"

using namespace juce;
using namespace stride;

class XmlTableListBox : public TableListBox
{
public:
    XmlTableListBox() = default;
};

class XmlTable : public Component,
                 public TableListBoxModel
{
public:
    XmlTable();
    virtual ~XmlTable() = default;

    // After constructor, proper use is
    // 1) setXmlFilepath() to set the filename
    // 2) loadData()
    // 3) setupColumns()
    // for subsequency calls to loadData(), do not call setupColumns()
    void setXmlFilepath(const std::string& filepath) { xmlFilePath = filepath; }
    virtual int loadData(const std::string& xmlFilename = std::string());
    virtual void setupColumns();

    // This is overloaded from TableListBoxModel, and must return the total number of rows in our table
    int getNumRows() override { return numRows; }
    void getNumRows (int newSortColumnId, bool isForwards);

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;

    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override;

    // This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
    // to change the sort order.
    void sortOrderChanged (int newSortColumnId, bool isForwards) override;

    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    // It can customize certain columns for certain behaviors like editable cells and buttons
    // It will return the component that is updated if one is found
    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override { return nullptr; }

    // This is overloaded from TableListBoxModel, and should choose the best width for the specified
    // column.
    int getColumnAutoSizeWidth (int columnId) override;

    String getText (const int columnNumber, const int rowNumber) const;

    void setText (const int columnNumber, const int rowNumber, const String& newText);

    //==============================================================================
    void resized() override;

    void setBlockingWindowPtr(BlockingWindow* blockingWindowPtr) { m_blockingWindowPtr = blockingWindowPtr; }

    static int writeXmlFile(const std::string& xmlFilename, XmlElement* rootElementPtr);

    virtual int createTableFile() = 0;
    virtual int deleteTableFile();
    virtual int getMinimumWidth() = 0;  // you must override to set the table width

    int setSelectedRow(int row);

    int getSelectedRow() { return tableComponent.getSelectedRow(); }

    Component* getCell(int col, int row) { return tableComponent.getCellComponent(col, row); }

    XmlElement* getDataList() { return dataList; }
    XmlElement* getColumnList() { return columnList; }

    void updateContents() { tableComponent.updateContent(); }

    virtual void removeEntry(int rowNumber);

protected:
    std::string xmlFilePath;
    XmlTableListBox tableComponent;     // the table component itself
    BlockingWindow* m_blockingWindowPtr = nullptr;
    Font font  { 14.0f };

    std::unique_ptr<XmlElement> tableData;  // This is the XML document loaded from the embedded file "demo table data.xml"
    XmlElement* columnList = nullptr;     // A pointer to the sub-node of xmlData that contains the list of columns
    XmlElement* dataList   = nullptr;     // A pointer to the sub-node of xmlData that contains the list of data rows
    int numRows;                          // The number of rows of data we've got

    //==============================================================================
    // This is a custom Label component, which we use for the table's editable text columns.
    class EditableTextCustomComponent  : public Label
    {
    public:
        EditableTextCustomComponent (XmlTable& td)  : owner (td)
        {
            // double click to edit the label text; single click handled below
            setEditable (false, true, false);
        }

        void mouseDown (const MouseEvent& event) override
        {
            // single click on the label should simply select the row
            owner.tableComponent.selectRowsBasedOnModifierKeys (row, event.mods, false);

            Label::mouseDown (event);
        }

        void textWasEdited() override
        {
            owner.setText (columnId, row, getText());
            owner.writeXmlFile(owner.xmlFilePath, owner.tableData.get());
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), dontSendNotification);
        }

        void paint (Graphics& g) override
        {
            auto& lf = getLookAndFeel();
            if (! dynamic_cast<LookAndFeel_V4*> (&lf))
                lf.setColour (textColourId, Colours::black);

            Label::paint (g);
        }

    private:
        XmlTable& owner;
        int row, columnId;
        Colour textColour;
    };
    friend EditableTextCustomComponent;

    class ButtonCustomComponent : public Component, public Button::Listener
    {
    public:
        ButtonCustomComponent() = delete;
        ButtonCustomComponent(XmlTable& td, BlockingWindow* blockingWindowPtr)
        : owner (td), blockingWindowPtr(blockingWindowPtr)
        {
            addAndMakeVisible(button);
            button.addListener(this);
        }

        void resized() override
        {
            button.setBoundsInset (BorderSize<int> (2));
        }

        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
        }
    protected:
        XmlTable& owner;
        BlockingWindow* blockingWindowPtr = nullptr;
        ImageButton button;
        int row, columnId;
    };

    //==============================================================================
    // A comparator used to sort our data when the user clicks a column header
    class DemoDataSorter
    {
    public:
        DemoDataSorter (const String& attributeToSortBy, bool forwards)
            : attributeToSort (attributeToSortBy),
              direction (forwards ? 1 : -1)
        {
        }

        int compareElements (XmlElement* first, XmlElement* second) const
        {
            auto result = first->getStringAttribute (attributeToSort)
                                .compareNatural (second->getStringAttribute (attributeToSort));

            if (result == 0)
                result = first->getStringAttribute ("ID")
                               .compareNatural (second->getStringAttribute ("ID"));

            return direction * result;
        }

    private:
        String attributeToSort;
        int direction;
    };

    //==============================================================================

    // (a utility method to search our XML for the attribute that matches a column ID)
    String getAttributeNameForColumnId (const int columnId) const
    {
        for (auto* columnXml : columnList->getChildIterator())
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("name");
        }

        return {};
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XmlTable)
};
