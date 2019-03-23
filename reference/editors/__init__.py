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

"""This module contains interfaces needed to run editors tabs (multi-file editing)

Also groups all the editors together to avoid cluttering the root directory.
"""

from PySide import QtCore
from PySide import QtGui

import os.path
import codecs

from ceed import compatibility

import ceed.ui.editors.notypedetected
import ceed.ui.editors.multipletypesdetected
import ceed.ui.editors.multiplepossiblefactories

class NoTypeDetectedDialog(QtGui.QDialog):
    def __init__(self, compatibilityManager):
        super(NoTypeDetectedDialog, self).__init__()

        self.ui = ceed.ui.editors.notypedetected.Ui_NoTypeDetectedDialog()
        self.ui.setupUi(self)

        self.typeChoice = self.findChild(QtGui.QListWidget, "typeChoice")

        for type_ in compatibilityManager.getKnownTypes():
            item = QtGui.QListWidgetItem()
            item.setText(type_)

            # TODO: We should give a better feedback about what's compatible with what
            item.setToolTip("Compatible with CEGUI: %s" % (", ".join(compatibilityManager.getCEGUIVersionsCompatibleWithType(type_))))

            self.typeChoice.addItem(item)

class MultipleTypesDetectedDialog(QtGui.QDialog):
    def __init__(self, compatibilityManager, possibleTypes):
        super(MultipleTypesDetectedDialog, self).__init__()

        self.ui = ceed.ui.editors.multipletypesdetected.Ui_MultipleTypesDetectedDialog()
        self.ui.setupUi(self)

        self.typeChoice = self.findChild(QtGui.QListWidget, "typeChoice")

        for type_ in compatibilityManager.getKnownTypes():
            item = QtGui.QListWidgetItem()
            item.setText(type_)

            if type_ in possibleTypes:
                font = QtGui.QFont()
                font.setBold(True)
                item.setFont(font)

            # TODO: We should give a better feedback about what's compatible with what
            item.setToolTip("Compatible with CEGUI: %s" % (", ".join(compatibilityManager.getCEGUIVersionsCompatibleWithType(type_))))

            self.typeChoice.addItem(item)
