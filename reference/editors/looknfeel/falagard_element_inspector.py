# #############################################################################
# created:    2nd July 2014
# author:     Lukas E Meindl
##############################################################################
##############################################################################
# CEED - Unified CEGUI asset editor
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

from ceed.propertytree import properties
from ceed.propertytree import utility as ptUtility

from ceed.cegui import ceguitypes as ct

from collections import OrderedDict

from falagard_element_editor import FalagardElementEditorProperty

import PyCEGUI


class FalagardElementAttributesManager(object):
    """Builds propertytree settings from a CEGUI Falagard element, allowing to edit its attributes in the editor.
    """

    # Maps CEGUI data types (in string form) to Python types

    _typeMap = {
        int: int,
        float: float,
        bool: bool,
        unicode: unicode,
        PyCEGUI.USize: ct.USize,
        PyCEGUI.UVector2: ct.UVector2,
        PyCEGUI.URect: ct.URect,
        PyCEGUI.AspectMode: ct.AspectMode,
        PyCEGUI.HorizontalAlignment: ct.HorizontalAlignment,
        PyCEGUI.VerticalAlignment: ct.VerticalAlignment,
        PyCEGUI.WindowUpdateMode: ct.WindowUpdateMode,
        PyCEGUI.Quaternion: ct.Quaternion,
        PyCEGUI.HorizontalFormatting: ct.HorizontalFormatting,
        PyCEGUI.VerticalFormatting: ct.VerticalFormatting,
        PyCEGUI.HorizontalTextFormatting: ct.HorizontalTextFormatting,
        PyCEGUI.VerticalTextFormatting: ct.VerticalTextFormatting,
        PyCEGUI.ItemListBase.SortMode: ct.SortMode,
        PyCEGUI.Colour: ct.Colour,
        PyCEGUI.ColourRect: ct.ColourRect,
        PyCEGUI.Font: ct.FontRef,
        PyCEGUI.Image: ct.ImageRef,
        PyCEGUI.BasicImage: ct.ImageRef
    }

    def __init__(self, propertyMap, visual):
        self.visual = visual
        self.propertyMap = propertyMap

    @staticmethod
    def getPythonTypeFromCeguiType(ceguiType):
        # Returns a corresponding Python type for a given CEGUI type
        return FalagardElementAttributesManager._typeMap.get(ceguiType, unicode)

    @staticmethod
    def getCeguiTypeTypeFromPythonType(pythonType):
        # Returns a corresponding CEGUI type for a given Python type
        typeMap = FalagardElementAttributesManager._typeMap
        for ceguiTypeEntry, pythonTypeEntry in typeMap.iteritems():
            if pythonTypeEntry == pythonType:
                return ceguiTypeEntry

        raise NotImplementedError("No conversion for this python type to a CEGUI type is known")

    def buildCategories(self, falagardElement):
        """Create all available Properties, PropertyDefinitions and PropertyLinkDefinition options for this WidgetLook
        and categorise them.

        Return the categories, ready to be loaded into an Inspector Widget.
        """
        settingsList = self.createSettingsForFalagardElement(falagardElement)

        categories = FalagardElementSettingCategory.categorisePropertyList(settingsList)

        # sort properties in categories
        for cat in categories.values():
            cat.sortProperties()

        # sort categories by name
        categories = OrderedDict(sorted(categories.items(), key=lambda t: t[0]))

        return OrderedDict(sorted(categories.items()))

    @staticmethod
    def getPythonCeguiTypeAndEditorOptions(propertyMap, category, propertyName, dataType):
        """
        Returns the pythonised cegui type and the editoroptions, using a propertyMap.
        :param propertyMap:
        :param category:
        :param propertyName:
        :param dataType:
        :return: value and propertyType
        """

        editorOptions = None

        # if the current property map specifies a different type, use that one instead
        pmEntry = propertyMap.getEntry(category, propertyName)
        if pmEntry and pmEntry.typeName:
            dataType = pmEntry.typeName

        # Retrieve the EditorSettings if available
        if pmEntry and pmEntry.editorSettings:
            editorOptions = pmEntry.editorSettings

        # get a native data type for the CEGUI data type, falling back to string
        pythonDataType = FalagardElementAttributesManager.getPythonTypeFromCeguiType(dataType)

        return pythonDataType, editorOptions

    @staticmethod
    def getEditorPropertyTypeAndValue(pythonDataType, currentValue):
        """
        Gets the editor options and the pythonised value based on the python data type and the native CEGUI type value.
        Converts the value to the right internal python class for the given cegui type.
        :param pythonDataType:
        :param currentValue:
        :return:
        """
        # get the callable that creates this data type
        # and the Property type to use.
        if issubclass(pythonDataType, ct.Base):
            # if it is a subclass of our ceguitypes, do some special handling
            value = pythonDataType.fromString(pythonDataType.toString(currentValue))
            propertyType = pythonDataType.getPropertyType()
        else:
            if currentValue is None:
                value = None
            elif pythonDataType is bool:
                # The built-in bool parses "false" as True
                # so we replace the default value creator.
                value = ptUtility.boolFromString(currentValue)
            else:
                value = pythonDataType(currentValue)

            propertyType = properties.Property

        return value, propertyType

    def createSettingsForFalagardElement(self, falagardElement):
        """
        Creates a list of settings for any type of Falagard Element (except the WidgetLookFeel itself)
        :param falagardElement:
        :return:
        """

        settings = []

        if falagardElement is None:
            return settings

        from falagard_element_interface import FalagardElementInterface

        attributeList = FalagardElementInterface.getListOfAttributes(falagardElement)

        for attributeName in attributeList:
            attributeValue, attributeCeguiType = FalagardElementInterface.getAttributeValue(falagardElement, attributeName, self.visual.tabbedEditor)
            newSetting = self.createPropertyForFalagardElement(falagardElement, attributeName, attributeValue, attributeCeguiType, "")
            settings.append(newSetting)

        return settings

    def createPropertyForFalagardElement(self, falagardElement, attributeName, attributeValue, attributeCeguiType, helpText):
        """
        Create a FalagardElementEditorProperty based on a type-specific property for the FalagardElement's attribute
        """

        from ceed.editors.looknfeel.falagard_element_interface import FalagardElementInterface
        falagardElementTypeStr = FalagardElementInterface.getFalagardElementTypeAsString(falagardElement)

        # Get the python type representing the cegui type and also the editor options
        pythonDataType, editorOptions = self.getPythonCeguiTypeAndEditorOptions(self.propertyMap, falagardElementTypeStr, attributeName, attributeCeguiType)

        # Get the pythonised type of the value and also its editor-propertytype
        pythonTypeValue, propertyType = self.getEditorPropertyTypeAndValue(pythonDataType, attributeValue)

        # Unmap the reference in case we reference to the WidgetLookFeel
        if attributeName == "look" and falagardElementTypeStr == "SectionSpecification" and pythonTypeValue:
            from ceed.editors.looknfeel.tabbed_editor import LookNFeelTabbedEditor
            pythonTypeValue, _ = LookNFeelTabbedEditor.unmapMappedNameIntoOriginalParts(pythonTypeValue)

        typedProperty = propertyType(name=attributeName,
                                     category=falagardElementTypeStr,
                                     helpText=helpText,
                                     value=pythonTypeValue,
                                     defaultValue=pythonTypeValue,
                                     readOnly=False,
                                     editorOptions=editorOptions,
                                     createComponents=True
                                     )

        # create and return the multi wrapper
        return FalagardElementEditorProperty(typedProperty, falagardElement, attributeName, self.visual)


class FalagardElementSettingCategory(object):
    """ A category for Falagard element settings.
    Categories have a name and hold a list of settings.
    """

    def __init__(self, name):
        """Initialise the instance with the specified name."""
        self.name = name
        self.properties = OrderedDict()

    @staticmethod
    def categorisePropertyList(propertyList, unknownCategoryName="Unknown"):
        """Given a list of settings, creates categories and adds the
        settings to them based on their 'category' field.

        The unknownCategoryName is used for a category that holds all
        properties that have no 'category' specified.
        """
        categories = {}
        for prop in propertyList:
            catName = prop.category if prop.category else unknownCategoryName
            if not catName in categories:
                categories[catName] = FalagardElementSettingCategory(catName)
            category = categories[catName]
            category.properties[prop.name] = prop

        return categories

    def sortProperties(self, reverse=False):
        """ We want to maintain the order used by the FalagardElement interface
        :param reverse:
        :return:
        """
        self.properties = OrderedDict(self.properties.items())