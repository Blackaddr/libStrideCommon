#include "Util/FileUtil.h"
#include "Util/ErrorMessage.h"
#include "Util/ErrorMessageWindow.h"
#include "Misc/XmlTableWindow.h"

//JUCE_IMPLEMENT_SINGLETON(XmlTable)

XmlTable::XmlTable()
{

}

int XmlTable::deleteTableFile()
{
    return FileUtil::deleteFileIfExists(xmlFilePath);
}

    // this loads the embedded database XML file into memory
int XmlTable::loadData(const std::string& xmlFilename)
{
    std::string filePathToLoad = xmlFilePath;
    if (!xmlFilename.empty()) {
        xmlFilePath = xmlFilename;
    }

    int result;
    if (!FileUtil::fileExists(xmlFilePath)) {
        noteMessage("XmlTable::loadData(): creating Devel table file");
        result = createTableFile();

        if (result != SUCCESS) {
            errorMessage("XmlTable::loadData(): unable to create table XML file");
            return result;
        }
    }

    std::string xmlStr;
    result = FileUtil::readFileToString(xmlFilePath, xmlStr);
    if (result != SUCCESS) {
        return result;
    }

    int selectedRow = -1;
    if (numRows > 0) {
        selectedRow = tableComponent.getSelectedRow();
    }

    tableData = parseXML(String(xmlStr));
    dataList   = tableData->getChildByName ("DATA");
    columnList = tableData->getChildByName ("COLUMNS");
    numRows    = dataList->getNumChildElements();

    tableComponent.updateContent();

    tableComponent.deselectAllRows();
    if (selectedRow < getNumRows() && selectedRow >= 0) {
        tableComponent.selectRow(selectedRow);
    } else if (getNumRows() > 0) {
        tableComponent.selectRow(0);
    }

    return result;
}

void XmlTable::setupColumns()
{
    addAndMakeVisible(tableComponent);
    tableComponent.setModel(this);

    // give it a border
    tableComponent.setColour (ListBox::outlineColourId, Colours::grey);
    tableComponent.setOutlineThickness (1);
    tableComponent.setMultipleSelectionEnabled(false);

    if (!columnList) {
        errorMessage("XmlTable::setupColumns(): columnList is a nullptr");
        return;
    }

    // Add some columns to the table header, based on the column list in our database..
    for (auto* columnXml : columnList->getChildIterator()) {
        tableComponent.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                        columnXml->getIntAttribute ("columnId"),
                                        columnXml->getIntAttribute ("width"),
                                        50, 400,
                                        TableHeaderComponent::defaultFlags);
    }

    // we could now change some initial settings..
    tableComponent.getHeader().setSortColumnId (1, true); // sort forwards by the ID column

    tableComponent.deselectAllRows();
    if (numRows > 0) {
        tableComponent.selectRow(0);
    }
}

void XmlTable::paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour (ListBox::backgroundColourId)
                                            .interpolatedWith (getLookAndFeel().findColour (ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll (Colours::lightblue);
    else if (rowNumber % 2)
        g.fillAll (alternateColour);
}

// This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
// to change the sort order.
void XmlTable::paintCell (Graphics& g, int rowNumber, int columnId,
                int width, int height, bool /*rowIsSelected*/)
{
    g.setColour (getLookAndFeel().findColour (ListBox::textColourId));
    g.setFont (font);

    if (auto* rowElement = dataList->getChildElement (rowNumber))
    {
        auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

        g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
    }

    g.setColour (getLookAndFeel().findColour (ListBox::backgroundColourId));
    g.fillRect (width - 1, 0, 1, height);
}

// This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
// to change the sort order.
void XmlTable::sortOrderChanged (int newSortColumnId, bool isForwards)
{
    if (newSortColumnId != 0)
    {
        DemoDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
        dataList->sortChildElements (sorter);

        tableComponent.updateContent();
    }
}

// This is overloaded from TableListBoxModel, and should choose the best width for the specified
// column.
int XmlTable::getColumnAutoSizeWidth (int columnId)
{
    int widest = 20;

    // find the widest bit of text in this column..
    for (int i = getNumRows(); --i >= 0;)
    {
        if (auto* rowElement = dataList->getChildElement (i))
        {
            auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

            widest = jmax (widest, font.getStringWidth (text));
        }
    }

    return widest + 8;
}

String XmlTable::getText (const int columnNumber, const int rowNumber) const
{
    if ((columnNumber < 0) || (rowNumber < 0) || !dataList) { return String(); }
    XmlElement* element = dataList->getChildElement (rowNumber);
    if (!element) { return String(); }
    return element->getStringAttribute( getAttributeNameForColumnId(columnNumber));
}

void XmlTable::setText (const int columnNumber, const int rowNumber, const String& newText)
{
    if ((columnNumber < 0) || (rowNumber < 0) || !dataList) { return; }
    auto columnName = tableComponent.getHeader().getColumnName (columnNumber);
    XmlElement* element = dataList->getChildElement(rowNumber);
    if (!element) { return; }
    element->setAttribute(columnName, newText);
    //tableComponent.updateContent();
}

void XmlTable::resized()
{
    // position our table with a gap around its edge
    tableComponent.setBoundsInset (BorderSize<int> (8));
}

int XmlTable::writeXmlFile(const std::string& xmlFilename, XmlElement* rootElementPtr) {
    if (rootElementPtr) {
        std::string xmlStr = rootElementPtr->toString().toStdString();
        int result = FileUtil::writeStringToFile(xmlStr, xmlFilename);
        if (result != SUCCESS) {
            std::string msg = std::string("Unable to write to xml file ") + xmlFilename;
            errorMessage(msg);
        }
        return result;
    } else {
        std::string msg = std::string("An unexpected error occurred with the device key table. It appears to be invalid.");
        errorMessage(msg);
        return FAILURE; 
    }
}

int XmlTable::setSelectedRow(int row) {
    if (row < getNumRows()) {
        tableComponent.deselectAllRows();
        tableComponent.selectRow(row);
        return SUCCESS;
    } else { return FAILURE; }

}

void XmlTable::removeEntry(int rowNumber) {
    if (!dataList) { return; }
    if (rowNumber >= numRows) { return; }  // out of range

    // find the row and delete it's element
    XmlElement* elementPtr = dataList->getFirstChildElement();  // row 0
    for (int i=1; i <= rowNumber; i++) {
        elementPtr = elementPtr->getNextElement();
    }
    dataList->removeChildElement(elementPtr, true);
    if (numRows > 0) { numRows--; }
    tableComponent.updateContent();

    // Update the row IDs
    elementPtr = dataList->getFirstChildElement();  // row 0
    for (int i=0; i < numRows; i++) {
        elementPtr->setAttribute(Identifier("ID"), i+1);
        elementPtr = elementPtr->getNextElement();
    }
    tableComponent.updateContent();

    int result = writeXmlFile(xmlFilePath, tableData.get());  // update the xml file

    if (result != SUCCESS) {
        displayErrorMessage(std::string("An error occured while removing the entry from row " + std::to_string(rowNumber)));
    }
}
