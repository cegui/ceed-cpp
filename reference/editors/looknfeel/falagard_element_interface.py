##############################################################################
# created:    5th July 2014
# author:     Lukas E Meindl
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


import PyCEGUI

from falagard_element_inspector import FalagardElementAttributesManager


class FalagardElementInterface(object):
    """ Implements static functions that can be used to get an set the
    attributes of Falagard elements. Also contains a list of all attributes
    for each type of Falagard element.
    """

    @staticmethod
    def getFalagardElementTypeAsString(falagardElement):
        """
        Returns the CEGUI class name that is corresponding to the Falagard element's CEGUI-type
        :param falagardElement:
        :return:
        """

        from hierarchy_tree_model import LookNFeelHierarchyTreeModel

        if isinstance(falagardElement, PyCEGUI.PropertyDefinitionBase):
            return u"PropertyDefinitionBase"
        if isinstance(falagardElement, PyCEGUI.PropertyInitialiser):
            return u"PropertyInitialiser"
        elif isinstance(falagardElement, PyCEGUI.NamedArea):
            return u"NamedArea"
        elif isinstance(falagardElement, PyCEGUI.ImagerySection):
            return u"ImagerySection"
        elif isinstance(falagardElement, PyCEGUI.StateImagery):
            return u"StateImagery"
        elif isinstance(falagardElement, PyCEGUI.WidgetComponent):
            return u"WidgetComponent"
        elif isinstance(falagardElement, PyCEGUI.ImageryComponent):
            return u"ImageryComponent"
        elif isinstance(falagardElement, PyCEGUI.TextComponent):
            return u"TextComponent"
        elif isinstance(falagardElement, PyCEGUI.FrameComponent):
            return u"FrameComponent"
        elif isinstance(falagardElement, PyCEGUI.LayerSpecification):
            return u"LayerSpecification"
        elif isinstance(falagardElement, PyCEGUI.SectionSpecification):
            return u"SectionSpecification"
        elif isinstance(falagardElement, PyCEGUI.ComponentArea):
            return u"ComponentArea"
        elif isinstance(falagardElement, PyCEGUI.ColourRect):
            return u"ColourRect"
        elif isinstance(falagardElement, PyCEGUI.Image):
            return u"Image"
        elif isinstance(falagardElement, LookNFeelHierarchyTreeModel):
            return u""
        elif falagardElement is None:
            return u""
        else:
            raise Exception("Unknown Falagard element used in FalagardElementInterface.getFalagardElementTypeAsString")

    @staticmethod
    def getListOfAttributes(falagardElement):
        """
        Returns a list of names of attributes for a given Falagard element. Children elements, which can only exist a maximum of one time, are also added to the list. The list
        can be used in connection with getAttributeValue and setAttributeValue.
        :param falagardElement:
        :return:
        """

        PROPERTY_DEFINITION_BASE_ATTRIBUTES = ["name", "type", "initialValue", "layoutOnWrite", "redrawOnWrite", "fireEvent", "help"]
        PROPERTY_INITIALISER_ATTRIBITES = ["name", "value"]

        NAMED_AREA_ATTRIBUTES = ["name"]
        IMAGERY_SECTION_ATTRIBUTES = ["name", "Colour", "ColourProperty"]
        STATE_IMAGERY_ATTRIBUTES = ["name", "clipped"]

        SECTION_SPECIFICATION_ATTRIBUTES = ["section", "look", "controlProperty", "controlValue", "controlWidget", "Colour", "ColourProperty"]
        LAYER_SPECIFICATION_ATTRIBUTES = ["priority"]

        WIDGET_COMPONENT_ATTRIBUTES = ["nameSuffix", "type", "renderer", "look", "autoWindow", "VertAlignment", "HorzAlignment"]

        IMAGERY_COMPONENT_ATTRIBUTES = ["Image", "ImageProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat", "HorzFormatProperty"]
        TEXT_COMPONENT_ATTRIBUTES = ["Text", "TextProperty", "Font", "FontProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat",
                                     "HorzFormatProperty"]
        FRAME_COMPONENT_ATTRIBUTES = ["Colour", "ColourProperty", "TopLeftCorner", "TopRightCorner", "BottomLeftCorner",
                                      "BottomRightCorner", "LeftEdge", "RightEdge", "TopEdge", "BottomEdge", "Background"]

        COMPONENT_AREA_ATTRIBUTES = ["AreaProperty", "NamedAreaSource <look>", "NamedAreaSource <name>"]

        if isinstance(falagardElement, PyCEGUI.PropertyDefinitionBase):
            return PROPERTY_DEFINITION_BASE_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.PropertyInitialiser):
            return PROPERTY_INITIALISER_ATTRIBITES
        elif isinstance(falagardElement, PyCEGUI.NamedArea):
            return NAMED_AREA_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.ImagerySection):
            return IMAGERY_SECTION_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.StateImagery):
            return STATE_IMAGERY_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.WidgetComponent):
            return WIDGET_COMPONENT_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.ImageryComponent):
            return IMAGERY_COMPONENT_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.TextComponent):
            return TEXT_COMPONENT_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.FrameComponent):
            return FRAME_COMPONENT_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.LayerSpecification):
            return LAYER_SPECIFICATION_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.SectionSpecification):
            return SECTION_SPECIFICATION_ATTRIBUTES
        elif isinstance(falagardElement, PyCEGUI.ComponentArea):
            return COMPONENT_AREA_ATTRIBUTES
        else:
            raise Exception("Unknown Falagard element used in FalagardElementInterface.getListOfAttributes")

    @staticmethod
    def getAttributeValue(falagardElement, attributeName, tabbedEditor):
        """
        Returns an attribute value of of a Falagard element using the Falagard element's
        getter function as implemented in the CEGUI code. The attribute is identified
        by a string, which is used to determine the right function call.
        :param falagardElement:
        :param attributeName: str
        :return:
        """
        attributeList = FalagardElementInterface.getListOfAttributes(falagardElement)

        attributeValue = None
        # Elements that can be children of a WidgetLookFeel:

        if isinstance(falagardElement, PyCEGUI.PropertyDefinitionBase):
            # "name", "type", "initialValue", "layoutOnWrite", "redrawOnWrite", "fireEvent", "help"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getPropertyName()
            elif attributeName == attributeList[1]:
                attributeValue = falagardElement.getDataType()
            elif attributeName == attributeList[2]:
                return FalagardElementInterface.getPropertyDefinitionBaseValueAsCeguiType(falagardElement)
            elif attributeName == attributeList[3]:
                attributeValue = falagardElement.isLayoutOnWrite()
            elif attributeName == attributeList[4]:
                attributeValue = falagardElement.isRedrawOnWrite()
            elif attributeName == attributeList[5]:
                attributeValue = falagardElement.getEventFiredOnWrite()
            elif attributeName == attributeList[6]:
                attributeValue = falagardElement.getHelpString()

        elif isinstance(falagardElement, PyCEGUI.PropertyInitialiser):
            # "name", "value"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getTargetPropertyName()
            elif attributeName == attributeList[1]:
                return FalagardElementInterface.getPropertyInitialiserValueAsCeguiType(falagardElement, tabbedEditor)

        if isinstance(falagardElement, PyCEGUI.NamedArea):
            # "name"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getName()

        elif isinstance(falagardElement, PyCEGUI.ImagerySection):
            # "name", "Colour", "ColourProperty"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getName()
            elif attributeName == attributeList[1]:
                attributeValue = falagardElement.getMasterColours()
            elif attributeName == attributeList[2]:
                attributeValue = falagardElement.getMasterColoursPropertySource()

        elif isinstance(falagardElement, PyCEGUI.StateImagery):
            # "name", "clipped"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getName()
            if attributeName == attributeList[1]:
                attributeValue = falagardElement.isClippedToDisplay()

        elif isinstance(falagardElement, PyCEGUI.WidgetComponent):
            # "nameSuffix", "type", "renderer", "look", "autoWindow", "VertAlignment", "HorzAlignment"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getWidgetName()
            elif attributeName == attributeList[1]:
                attributeValue = falagardElement.getBaseWidgetType()
            elif attributeName == attributeList[2]:
                attributeValue = falagardElement.getWindowRendererType()
            elif attributeName == attributeList[3]:
                attributeValue = falagardElement.getWidgetLookName()
            elif attributeName == attributeList[4]:
                attributeValue = falagardElement.isAutoWindow()
            elif attributeName == attributeList[5]:
                attributeValue = falagardElement.getVerticalWidgetAlignment()
            elif attributeName == attributeList[6]:
                attributeValue = falagardElement.getHorizontalWidgetAlignment()

        # Elements that can be children of an ImagerySection:
        elif isinstance(falagardElement, PyCEGUI.ImageryComponent):
            # "Image", "ImageProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat", "HorzFormatProperty"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getImage()
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[1]:
                attributeValue = falagardElement.getImagePropertySource()
            elif attributeName == attributeList[2]:
                attributeValue = falagardElement.getColours()
            elif attributeName == attributeList[3]:
                attributeValue = falagardElement.getColoursPropertySource()
            elif attributeName == attributeList[4]:
                attributeValue = falagardElement.getVerticalFormattingFromComponent()
            elif attributeName == attributeList[5]:
                attributeValue = falagardElement.getVerticalFormattingPropertySource()
            elif attributeName == attributeList[6]:
                attributeValue = falagardElement.getHorizontalFormattingFromComponent()
            elif attributeName == attributeList[7]:
                attributeValue = falagardElement.getHorizontalFormattingPropertySource()

        elif isinstance(falagardElement, PyCEGUI.TextComponent):
            # ""Text", "TextProperty", "Font", "FontProperty", "Colour", "ColourProperty", "VertFormat",
            # "VertFormatProperty", "HorzFormat", "HorzFormatProperty"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getText()
            elif attributeName == attributeList[1]:
                attributeValue = falagardElement.getTextPropertySource()
            elif attributeName == attributeList[2]:
                attributeValue = falagardElement.getFont()
                if not attributeValue:
                    return None, PyCEGUI.Font
            elif attributeName == attributeList[3]:
                attributeValue = falagardElement.getFontPropertySource()
            elif attributeName == attributeList[4]:
                attributeValue = falagardElement.getColours()
            elif attributeName == attributeList[5]:
                attributeValue = falagardElement.getColoursPropertySource()
            elif attributeName == attributeList[6]:
                attributeValue = falagardElement.getVerticalFormattingFromComponent()
            elif attributeName == attributeList[7]:
                attributeValue = falagardElement.getVerticalFormattingPropertySource()
            elif attributeName == attributeList[8]:
                attributeValue = falagardElement.getHorizontalFormattingFromComponent()
            elif attributeName == attributeList[9]:
                attributeValue = falagardElement.getHorizontalFormattingPropertySource()

        elif isinstance(falagardElement, PyCEGUI.FrameComponent):
            # "Colour", "ColourProperty", "TopLeftCorner", "TopRightCorner", "BottomLeftCorner",
            # "BottomRightCorner", "LeftEdge", "RightEdge", "TopEdge", "BottomEdge", "Background"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getColours()
            elif attributeName == attributeList[1]:
                attributeValue = falagardElement.getColoursPropertySource()
            elif attributeName == attributeList[2]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_TOP_LEFT_CORNER)
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[3]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_TOP_RIGHT_CORNER)
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[4]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_BOTTOM_LEFT_CORNER)
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[5]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_BOTTOM_RIGHT_CORNER)
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[6]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_LEFT_EDGE)
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[7]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_RIGHT_EDGE)
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[8]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_TOP_EDGE)
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[9]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_BOTTOM_EDGE)
                if not attributeValue:
                    return None, PyCEGUI.Image
            elif attributeName == attributeList[10]:
                attributeValue = falagardElement.getImage(PyCEGUI.FrameImageComponent.FIC_BACKGROUND)
                if not attributeValue:
                    return None, PyCEGUI.Image

        # Elements that can be children of a StateImagery:
        elif isinstance(falagardElement, PyCEGUI.LayerSpecification):
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getLayerPriority()

        # Elements that can be children of a LayerSpecification:
        elif isinstance(falagardElement, PyCEGUI.SectionSpecification):
            # "section", "look", "controlProperty", "controlValue", "controlWidget", "Colour", "ColourProperty"
            if attributeName == attributeList[0]:
                attributeValue = falagardElement.getSectionName()
            elif attributeName == attributeList[1]:
                attributeValue = falagardElement.getOwnerWidgetLookFeel()
            elif attributeName == attributeList[2]:
                attributeValue = falagardElement.getRenderControlPropertySource()
            elif attributeName == attributeList[3]:
                attributeValue = falagardElement.getRenderControlValue()
            elif attributeName == attributeList[4]:
                attributeValue = falagardElement.getRenderControlWidget()
            elif attributeName == attributeList[5]:
                attributeValue = falagardElement.getOverrideColours()
            elif attributeName == attributeList[6]:
                attributeValue = falagardElement.getOverrideColoursPropertySource()

        # A ComponentArea element
        elif isinstance(falagardElement, PyCEGUI.ComponentArea):
            if attributeName == attributeList[0]:
                if falagardElement.isAreaFetchedFromProperty():
                    attributeValue = falagardElement.getAreaPropertySource()
                else:
                    return None, unicode
            if attributeName == attributeList[1]:
                if falagardElement.isAreaFetchedFromNamedArea():
                    attributeValue = falagardElement.getAreaPropertySource()
                else:
                    return None, unicode
            if attributeName == attributeList[2]:
                if falagardElement.isAreaFetchedFromNamedArea():
                    attributeValue = falagardElement.getNamedAreaSourceLook()
                else:
                    return None, unicode

        # Please do not falsely "simplify" this line. The attributeValue may be False or "" but not None
        if attributeValue is None:
            raise Exception("Unknown Falagard element and/or attribute used in FalagardElementInterface.getAttributeValue")

        return attributeValue, type(attributeValue)

    @staticmethod
    def setAttributeValue(falagardElement, attributeName, attributeValue):
        """
        Sets an attribute value of of a Falagard element using the Falagard element's
        getter function as implemented in the CEGUI code. The attribute is identified
        by a string, which is used to determine the right function call.
        :param falagardElement:
        :param attributeName: str
        :param attributeValue:
        :return:
        """

        attributeList = FalagardElementInterface.getListOfAttributes(falagardElement)

        # Elements that can be children of a WidgetLookFeel:

        if isinstance(falagardElement, PyCEGUI.PropertyDefinitionBase):
            # "name", "type", "initialValue", "layoutOnWrite", "redrawOnWrite", "fireEvent", "help"
            if attributeName == attributeList[0]:
                #TODO Ident:
                raise Exception("TODO RENAME")
            elif attributeName == attributeList[1]:
                #TODO Ident
                raise Exception("TODO TYPECHANGE")
            elif attributeName == attributeList[2]:
                FalagardElementInterface.setPropertyDefinitionBaseValue(falagardElement, attributeValue)
            elif attributeName == attributeList[3]:
                falagardElement.setLayoutOnWrite(attributeValue)
            elif attributeName == attributeList[4]:
                falagardElement.setRedrawOnWrite(attributeValue)
            elif attributeName == attributeList[5]:
                falagardElement.setEventFiredOnWrite(attributeValue)
            elif attributeName == attributeList[6]:
                falagardElement.setEventFiredOnWrite(attributeValue)
            elif attributeName == attributeList[7]:
                falagardElement.setHelpString(attributeValue)

        elif isinstance(falagardElement, PyCEGUI.PropertyInitialiser):
            # "name", "value"
            if attributeName == attributeList[0]:
                #TODO: What to do with the initialiser value once the type changes?
                #falagardElement.setTargetPropertyName(attributeValue)
                raise Exception("TODO TYPECHANGE")
            elif attributeName == attributeList[1]:
                FalagardElementInterface.setPropertyInitialiserValue(falagardElement, attributeValue)

        elif isinstance(falagardElement, PyCEGUI.NamedArea):
            # "name"
            if attributeName == attributeList[0]:
                falagardElement.setName(attributeValue)

        elif isinstance(falagardElement, PyCEGUI.ImagerySection):
            # "name", "Colour", "ColourProperty"
            if attributeName == attributeList[0]:
                falagardElement.setName(attributeValue)
            elif attributeName == attributeList[1]:
                falagardElement.setMasterColours(attributeValue)
            elif attributeName == attributeList[2]:
                falagardElement.setMasterColoursPropertySource(attributeValue)

        elif isinstance(falagardElement, PyCEGUI.StateImagery):
            # "name", "clipped"
            if attributeName == attributeList[0]:
                falagardElement.setName(attributeValue)
            if attributeName == attributeList[1]:
                falagardElement.isClippedToDisplay(attributeValue)

        elif isinstance(falagardElement, PyCEGUI.WidgetComponent):
            # "nameSuffix", "type", "renderer", "look", "autoWindow", "VertAlignment", "HorzAlignment"
            if attributeName == attributeList[0]:
                falagardElement.setWidgetName(attributeValue)
            elif attributeName == attributeList[1]:
                falagardElement.setBaseWidgetType(attributeValue)
            elif attributeName == attributeList[2]:
                falagardElement.setWindowRendererType(attributeValue)
            elif attributeName == attributeList[3]:
                falagardElement.setWidgetLookName(attributeValue)
            elif attributeName == attributeList[4]:
                falagardElement.isAutoWindow(attributeValue)
            elif attributeName == attributeList[5]:
                falagardElement.setVerticalWidgetAlignment(attributeValue)
            elif attributeName == attributeList[6]:
                falagardElement.setHorizontalWidgetAlignment(attributeValue)

        # Elements that can be children of an ImagerySection:

        elif isinstance(falagardElement, PyCEGUI.ImageryComponent):
            # "Image", "ImageProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat", "HorzFormatProperty"
            if attributeName == attributeList[0]:
                falagardElement.setImage(attributeValue)
            elif attributeName == attributeList[1]:
                falagardElement.setImagePropertySource(attributeValue)
            elif attributeName == attributeList[2]:
                falagardElement.setColours(attributeValue)
            elif attributeName == attributeList[3]:
                falagardElement.setColoursPropertySource(attributeValue)
            elif attributeName == attributeList[4]:
                falagardElement.setVerticalFormatting(attributeValue)
            elif attributeName == attributeList[5]:
                falagardElement.setVerticalFormattingPropertySource(attributeValue)
            elif attributeName == attributeList[6]:
                falagardElement.setHorizontalFormatting(attributeValue)
            elif attributeName == attributeList[7]:
                falagardElement.setHorizontalFormattingPropertySource(attributeValue)

        elif isinstance(falagardElement, PyCEGUI.TextComponent):
            # "Text", "TextProperty", "Font", "FontProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat", "HorzFormatProperty"
            if attributeName == attributeList[0]:
                falagardElement.setText(attributeValue)
            elif attributeName == attributeList[1]:
                falagardElement.setTextPropertySource(attributeValue)
            elif attributeName == attributeList[2]:
                falagardElement.setFont(attributeValue.getName())
            elif attributeName == attributeList[3]:
                falagardElement.setFontPropertySource(attributeValue)
            elif attributeName == attributeList[4]:
                falagardElement.setColours(attributeValue)
            elif attributeName == attributeList[5]:
                falagardElement.setColoursPropertySource(attributeValue)
            elif attributeName == attributeList[6]:
                falagardElement.setVerticalFormatting(attributeValue)
            elif attributeName == attributeList[7]:
                falagardElement.setVerticalFormattingPropertySource(attributeValue)
            elif attributeName == attributeList[8]:
                falagardElement.setHorizontalFormatting(attributeValue)
            elif attributeName == attributeList[9]:
                falagardElement.setHorizontalFormattingPropertySource(attributeValue)

        elif isinstance(falagardElement, PyCEGUI.FrameComponent):
            # "Colour", "ColourProperty",
            if attributeName == attributeList[0]:
                falagardElement.setColours(attributeValue)
            elif attributeName == attributeList[1]:
                falagardElement.setColoursPropertySource(attributeValue)
            elif attributeName == attributeList[2]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_TOP_LEFT_CORNER, attributeValue)
            elif attributeName == attributeList[3]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_TOP_RIGHT_CORNER, attributeValue)
            elif attributeName == attributeList[4]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_BOTTOM_LEFT_CORNER, attributeValue)
            elif attributeName == attributeList[5]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_BOTTOM_RIGHT_CORNER, attributeValue)
            elif attributeName == attributeList[6]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_LEFT_EDGE, attributeValue)
            elif attributeName == attributeList[7]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_RIGHT_EDGE, attributeValue)
            elif attributeName == attributeList[8]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_TOP_EDGE, attributeValue)
            elif attributeName == attributeList[9]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_BOTTOM_EDGE, attributeValue)
            elif attributeName == attributeList[10]:
                falagardElement.setImage(PyCEGUI.FrameImageComponent.FIC_BACKGROUND, attributeValue)

        # Elements that can be children of a StateImagery:

        elif isinstance(falagardElement, PyCEGUI.LayerSpecification):
            if attributeName == attributeList[0]:
                falagardElement.setLayerPriority(attributeValue)

        # General elements:

        # A SectionSpecification element
        elif isinstance(falagardElement, PyCEGUI.SectionSpecification):
            # "section", "look", "controlProperty", "controlValue", "controlWidget", "Colour", "ColourProperty"
            if attributeName == attributeList[0]:
                falagardElement.setSectionName(attributeValue)
            elif attributeName == attributeList[1]:
                falagardElement.setOwnerWidgetLookFeel(attributeValue)
            elif attributeName == attributeList[2]:
                falagardElement.setRenderControlPropertySource(attributeValue)
            elif attributeName == attributeList[3]:
                falagardElement.setRenderControlValue(attributeValue)
            elif attributeName == attributeList[4]:
                falagardElement.setRenderControlWidget(attributeValue)
            elif attributeName == attributeList[5]:
                falagardElement.setOverrideColours(attributeValue)
            elif attributeName == attributeList[6]:
                falagardElement.setOverrideColoursPropertySource(attributeValue)

        # A ComponentArea element
        elif isinstance(falagardElement, PyCEGUI.ComponentArea):
            if attributeName == attributeList[0]:
                falagardElement.setAreaPropertySource(attributeValue)
            if attributeName == attributeList[1]:
                falagardElement.setAreaPropertySource(attributeValue)
            if attributeName == attributeList[2]:
                falagardElement.setNamedAreaSouce(attributeValue, falagardElement.getAreaPropertySource())

        else:
            raise Exception("Unknown Falagard element and/or attribute used in FalagardElementInterface.setAttributeValue")

    @staticmethod
    def getPropertyInitialiserValueAsCeguiType(propertyInitialiser, tabbedEditor):
        """
        Returns a CEGUI value and CEGUI type based on the PropertyInitialiser value
        :param propertyInitialiser:
        :return:
        """
        propertyName = propertyInitialiser.getTargetPropertyName()
        initialValue = propertyInitialiser.getInitialiserValue()

        # We create a dummy window to be able to retrieve the correct dataType
        dummyWindow = PyCEGUI.WindowManager.getSingleton().createWindow(tabbedEditor.targetWidgetLook)
        propertyInstance = dummyWindow.getPropertyInstance(propertyName)
        dataType = propertyInstance.getDataType()
        PyCEGUI.WindowManager.getSingleton().destroyWindow(dummyWindow)

        return FalagardElementInterface.convertToCeguiValueAndCeguiType(initialValue, dataType)

    @staticmethod
    def getPropertyDefinitionBaseValueAsCeguiType(propertyDefBase):
        """
        Returns a CEGUI value and CEGUI type based on the PropertyDefinitionBase value
        :param propertyDefBase:
        :return:
        """

        dataType = propertyDefBase.getDataType()
        initialValue = propertyDefBase.getInitialValue()

        return FalagardElementInterface.convertToCeguiValueAndCeguiType(initialValue, dataType)

    @staticmethod
    def convertToCeguiValueAndCeguiType(valueAsString, dataTypeAsString):
        """
        Converts a string based CEGUI value and CEGUI type to the native CEGUI value and CEGUI type
        :param valueAsString: str
        :param dataTypeAsString: str
        :return:
        """
        from ceed.propertysetinspector import CEGUIPropertyManager
        pythonDataType = CEGUIPropertyManager.getPythonTypeFromStringifiedCeguiType(dataTypeAsString)

        value = FalagardElementInterface.getCeguiTypeValueFromString(pythonDataType, valueAsString)
        valueType = FalagardElementAttributesManager.getCeguiTypeTypeFromPythonType(pythonDataType)

        return value, valueType

    @staticmethod
    def getCeguiTypeValueFromString(pythonDataType, valueAsString):
        """
        Returns a CEGUI-typed object based on a given string and Python type
        :param pythonDataType:
        :param valueAsString: str
        :return:
        """

        from ceed.cegui import ceguitypes as ceguiTypes
        if issubclass(pythonDataType, ceguiTypes.Base):
            # if the type is a subtype of the python cegui type, then use the conversion function
            value = pythonDataType.tryToCeguiType(valueAsString)
        else:
            if valueAsString is None:
                value = None
            elif pythonDataType is bool:
                # The built-in bool parses "false" as True
                # so we replace the default value creator.
                from ceed.propertytree import utility as propertyTreeUtility
                value = propertyTreeUtility.boolFromString(valueAsString)
            elif pythonDataType is unicode:
                value = valueAsString

        return value

    @staticmethod
    def setPropertyInitialiserValue(propertyInitialiser, value):

        valueType = type(value)

        pythonType = FalagardElementAttributesManager.getPythonTypeFromCeguiType(valueType)

        from ceed.cegui import ceguitypes
        if issubclass(pythonType, ceguitypes.Base):
            # if the value's type is a subclass of a python ceguitype, convert the value to a string
            valueAsString = pythonType.toString(value)
        else:
            valueAsString = unicode(value)

        propertyInitialiser.setInitialiserValue(valueAsString)

    @staticmethod
    def setPropertyDefinitionBaseValue(propertyDefBase, value):

        valueType = type(value)

        pythonType = FalagardElementAttributesManager.getPythonTypeFromCeguiType(valueType)

        from ceed.cegui import ceguitypes
        if issubclass(pythonType, ceguitypes.Base):
            # if the value's type is a subclass of a python ceguitype, convert the value to a string
            valueAsString = pythonType.toString(value)
        elif value is None:
            valueAsString = u""
        else:
            valueAsString = unicode(value)

        propertyDefBase.setInitialValue(valueAsString)