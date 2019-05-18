#include "src/editors/looknfeel/LookNFeelCodeMode.h"
#include "src/editors/looknfeel/LookNFeelEditor.h"

LookNFeelCodeMode::LookNFeelCodeMode(LookNFeelEditor& editor)
    : CodeEditMode(editor)
{
/*
        self.highlighter = WidgetLookHighlighter(self)
*/
}

// Returns the Look n' Feel XML string based on all WidgetLookFeels that belong to the Look n' Feel file according to the editor
QString LookNFeelCodeMode::getNativeCode()
{
    LookNFeelEditor& editor = static_cast<LookNFeelEditor&>(_editor);

/*
        // We add every WidgetLookFeel name of this Look N' Feel to a StringSet
        nameSet = editor.getStringSetOfWidgetLookFeelNames()
        // We parse all WidgetLookFeels as XML to a string
        lookAndFeelString = PyCEGUI.WidgetLookManager.getSingleton().getWidgetLookSetAsString(nameSet)

        return editor.unmapWidgetLookReferences(lookAndFeelString)
*/

    assert(false);
    return "";
}

bool LookNFeelCodeMode::propagateNativeCode(const QString& code)
{
    LookNFeelEditor& editor = static_cast<LookNFeelEditor&>(_editor);

/*
        # we have to make the context the current context to ensure textures are fine
        mainwindow.MainWindow.instance.ceguiContainerWidget.makeGLContextCurrent()

        loadingSuccessful = editor.tryUpdatingWidgetLookFeel(code)
        editor.visual.updateToNewTargetWidgetLook()

        return loadingSuccessful
*/

    assert(false);
    return false;
}

// Refreshes this Code editing mode with current native source code and moves to and selects the WidgetLookFeel code
void LookNFeelCodeMode::refreshFromVisual()
{
    LookNFeelEditor& editor = static_cast<LookNFeelEditor&>(_editor);

    if (!editor.getTargetWidgetLook().isEmpty())
    {
        // Refresh the WidgetLookFeel Highlighter based on the new name of the WidgetLook
        /*
        auto originalWidgetLookName = editor.unmapMappedNameIntoOriginalParts(editor.getTargetWidgetLook());
        highlighter.updateWidgetLookRule(originalWidgetLookName);
        */
    }

    CodeEditMode::refreshFromVisual();

    if (!editor.getTargetWidgetLook().isEmpty())
    {
        // Refresh the WidgetLookFeel Highlighter based on the new name of the WidgetLook
        /*
        auto originalWidgetLookName = editor.unmapMappedNameIntoOriginalParts(editor.getTargetWidgetLook());
        moveToAndSelectWidgetLookFeel(originalWidgetLookName);
        */
    }
}

void LookNFeelCodeMode::moveToAndSelectWidgetLookFeel(QString widgetLookFeelName)
{
    QString wlfTagStartText = QString("<WidgetLook name=\"%1\"").arg(widgetLookFeelName);
    QString wlfTagEndText = "</WidgetLook>";

    // Move cursor to the start of the entire text
    moveCursor(QTextCursor::Start);

    // Find the beginning of the WidgetLookFeel element in the text
    if (!find(wlfTagStartText)) return;

    // Retrieve the position of the cursor which points to the found text
    auto cursor = textCursor();
    auto startPos = cursor.selectionStart();

    // Find the end of the WidgetLookFeel element in the text
    if (!find(wlfTagEndText)) return;

    cursor.setPosition(startPos);
    setTextCursor(cursor);
}

/*
class WidgetLookHighlighter(QtGui.QSyntaxHighlighter):
    """
    Highlighter for the LNF code editing
    """
    def __init__(self, parent):
        super(WidgetLookHighlighter, self).__init__(parent)
        self.parent = parent

        # A dictionary containing the rules names associated with their start regex, end regex and pattern to be used
        self.multilineHighlightingRules = dict()

    def updateWidgetLookRule(self, widgetLookName):
        """
        Updates the regular expression for the WidgetLook highlighting rule
        :param widgetLookName:
        :return:
        """
        wlfTagStartText = "<WidgetLook name=\"%s\"" % widgetLookName
        regexStart = QtCore.QRegExp(wlfTagStartText)
        regexStart.setMinimal(True)

        wlfTagEndText = "</WidgetLook>"
        regexEnd = QtCore.QRegExp(wlfTagEndText)
        regexEnd.setMinimal(True)

        palette = QtGui.QApplication.palette()

        # FIXME: The color palette should be used correctly here instead of hardcoding the color.
        # However neither mpreisler or me (Ident) knew how to do it "the right way"

        highlightingFormat = QtGui.QTextCharFormat()
        highlightingFormat.setForeground(QtGui.QColor(0, 89, 176))
        highlightingFormat.setBackground(palette.color(QtGui.QPalette.Normal, QtGui.QPalette.Base))

        rule = [regexStart, regexEnd, highlightingFormat]
        self.multilineHighlightingRules["WidgetLookRule"] = rule

    def highlightBlock(self, text):
        """

        :param text:
        :return:
        """

        # Sets an integer representing the state of the multiline highlighting rule
        self.setCurrentBlockState(0)

        for dictionaryKey in self.multilineHighlightingRules:
            multilineHighlightingRule = self.multilineHighlightingRules[dictionaryKey]

            regexStart = QtCore.QRegExp(multilineHighlightingRule[0])
            regexEnd = QtCore.QRegExp(multilineHighlightingRule[1])
            highlightFormat = multilineHighlightingRule[2]

            positionOfStartMatch = regexStart.indexIn(text)
            if positionOfStartMatch >= 0:
                self.setCurrentBlockState(1)

            positionOfEndMatch = regexEnd.indexIn(text)
            if positionOfEndMatch >= 0:
                self.setCurrentBlockState(2)

            # In case the match of the start has been found in this line
            if self.currentBlockState() == 1:
                length = len(text) - positionOfStartMatch
                self.setFormat(positionOfStartMatch, length, highlightFormat)

            # In case a match for the end been found in this line and the start was found in a previous one
            if self.currentBlockState() == 2 and self.previousBlockState() == 1:
                length = positionOfEndMatch + regexEnd.matchedLength()
                self.setFormat(0, length, highlightFormat)

            # In case the match of the start has been found in a previous line, and no end was found in this line
            if self.previousBlockState() == 1 and not self.currentBlockState() == 2:
                length = len(text)
                self.setFormat(0, length, highlightFormat)
                self.setCurrentBlockState(1)
*/
