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

from PySide import QtCore
from PySide import QtGui
from .qtwidgets import LineEditWithClearButton

from .propertytree import properties
from .propertytree import ui as ptUi
from .propertytree import utility as ptUtility

from ceed.cegui import ceguitypes as ct

from collections import OrderedDict

import PyCEGUI


class PropertyInspectorWidget(QtGui.QWidget):
    """Full blown inspector widget for CEGUI PropertySet(s).

    Requires a call to 'setPropertyManager()' before
    it can show properties via 'setPropertySets'.
    """

    def __init__(self, parent=None):
        super(PropertyInspectorWidget, self).__init__(parent)

        layout = QtGui.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)

        # if the filter starts with this, we only show modified properties
        self.modifiedFilterPrefix = "*"
        self.filterBox = LineEditWithClearButton()
        self.filterBox.setPlaceholderText("Filter (prefix with '{}' to show modified)".format(self.modifiedFilterPrefix))
        self.filterBox.textChanged.connect(self.filterChanged)

        self.selectionLabel = QtGui.QLabel();
        self.selectionLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.selectionLabel.setFrameStyle(QtGui.QFrame.StyledPanel)
        self.selectionLabel.setFrameShadow(QtGui.QFrame.Sunken)

        self.selectionObjectPath = ""
        self.selectionObjectDescription = "Nothing is selected."
        self.selectionLabelTooltip = ""

        self.ptree = ptUi.PropertyTreeWidget()
        """ :type : PropertyTreeWidget"""

        layout.addWidget(self.selectionLabel)
        layout.addWidget(self.filterBox)
        layout.addWidget(self.ptree)

        # set the minimum size to a reasonable value for this widget
        self.setMinimumSize(200, 200)

        self.propertyManager = None

        self.currentSource = []

    def sizeHint(self):
        # we'd rather have this size
        return QtCore.QSize(400, 600)

    def filterChanged(self, filterText):
        if filterText and filterText.startswith(self.modifiedFilterPrefix):
            self.ptree.setFilter(filterText[len(self.modifiedFilterPrefix):], True)
        else:
            self.ptree.setFilter(filterText)

    def setPropertyManager(self, propertyManager):
        self.propertyManager = propertyManager

    def resizeEvent(self, QResizeEvent):
        self.updateSelectionLabelElidedText()

        super(PropertyInspectorWidget, self).resizeEvent(QResizeEvent)

    @staticmethod
    def generateLabelForSet(ceguiPropertySet):
        # We do not know what the property set is but we can take a few informed
        # guesses. Most likely it will be a CEGUI::Window.

        if isinstance(ceguiPropertySet, PyCEGUI.Window):
            return ceguiPropertySet.getNamePath(), ceguiPropertySet.getType()
        else:
            return "", "Unknown PropertySet"

    def setSource(self, source):

        #We check what kind of source we are dealing with
        if type(source) is list:
            if len(source) == 0:
                self.selectionObjectPath = ""
                self.selectionObjectDescription = "Nothing is selected."
                self.selectionLabelTooltip = ""

            elif len(source) == 1:
                self.selectionObjectPath, self.selectionObjectDescription = PropertyInspectorWidget.generateLabelForSet(source[0])

                selectionInfoTuple = (self.selectionObjectPath, self.selectionObjectDescription)
                self.selectionLabelTooltip = " : ".join(selectionInfoTuple)

            else:
                tooltip = ""
                for ceguiPropertySet in source:
                    path, typeName = PropertyInspectorWidget.generateLabelForSet(ceguiPropertySet)
                
                    selectionInfoTuple = (path, typeName)
                    joinedPathAndName = " : ".join(selectionInfoTuple)
                    tooltip += joinedPathAndName + "\n"

                self.selectionObjectPath = ""
                self.selectionObjectDescription = "Multiple selections..."
                self.selectionLabelTooltip = tooltip.rstrip('\n')

        else:
            #Otherwise it must be a FalagardElement
            from ceed.editors.looknfeel.hierarchy_tree_item import LookNFeelHierarchyItem
            falagardEleName, falagardEleTooltip = LookNFeelHierarchyItem.getNameAndToolTip(source, "")
            self.selectionObjectPath = ""
            self.selectionObjectDescription = falagardEleName
            self.selectionLabelTooltip = falagardEleTooltip

        self.updateSelectionLabelElidedText()

        categories = self.propertyManager.buildCategories(source)

        # load them into the tree
        self.ptree.load(categories)

        self.currentSource = source

    def getSources(self):
        return self.currentSource


    def updateSelectionLabelElidedText(self):
        """
        Shortens the window/widget path so that the whole text will fit into the label. The beginning of the, if necessary, cut-off path text will be "...".
        """

        adjustedSelectionObjectPath = ""
        if self.selectionObjectPath:
            adjustedSelectionObjectPath = self.selectionObjectPath + " : "

        fontMetrics = self.selectionLabel.fontMetrics()
        labelWidth = self.selectionLabel.size().width()
        objectDescriptionWidth = fontMetrics.width(self.selectionObjectDescription)
        objectPathWidth = fontMetrics.width(adjustedSelectionObjectPath)
        margin = 6
        minWidthTakenByPath = 20

        if labelWidth > objectDescriptionWidth + objectPathWidth:
            finalText = adjustedSelectionObjectPath + self.selectionObjectDescription
        elif labelWidth < minWidthTakenByPath + objectDescriptionWidth:
            finalText = fontMetrics.elidedText(self.selectionObjectDescription, QtCore.Qt.ElideRight, labelWidth - margin)
        else:
            alteredPathText = fontMetrics.elidedText(adjustedSelectionObjectPath, QtCore.Qt.ElideLeft, labelWidth - margin - objectDescriptionWidth)
            finalText = alteredPathText + self.selectionObjectDescription

        self.selectionLabel.setText(finalText)
        self.selectionLabel.setToolTip(self.selectionLabelTooltip)

class CEGUIPropertyManager(object):
    """Builds propertytree properties from CEGUI properties and PropertySets,
    using a PropertyMap.
    """

    # Maps CEGUI data types (in string form) to Python types

    _typeMap = {
        "int": int,
        "uint": int,
        "float": float,
        "bool": bool,
        "String": unicode,
        "USize": ct.USize,
        "UVector2": ct.UVector2,
        "URect": ct.URect,
        "AspectMode": ct.AspectMode,
        "HorizontalAlignment": ct.HorizontalAlignment,
        "VerticalAlignment": ct.VerticalAlignment,
        "WindowUpdateMode": ct.WindowUpdateMode,
        "Quaternion": ct.Quaternion,
        "HorizontalFormatting": ct.HorizontalFormatting,
        "VerticalFormatting": ct.VerticalFormatting,
        "HorizontalTextFormatting": ct.HorizontalTextFormatting,
        "VerticalTextFormatting": ct.VerticalTextFormatting,
        "SortMode": ct.SortMode,
        "Colour": ct.Colour,
        "ColourRect": ct.ColourRect,
        "Font": ct.FontRef,
        "Image": ct.ImageRef
    }
    # TODO: Font*, Image*, UBox?

    def __init__(self, propertyMap):
        self.propertyMap = propertyMap

    @staticmethod
    def getPythonTypeFromStringifiedCeguiType(ceguiStrType):
        #if not ceguiStrType in CEGUIPropertyManager._typeMap:
        #    print("TODO: " + ceguiStrType)
        return CEGUIPropertyManager._typeMap.get(ceguiStrType, unicode)

    @staticmethod
    def getCEGUIPropertyGUID(ceguiProperty):
        # HACK: The GUID is used as a hash value (to be able
        # to tell if two properties are the same).
        # There's currently no way to get this information, apart
        # from examining the name, datatype, origin etc. of the property
        # and build a string/hash value out of it.
        return "/".join([ceguiProperty.getOrigin(),
                         ceguiProperty.getName(),
                         ceguiProperty.getDataType()])

    def buildCategories(self, ceguiPropertySets):
        """Create all available properties for all CEGUI PropertySets
        and categorise them.

        Return the categories, ready to be loaded into an Inspector Widget.
        """
        propertyList = self.buildProperties(ceguiPropertySets)
        categories = properties.PropertyCategory.categorisePropertyList(propertyList)

        # sort properties in categories
        for cat in categories.values():
            cat.sortProperties()

        # sort categories by name
        categories = OrderedDict(sorted(categories.items(), key=lambda t: t[0]))

        return categories

    def buildProperties(self, ceguiPropertySets):
        """Create and return all available properties for the specified PropertySets."""
        # short name
        cgSets = ceguiPropertySets

        if len(cgSets) == 0:
            return []

        # * A CEGUI property does not have a value, it's similar to a definition
        #   and we need an object that has that property to be able to get a value.
        # * Each CEGUI PropertySet (widget, font, others) has it's own list of properties.
        # * Some properties may be shared across PropertSets but some are not.
        #
        # It's pretty simple to map the properties 1:1 when we have only one
        # set to process. When we have more, however, we need to group the
        # properties that are shared across sets so we display them as one
        # property that affects all the sets that have it.
        # We use getCEGUIPropertyGUID() to determine if two CEGUI properties
        # are the same.

        cgProps = dict()

        for cgSet in cgSets:

            # add a custom attribute to the PropertySet.
            # this will be filled later on with callbacks (see
            # 'createProperty'), one for each property that
            # will be called when the properties of the set change.
            # it's OK to clear any previous value because we only
            # use this internally and we only need a max of one 'listener'
            # for each property.
            # It's not pretty but it does the job well enough.
            setattr(cgSet, "propertyManagerCallbacks", dict())

            propIt = cgSet.getPropertyIterator()

            while not propIt.isAtEnd():
                cgProp = propIt.getCurrentValue()
                guid = self.getCEGUIPropertyGUID(cgProp)

                # if we already know this property, add the current set
                # to the list.
                if guid in cgProps:
                    cgProps[guid][1].append(cgSet)
                # if it's a new property, check if it can be added
                else:
                    # we don't support unreadable properties
                    if cgProp.isReadable():
                        #print("XXX: {}/{}/{}".format(cgProp.getOrigin(), cgProp.getName(), cgProp.getDataType()))
                        # check mapping and ignore hidden properties
                        pmEntry = self.propertyMap.getEntry(cgProp.getOrigin(), cgProp.getName())
                        if (not pmEntry) or (not pmEntry.hidden):
                            cgProps[guid] = (cgProp, [cgSet])

                propIt.next()

        # Convert the CEGUI properties with their sets to property tree properties.
        ptProps = [self.createProperty(ceguiProperty, propertySet) for ceguiProperty, propertySet in cgProps.values()]

        return ptProps

    @staticmethod
    def createProperty(ceguiProperty, ceguiSets, propertyMap, multiWrapperType=properties.MultiPropertyWrapper):
        """Create one MultiPropertyWrapper based property for the CEGUI Property
        for all of the PropertySets specified.
        """
        # get property information
        name = ceguiProperty.getName()
        category = ceguiProperty.getOrigin()
        helpText = ceguiProperty.getHelp()
        readOnly = not ceguiProperty.isWritable()

        # get the CEGUI data type of the property
        propertyDataType = ceguiProperty.getDataType()
        # if the current property map specifies a different type, use that one instead
        pmEntry = propertyMap.getEntry(category, name)
        if pmEntry and pmEntry.typeName:
            propertyDataType = pmEntry.typeName
        # get a native data type for the CEGUI data type, falling back to string
        pythonDataType = CEGUIPropertyManager.getPythonTypeFromStringifiedCeguiType(propertyDataType)

        # get the callable that creates this data type
        # and the Property type to use.
        valueCreator = None

        if issubclass(pythonDataType, ct.Base):
            # if it is a subclass of our ceguitypes, do some special handling
            valueCreator = pythonDataType.fromString
            propertyType = pythonDataType.getPropertyType()
        else:
            if pythonDataType is bool:
                # The built-in bool parses "false" as True
                # so we replace the default value creator.
                valueCreator = ptUtility.boolFromString
            else:
                valueCreator = pythonDataType
            propertyType = properties.Property

        value = None
        defaultValue = None

        # create the inner properties;
        # one property for each CEGUI PropertySet
        innerProperties = []
        for ceguiSet in ceguiSets:
            assert ceguiSet.isPropertyPresent(name), "Property '%s' was not found in PropertySet." % name
            value = valueCreator(ceguiProperty.get(ceguiSet))
            defaultValue = valueCreator(ceguiProperty.getDefault(ceguiSet))

            innerProperty = propertyType(name=name,
                                         category=category,
                                         helpText=helpText,
                                         value=value,
                                         defaultValue=defaultValue,
                                         readOnly=readOnly,
                                         createComponents=False  # no need for components, the template will provide these
            )
            innerProperties.append(innerProperty)

            # hook the inner callback (the 'cb' function) to
            # the value changed notification of the cegui propertyset
            # so that we update our value(s) when the propertyset's
            # property changes because of another editor (i.e. visual, undo, redo)
            def makeCallback(cs, cp, ip):
                def cb():
                    ip.setValue(valueCreator(cp.get(cs)))

                return cb

            ceguiSet.propertyManagerCallbacks[name] = makeCallback(ceguiSet, ceguiProperty, innerProperty)

        # create the template property;
        # this is the property that will create the components
        # and it will be edited.
        editorOptions = None
        if pmEntry and pmEntry.editorSettings:
            editorOptions = pmEntry.editorSettings
        templateProperty = propertyType(name=name,
                                        category=category,
                                        helpText=helpText,
                                        value=value,
                                        defaultValue=defaultValue,
                                        readOnly=readOnly,
                                        editorOptions=editorOptions
        )

        # create the multi wrapper
        multiProperty = multiWrapperType(templateProperty, innerProperties, True)

        return multiProperty

    @staticmethod
    def updateAllValues(ceguiPropertySets):
        """Abuses all property manager callbacks defined for given property sets
        to update all values from them to the respective inspector widgets
        """

        for ceguiPropertySet in ceguiPropertySets:
            if not hasattr(ceguiPropertySet, "propertyManagerCallbacks"):
                continue

            for _, callback in ceguiPropertySet.propertyManagerCallbacks.iteritems():
                callback()