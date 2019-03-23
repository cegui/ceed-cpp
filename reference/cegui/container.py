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

"""Implementation of a convenience Qt and CEGUI interaction containment.

Allows you to use CEGUI as if it were a Qt widget.
"""

from PySide import QtCore
from PySide import QtGui
from PySide import QtWebKit

import collections
import logging

from ceed.cegui import qtgraphics

import ceed.ui.ceguidebuginfo
import PyCEGUI

class LogMessageWrapper(object):
    def __init__(self, message, level):
        self.message = message
        self.level = level

    def asTableRow(self):
        bgColour = "transparent"

        if self.level == PyCEGUI.LoggingLevel.Errors:
            stringLevel = "E"
            bgColour = "#ff5f5f"

        elif self.level == PyCEGUI.LoggingLevel.Warnings:
            stringLevel = "W"
            bgColour = "#fff76f"

        else:
            stringLevel = " "
            bgColour = "transparent"

        return "<tr><td style=\"background: %s\">%s</td><td>%s</td></tr>\n" % (bgColour, stringLevel, self.message)

class DebugInfo(QtGui.QDialog):
    """A debugging/info widget about the embedded CEGUI instance"""

    # This will allow us to view logs in Qt in the future

    def __init__(self, containerWidget):
        super(DebugInfo, self).__init__()

        self.setVisible(False)

        self.setWindowFlags(self.windowFlags() | QtCore.Qt.WindowStaysOnTopHint)

        self.containerWidget = containerWidget
        # update FPS and render time very second
        self.boxUpdateInterval = 1

        self.ui = ceed.ui.ceguidebuginfo.Ui_CEGUIWidgetInfo()
        self.ui.setupUi(self)

        self.currentFPSBox = self.findChild(QtGui.QLineEdit, "currentFPSBox")
        self.currentRenderTimeBox = self.findChild(QtGui.QLineEdit, "currentRenderTimeBox")

        self.errors = 0
        self.errorsBox = self.findChild(QtGui.QLineEdit, "errorsBox")

        self.warnings = 0
        self.warningsBox = self.findChild(QtGui.QLineEdit, "warningsBox")

        self.others = 0
        self.othersBox = self.findChild(QtGui.QLineEdit, "othersBox")

        self.logViewArea = self.findChild(QtGui.QWidget, "logViewArea")
        self.logViewAreaLayout = QtGui.QVBoxLayout()

        self.logView = QtWebKit.QWebView()
        self.logView.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Ignored)
        self.logViewAreaLayout.addWidget(self.logView)

        self.logViewArea.setLayout(self.logViewAreaLayout)

        self.logMessagesLimit = settings.getEntry("global/cegui_debug_info/log_limit").value
        self.logMessages = collections.deque([])

        self.containerWidget.ceguiInstance.logger.registerSubscriber(self.logEvent)

    def logEvent(self, message, level):
        if level == PyCEGUI.LoggingLevel.Errors:
            self.errors += 1
            self.errorsBox.setText(str(self.errors))

        elif level == PyCEGUI.LoggingLevel.Warnings:
            self.warnings += 1
            self.warningsBox.setText(str(self.warnings))

        else:
            self.others += 1
            self.othersBox.setText(str(self.others))

        # log info using the logging message, allows debug outputs without GUI
        logging.info("CEGUI message: %s", message)

        # remove old messages
        while len(self.logMessages) >= self.logMessagesLimit:
            self.logMessages.popleft()

        self.logMessages.append(LogMessageWrapper(message, level))

    def show(self):
        htmlLog = "\n".join([msg.asTableRow() for msg in self.logMessages])

        self.logView.setHtml("""
<html>
<body>
<style type="text/css">
font-size: 10px;
</style>
<table>
<thead>
<th></th><th>Message</th>
</thead>
<tbody>
""" + htmlLog + """
</tbody>
</table>
</html>""")

        super(DebugInfo, self).show()
        self.updateFPSTick()

    def updateFPSTick(self):
        if not self.isVisible():
            return

        lastRenderDelta = self.containerWidget.ceguiInstance.lastRenderTimeDelta
        if lastRenderDelta <= 0:
            lastRenderDelta = 1

        self.currentFPSBox.setText("%0.6f" % (1.0 / lastRenderDelta))

        QtCore.QTimer.singleShot(500, self.updateFPSTick)

# we import here to avoid circular dependencies (GraphicsView has to be defined at this point)
import ceed.ui.ceguicontainerwidget
