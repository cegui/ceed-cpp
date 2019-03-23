##############################################################################
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

"""Contains reusable widgets that I haven't found in Qt for some reason
"""

from PySide import QtCore
from PySide import QtGui

import ceed.ui.widgets.filelineedit
import ceed.ui.widgets.keysequencebuttondialog

class LineEditWithClearButton(QtGui.QLineEdit):
    """A QLineEdit with an inline clear button.

    Hitting Escape in the line edit clears it.

    Based on http://labs.qt.nokia.com/2007/06/06/lineedit-with-a-clear-button/
    """

    def __init__(self, parent=None):
        super(LineEditWithClearButton, self).__init__(parent)

        btn = self.button = QtGui.QToolButton(self)
        icon = QtGui.QPixmap("icons/widgets/edit-clear.png")
        btn.setIcon(icon)
        btn.setIconSize(icon.size())
        btn.setCursor(QtCore.Qt.ArrowCursor)
        btn.setStyleSheet("QToolButton { border: none; padding: 0px; }")
        btn.hide()

        btn.clicked.connect(self.clear)
        self.textChanged.connect(self.updateCloseButton)

        clearAction = QtGui.QAction(self)
        clearAction.setShortcut(QtGui.QKeySequence("Esc"))
        clearAction.setShortcutContext(QtCore.Qt.ShortcutContext.WidgetShortcut)
        clearAction.triggered.connect(self.clear)
        self.addAction(clearAction)

        frameWidth = self.style().pixelMetric(QtGui.QStyle.PM_DefaultFrameWidth)
        self.setStyleSheet("QLineEdit { padding-right: %ipx; }" % (btn.sizeHint().width() + frameWidth + 1))

        minSizeHint = self.minimumSizeHint()
        self.setMinimumSize(max(minSizeHint.width(), btn.sizeHint().width() + frameWidth * 2 + 2),
                            max(minSizeHint.height(), btn.sizeHint().height() + frameWidth * 2 + 2))

    def resizeEvent(self, event):
        sz = self.button.sizeHint()
        frameWidth = self.style().pixelMetric(QtGui.QStyle.PM_DefaultFrameWidth)
        self.button.move(self.rect().right() - frameWidth - sz.width(),
                         (self.rect().bottom() + 1 - sz.height()) / 2)

    def updateCloseButton(self, text):
        self.button.setVisible(not not text)
