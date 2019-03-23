##############################################################################
#   created:    25th June 2014
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
##############################################################################

from PySide import QtCore
from PySide import QtGui

import PyCEGUI


class LookNFeelHierarchyItem(QtGui.QStandardItem):

    def __init__(self, falagardElement, prefix=""):
        """
        Creates a hierarchy item based on an element of the WidgetLookFeel object. The element can be
        WidgetLookFeel itself or any child node (such as StateImagery or NamedArea, etc.) that is contained
        in the WidgetLookFeel or any of its children. The children of this element will created as well. The
        resulting hierarchy will be equal, or at least very similar, to the node hierarchy seen in the XML file.
        :param falagardElement:
        :return:
        """

        self.falagardElement = falagardElement
        self.prefix = prefix

        name, toolTip = self.getNameAndToolTip(falagardElement, self.prefix)

        super(LookNFeelHierarchyItem, self).__init__(name)
        self.setToolTip(toolTip)

        self.setFlags(QtCore.Qt.ItemIsEnabled |
                      QtCore.Qt.ItemIsSelectable)

        self.setData(None, QtCore.Qt.CheckStateRole)

        self.createChildren()

    @staticmethod
    def getNameAndToolTip(falagardElement, prefix):
        """
        Creates a name and tooltip for any element that can be part of a WidgetLookFeel and returns it
        :param falagardElement:
        :return: str, str
        """
        name = prefix

        from ceed.editors.looknfeel.falagard_element_interface import FalagardElementInterface
        toolTip = u"type: " + FalagardElementInterface.getFalagardElementTypeAsString(falagardElement)

        # Elements that can be children of a WidgetLookFeel:

        if isinstance(falagardElement, PyCEGUI.PropertyDefinitionBase):
            name += u"\"" + falagardElement.getPropertyName() + u"\""

        elif isinstance(falagardElement, PyCEGUI.PropertyInitialiser):
            name += u"\"" + falagardElement.getTargetPropertyName() + u"\""

        elif isinstance(falagardElement, PyCEGUI.NamedArea):
            name += u"\"" + falagardElement.getName() + u"\""
        elif isinstance(falagardElement, PyCEGUI.ImagerySection):
            name += u"\"" + falagardElement.getName() + u"\""
        elif isinstance(falagardElement, PyCEGUI.StateImagery):
            name += u"\"" + falagardElement.getName() + u"\""
        elif isinstance(falagardElement, PyCEGUI.WidgetComponent):
            name += u"\"" + falagardElement.getWidgetName() + u"\"" + u" (\"" + falagardElement.getBaseWidgetType() + u"\")"

        # Elements that can be children of a ImagerySection:
        elif isinstance(falagardElement, PyCEGUI.ImageryComponent):
            name += u"ImageryComponent"
        elif isinstance(falagardElement, PyCEGUI.TextComponent):
            name += u"TextComponent"
        elif isinstance(falagardElement, PyCEGUI.FrameComponent):
            name += u"FrameComponent"

        # Elements that can be children of a StateImagery:
        elif isinstance(falagardElement, PyCEGUI.LayerSpecification):
            name += u"Layer"

        # Elements that can be children of a LayerSpecification:
        elif isinstance(falagardElement, PyCEGUI.SectionSpecification):
            name += u"Section: \"" + falagardElement.getSectionName() + "\""

        # The ComponentArea element
        elif isinstance(falagardElement, PyCEGUI.ComponentArea):
            name += u"Area"

        # The Image element
        elif isinstance(falagardElement, PyCEGUI.Image):
            name += u" \"" + falagardElement.getName() + u"\""

        return name, toolTip

    def createChildren(self):
        """
        Creates the children items for for the this item's WidgetLookFeel element
        :param:
        :return:
        """
        if isinstance(self.falagardElement, PyCEGUI.NamedArea):
            self.createNamedAreaChildren()
        elif isinstance(self.falagardElement, PyCEGUI.ImagerySection):
            self.createImagerySectionChildren()
        elif isinstance(self.falagardElement, PyCEGUI.StateImagery):
            self.createStateImageryChildren()
        elif isinstance(self.falagardElement, PyCEGUI.WidgetComponent):
            self.createWidgetComponentChildren()
        elif isinstance(self.falagardElement, PyCEGUI.ImageryComponent):
            self.createImageryComponentChildren()
        elif isinstance(self.falagardElement, PyCEGUI.TextComponent):
            self.createTextComponentChildren()
        elif isinstance(self.falagardElement, PyCEGUI.FrameComponent):
            self.createFrameComponentChildren()
        elif isinstance(self.falagardElement, PyCEGUI.LayerSpecification):
            self.createLayerSpecificationChildren()
        elif isinstance(self.falagardElement, PyCEGUI.SectionSpecification):
            return

    def createAndAddItem(self, falagardElement, prefix=u""):
        """
        Creates an item based on the supplied object and adds it to this object. A prefix string can be added to the displayed name.
        :param falagardElement:
        :param prefix: str
        :return:
        """
        newItem = LookNFeelHierarchyItem(falagardElement, prefix)
        self.appendRow(newItem)

    def createNamedAreaChildren(self):
        """
        Creates and appends an item for the Area of the NamedArea
        :return:
        """
        area = self.falagardElement.getArea()
        self.createAndAddItem(area)

    def createImagerySectionChildren(self):
        """
        Creates and appends children items based on an ImagerySection.
        :return:
        """
        frameComponentList = self.falagardElement.getFrameComponentPointers()
        for frameComponent in frameComponentList:
            self.createAndAddItem(frameComponent)

        textComponentList = self.falagardElement.getTextComponentPointers()
        for textComponent in textComponentList:
            self.createAndAddItem(textComponent)

        imageryComponentList = self.falagardElement.getImageryComponentPointers()
        for imageryComponent in imageryComponentList:
            self.createAndAddItem(imageryComponent)

    def createStateImageryChildren(self):
        """
        Creates and appends children items based on an ImagerySection.
        :return:
        """
        layerSpecList = self.falagardElement.getLayerSpecificationPointers()
        for layerSpec in layerSpecList:
            self.createAndAddItem(layerSpec)

    def createWidgetComponentChildren(self):
        """
        Creates and appends children items based on a WidgetComponent (child widget).
        :return:
        """
        area = self.falagardElement.getComponentArea()
        self.createAndAddItem(area)

    def createImageryComponentChildren(self):
        """
        Creates and appends children items based on an ImageryComponent.
        :return:
        """
        area = self.falagardElement.getComponentArea()
        self.createAndAddItem(area)

    def createTextComponentChildren(self):
        """
        Creates and appends children items based on a TextComponent.
        :return:
        """
        area = self.falagardElement.getComponentArea()
        self.createAndAddItem(area)

    def createFrameComponentChildren(self):
        """
        Creates and appends children items based on a FrameComponent.
        :return:
        """
        area = self.falagardElement.getComponentArea()
        self.createAndAddItem(area)

    def createLayerSpecificationChildren(self):
        """
        Creates and appends children items based on a LayerSpecification.
        :return:
        """
        sectionSpecList = self.falagardElement.getSectionSpecificationPointers()
        for sectionSpec in sectionSpecList:
            self.createAndAddItem(sectionSpec)