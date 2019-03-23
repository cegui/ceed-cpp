# #############################################################################
#   created:    2nd July 2014
#   author:     Lukas E Meindl
##############################################################################
##############################################################################
#   CEED - Unified CEGUI asset editor
#
#   Copyright (C) 2011-2014   Martin Preisler <martin@preisler.me>
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
###############################################################################

from PySide import QtGui

from ceed.propertysetinspector import PropertyInspectorWidget

import ceed.propertytree as pt

from ceed.propertytree.editors import PropertyEditorRegistry
from ceed.editors.looknfeel import undoable_commands


class LookNFeelFalagardElementEditorDockWidget(QtGui.QDockWidget):
    """This dock widget allows to add, remove or edit the Property, PropertyDefinition and PropertyLinkDefinition elements of a WidgetLook
    """

    def __init__(self, visual, tabbedEditor):
        """
        :param visual: LookNFeelVisualEditing
        :param tabbedEditor: LookNFeelTabbedEditor
        :return:
        """
        super(LookNFeelFalagardElementEditorDockWidget, self).__init__()
        self.setObjectName("FalagardElementEditorDockWidget")
        self.visual = visual
        self.tabbedEditor = tabbedEditor

        self.setWindowTitle("Falagard Element Editor")
        # Make the dock take as much space as it can vertically
        self.setSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Maximum)

        self.inspector = PropertyInspectorWidget()
        self.inspector.ptree.setupRegistry(PropertyEditorRegistry(True))

        self.setWidget(self.inspector)


class FalagardElementEditorProperty(pt.properties.SinglePropertyWrapper):
    """Overrides the default Property to update the 'inner properties'
    and to create undoable commands that update the WidgetLookFeel element.
    """

    def __init__(self, ownedProperty, falagardElement, attributeName, visual):
        super(FalagardElementEditorProperty, self).__init__(ownedProperty)

        self.visual = visual
        self.falagardElement = falagardElement
        self.attributeName = attributeName

    def tryUpdateInner(self, newValue, reason=pt.properties.Property.ChangeValueReason.Unknown):
        if super(FalagardElementEditorProperty, self).tryUpdateInner(newValue, reason):
            if type(newValue) is str:
                ceguiValue = unicode(newValue)
            else:
                ceguiValue = newValue

            # Create the undoable command for editing the attribute,
            # but tell it not to trigger the change-callback
            # on the first run because our editor value has already changed,
            # we just want to sync the Falagard element's attribute value now.
            cmd = undoable_commands.FalagardElementAttributeEdit(self, self.visual, self.falagardElement, self.attributeName, ceguiValue,
                                                                 ignoreNextCallback=True)
            self.visual.tabbedEditor.undoStack.push(cmd)

            # make sure to redraw the scene to preview the property
            self.visual.scene.update()

            return True

        return False