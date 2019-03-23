##############################################################################
#   CEED - Unified CEGUI asset editor
#
#   Copyright (C) 2011-2015   Lukas Meindl <cegui@lukasmeindl.at>
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

from PySide import QtGui

from ceed.editors import multi


class CodeEditingWithViewRestore(multi.CodeEditMode):
    def __init__(self):
        super(CodeEditingWithViewRestore, self).__init__()

        self.lastVertScrollBarValue = 0
        """:type : int"""
        self.lastCursorSelectionEnd = 0
        """:type : int"""
        self.lastCursorSelectionStart = 0
        """:type : int"""

    def refreshFromVisual(self):
        """Refreshes this Code editing mode with current native source code and moves to the last scroll and cursor
        positions."""

        super(CodeEditingWithViewRestore, self).refreshFromVisual()

        vbar = self.verticalScrollBar()
        vbar.setValue(self.lastVertScrollBarValue)

        textCursor = self.textCursor()
        textCursor.setPosition(self.lastCursorSelectionStart)
        textCursor.setPosition(self.lastCursorSelectionEnd, QtGui.QTextCursor.KeepAnchor)
        self.setFocus()
        self.setTextCursor(textCursor)

    def propagateToVisual(self):
        """Propagates source code from this Code editing mode to your editor implementation and stores
        the last scrollbar and cursor positions

        Returns True if changes were accepted (the code was valid, etc...)
        Returns False if changes weren't accepted (invalid code most likely)"""

        vbar = self.verticalScrollBar()
        self.lastVertScrollBarValue = vbar.value()

        textCursor = self.textCursor()
        self.lastCursorSelectionStart = textCursor.selectionStart()
        self.lastCursorSelectionEnd = textCursor.selectionEnd()

        return super(CodeEditingWithViewRestore, self).propagateToVisual()