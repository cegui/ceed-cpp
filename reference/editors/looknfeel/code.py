# #############################################################################
#   CEED - Unified CEGUI asset editor
#
#   Copyright (C) 2011-2012   Martin Preisler <martin@preisler.me>
#                             and contributing authors (see AUTHORS file)
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
##############################################################################

from ceed.editors import multi

from PySide import QtGui
from PySide import QtCore

import PyCEGUI

class CodeEditing(multi.CodeEditMode):
    def __init__(self, tabbedEditor):
        """
        :param tabbedEditor: LookNFeelTabbedEditor
        :return:
        """
        super(CodeEditing, self).__init__()

        self.tabbedEditor = tabbedEditor
        self.highlighter = WidgetLookHighlighter(self)

    def getNativeCode(self):
        # Returns the Look n' Feel XML string based on all WidgetLookFeels that belong to the Look n' Feel file according to the editor

        # We add every WidgetLookFeel name of this Look N' Feel to a StringSet
        nameSet = self.tabbedEditor.getStringSetOfWidgetLookFeelNames()
        # We parse all WidgetLookFeels as XML to a string
        lookAndFeelString = PyCEGUI.WidgetLookManager.getSingleton().getWidgetLookSetAsString(nameSet)

        lookAndFeelString = self.tabbedEditor.unmapWidgetLookReferences(lookAndFeelString)

        return lookAndFeelString

    def propagateNativeCode(self, code):
        # we have to make the context the current context to ensure textures are fine
        mainwindow.MainWindow.instance.ceguiContainerWidget.makeGLContextCurrent()

        loadingSuccessful = self.tabbedEditor.tryUpdatingWidgetLookFeel(code)
        self.tabbedEditor.visual.updateToNewTargetWidgetLook()

        return loadingSuccessful

    def moveToAndSelectWidgetLookFeel(self, widgetLookFeelName):
        wlfTagStartText = "<WidgetLook name=\"%s\"" % widgetLookFeelName
        wlfTagEndText = "</WidgetLook>"

        # Move cursor to the start of the entire text
        self.moveCursor(QtGui.QTextCursor.Start)

        # Find the beginning of the WidgetLookFeel element in the text
        textWasFound = self.find(wlfTagStartText)
        if not textWasFound:
            return

        # Retrieve the position of the cursor which points to the found text
        textCursor = self.textCursor()
        startPos = textCursor.selectionStart()

        # Find the end of the WidgetLookFeel element in the text
        textWasFound = self.find(wlfTagEndText)
        if not textWasFound:
            return

        textCursor.setPosition(startPos)
        self.setTextCursor(textCursor)

    def refreshFromVisual(self):
        """Refreshes this Code editing mode with current native source code and moves to and selects the
        WidgetLookFeel code."""

        if self.tabbedEditor.targetWidgetLook:
            # Refresh the WidgetLookFeel Highlighter based on the new name of the WidgetLook
            originalWidgetLookName, _ = self.tabbedEditor.unmapMappedNameIntoOriginalParts(self.tabbedEditor.targetWidgetLook)
            self.highlighter.updateWidgetLookRule(originalWidgetLookName)

        super(CodeEditing, self).refreshFromVisual()

        if self.tabbedEditor.targetWidgetLook:
            originalWidgetLookName, _ = self.tabbedEditor.unmapMappedNameIntoOriginalParts(self.tabbedEditor.targetWidgetLook)
            self.moveToAndSelectWidgetLookFeel(originalWidgetLookName)


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


# needs to be at the end, imported to get the singleton
from ceed import mainwindow
