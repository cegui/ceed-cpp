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

from ceed import commands

idbase = 1300


class TargetWidgetChangeCommand(commands.UndoCommand):
    """This command changes the Look n' Feel widget targeted for editing to another one
    """

    def __init__(self, visual, tabbedEditor, newTargetWidgetLook):
        """
        :param visual: LookNFeelVisualEditing
        :param tabbedEditor: LookNFeelTabbedEditor
        :param newTargetWidgetLook: string
        :return:
        """
        super(TargetWidgetChangeCommand, self).__init__()

        self.visual = visual
        self.tabbedEditor = tabbedEditor
        self.oldTargetWidgetLook = tabbedEditor.targetWidgetLook
        self.newTargetWidgetLook = newTargetWidgetLook

        self.refreshText()

    def refreshText(self):
        from ceed.editors.looknfeel.tabbed_editor import LookNFeelTabbedEditor
        if self.oldTargetWidgetLook:
            originalOldTargetName, _ = LookNFeelTabbedEditor.unmapMappedNameIntoOriginalParts(self.oldTargetWidgetLook)
            originalOldTargetName = u"\"" + originalOldTargetName + u"\""
        else:
            originalOldTargetName = u"no selection"

        if self.newTargetWidgetLook:
            originalNewTargetName, _ = LookNFeelTabbedEditor.unmapMappedNameIntoOriginalParts(self.newTargetWidgetLook)
            originalNewTargetName =  u"\"" + originalNewTargetName + u"\""
        else:
            originalNewTargetName, _ = u"no selection"

        self.setText(u"Changed editing target from " + originalOldTargetName + " to " + originalNewTargetName)

    def id(self):
        return idbase + 1

    def mergeWith(self, cmd):
        if self.newTargetWidgetLook == cmd.newTargetWidgetLook:
            return True

        return False

    def undo(self):
        super(TargetWidgetChangeCommand, self).undo()

        self.tabbedEditor.targetWidgetLook = self.oldTargetWidgetLook
        self.visual.updateToNewTargetWidgetLook()

    def redo(self):
        self.tabbedEditor.targetWidgetLook = self.newTargetWidgetLook
        self.visual.updateToNewTargetWidgetLook()

        super(TargetWidgetChangeCommand, self).redo()


class FalagardElementAttributeEdit(commands.UndoCommand):
    """This command resizes given widgets from old positions and old sizes to new
    """

    def __init__(self, falagardProperty, visual, falagardElement, attributeName, newValue, ignoreNextCallback=False):
        super(FalagardElementAttributeEdit, self).__init__()

        self.visual = visual
        """ :type : LookNFeelVisualEditing """

        self.falagardElement = falagardElement
        self.attributeName = attributeName

        self.falagardProperty = falagardProperty
        """ :type : FalagardElementEditorProperty"""

        # We retrieve the momentary value using the getter callback and store it as old value
        from falagard_element_interface import FalagardElementInterface
        self.oldValue, ceguiType = FalagardElementInterface.getAttributeValue(falagardElement, attributeName, visual.tabbedEditor)
        self.oldValueAsString = unicode(self.oldValue)

        # If the value is a subtype of
        from ceed.cegui import ceguitypes
        newValueType = type(newValue)
        if issubclass(newValueType, ceguitypes.Base):
            # if it is a subclass of our ceguitypes, do some special handling
            self.newValueAsString = unicode(newValue)
            self.newValue = newValueType.toCeguiType(self.newValueAsString)
        elif newValueType is bool or newValueType is unicode:
            self.newValue = newValue
            self.newValueAsString = unicode(newValue)
        else:
            raise Exception("Unexpected type encountered")

        # Get the Falagard element's type as string so we can display better info
        from ceed.editors.looknfeel.falagard_element_interface import FalagardElementInterface
        self.falagardElementName = FalagardElementInterface.getFalagardElementTypeAsString(falagardElement)

        self.refreshText()

        self.ignoreNextCall = ignoreNextCallback

    def refreshText(self):
        self.setText("Changing '%s' in '%s' to value '%s'" % (self.attributeName, self.falagardElementName, self.newValueAsString))

    def id(self):
        return idbase + 2

    def mergeWith(self, cmd):
        if self.falagardElement == cmd.falagardElement and self.attributeName == cmd.attributeName:
            self.newValue = cmd.newValue
            self.newValueAsString = cmd.newValueAsString

            self.refreshText()

            return True

        return False

    def undo(self):
        super(FalagardElementAttributeEdit, self).undo()

        # We set the value using the setter callback
        from falagard_element_interface import FalagardElementInterface
        FalagardElementInterface.setAttributeValue(self.falagardElement, self.attributeName, self.oldValue)

        from ceed.propertytree.properties import Property
        #TODO Ident: Refresh the property view afterwards instead
        # self.falagardProperty.setValue(self.oldValue, reason=Property.ChangeValueReason.InnerValueChanged)

        self.visual.updateWidgetLookPreview()

    def redo(self):
        # We set the value using the setter callback
        from falagard_element_interface import FalagardElementInterface
        FalagardElementInterface.setAttributeValue(self.falagardElement, self.attributeName, self.newValue)

        from ceed.propertytree.properties import Property
        #TODO Ident: Refresh the property view afterwards instead
        #self.falagardProperty.setValue(self.newValue, reason=Property.ChangeValueReason.InnerValueChanged)

        self.visual.destroyCurrentPreviewWidget()

        self.visual.tabbedEditor.removeOwnedWidgetLookFalagardMappings()
        self.visual.tabbedEditor.addMappedWidgetLookFalagardMappings()

        self.visual.updateWidgetLookPreview()

        """
        # We add every WidgetLookFeel name of this Look N' Feel to a StringSet
        nameSet = self.visual.tabbedEditor.getStringSetOfWidgetLookFeelNames()
        # We parse all WidgetLookFeels as XML to a string
        import PyCEGUI
        lookAndFeelString = PyCEGUI.WidgetLookManager.getSingleton().getWidgetLookSetAsString(nameSet)

        lookAndFeelString = self.visual.tabbedEditor.unmapWidgetLookReferences(lookAndFeelString)

        self.visual.tabbedEditor.tryUpdatingWidgetLookFeel(lookAndFeelString)
        self.visual.updateToNewTargetWidgetLook()
        """

        super(FalagardElementAttributeEdit, self).redo()

# needs to be at the end, imported to get the singleton
from ceed import mainwindow