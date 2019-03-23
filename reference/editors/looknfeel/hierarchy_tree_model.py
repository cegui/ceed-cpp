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

from PySide import QtCore
from PySide import QtGui
import cPickle

from hierarchy_tree_item import LookNFeelHierarchyItem

class LookNFeelHierarchyTreeModel(QtGui.QStandardItemModel):
    def __init__(self, dockWidget):
        super(LookNFeelHierarchyTreeModel, self).__init__()

        self.dockWidget = dockWidget
        """ :type : LookNFeelHierarchyDockWidget """
        self.setItemPrototype(LookNFeelHierarchyItem(self))

        self.widgetLookObject = None

        self.limitDisplayToStateImagery = None
        """ A string defining a name of a StateImagery. This StateImagery, and everything referenced within it, will be the only one
        to be displayed in the hierarchy. The value None means there won't be any limitation and everything will be displayed. """

    def updateTree(self, widgetLookObject, limitDisplayTo):
        """
        Sets the WidgetLookFeel object based on which this widget should create the hierarchy.
        Rebuilds the hierarchy tree based on a WidgetFeelLook object.
        :param widgetLookObject: PyCEGUI.WidgetLookFeel
        :param limitDisplayTo: str
        :return:
        """

        self.limitDisplayToStateImagery = limitDisplayTo
        self.widgetLookObject = widgetLookObject

        # Clear any existing hierarchy
        self.clear()

        if self.widgetLookObject is None:
            return

        self.appendAllWidgetLookFeelChildren(self.widgetLookObject)

    def appendAllWidgetLookFeelChildren(self, widgetLookObject):
        """
        Iterates over all contained elements of the WidgetLookFeel. Creates the contained elements as
        new items and appends them to this tree view's root item
        :param widgetLookObject: PyCEGUI.WidgetLookFeel
        :return:
        """

        # Add all properties

        propertyDefMap = widgetLookObject.getPropertyDefinitionMap(False)
        if propertyDefMap:
            categoryItem = self.createAndAddCategoryToRoot("PropertyDefinitions", "type: PropertyDefinitionBase")
            for propDefMapEntry in propertyDefMap:
                currentPropertyDef = PyCEGUI.Workarounds.PropertyDefinitionBaseMapGet(propertyDefMap, propDefMapEntry.key)
                self.createAndAddItem(currentPropertyDef, categoryItem)

        propertyLinkDefMap = widgetLookObject.getPropertyLinkDefinitionMap(False)
        if propertyLinkDefMap:
            categoryItem = self.createAndAddCategoryToRoot("PropertyLinkDefinitions", "type: PropertyDefinitionBase")
            for propLinkDefMapEntry in propertyLinkDefMap:
                currentPropertyLinkDef = PyCEGUI.Workarounds.PropertyDefinitionBaseMapGet(propertyLinkDefMap, propLinkDefMapEntry.key)
                self.createAndAddItem(currentPropertyLinkDef, categoryItem)

        propertyInitialiserMap = widgetLookObject.getPropertyInitialiserMap(False)
        if propertyInitialiserMap:
            categoryItem = self.createAndAddCategoryToRoot("Properties", "type: PropertyInitialiser")
            for propertyInitialiserMapEntry in propertyInitialiserMap:
                currentPropertyInitialiser = PyCEGUI.Workarounds.PropertyInitialiserMapGet(propertyInitialiserMap, propertyInitialiserMapEntry.key)
                self.createAndAddItem(currentPropertyInitialiser, categoryItem)

        # Create and add all view-dependent hierarchy items owned by the WidgetLookFeel

        namedAreaMap = widgetLookObject.getNamedAreaMap(False)
        if namedAreaMap:
            categoryItem = self.createAndAddCategoryToRoot("NamedAreas", "type: NamedArea")
            for namedAreaMapEntry in namedAreaMap:
                currentNamedArea = PyCEGUI.Workarounds.NamedAreaMapGet(namedAreaMap, namedAreaMapEntry.key)
                self.createAndAddItem(currentNamedArea, categoryItem)

        # Gather all elements associated with the currently selected view
        stateImageryNames, imagerySectionNames = self.getViewDependentElementNames(widgetLookObject)

        imagerySectionMap = widgetLookObject.getImagerySectionMap(False)
        if imagerySectionNames:
            categoryItem = self.createAndAddCategoryToRoot("ImagerySections", "type: ImagerySection")
            for imagerySectionName in imagerySectionNames:
                currentImagerySection = PyCEGUI.Workarounds.ImagerySectionMapGet(imagerySectionMap, imagerySectionName)
                self.createAndAddItem(currentImagerySection, categoryItem)

        stateImageryMap = widgetLookObject.getStateImageryMap(False)
        if stateImageryNames:
            categoryItem = self.createAndAddCategoryToRoot("StateImageries", "type: StateImagery")
            for stateImageryName in stateImageryNames:
                currentStateImagery = PyCEGUI.Workarounds.StateImageryMapGet(stateImageryMap, stateImageryName)
                self.createAndAddItem(currentStateImagery, categoryItem)

        widgetComponentMap = widgetLookObject.getWidgetComponentMap(False)
        if widgetComponentMap:
            categoryItem = self.createAndAddCategoryToRoot("WidgetComponents", "type: WidgetComponent")
            for widgetComponentMapEntry in widgetComponentMap:
                widgetComponent = PyCEGUI.Workarounds.WidgetComponentMapGet(widgetComponentMap, widgetComponentMapEntry.key)
                self.createAndAddItem(widgetComponent, categoryItem)

    def getViewDependentElementNames(self, widgetLookObject):
        """
        Returns the StateImagery, ImagerySection, WidgetComponent and NamedArea names associated with the current view
        :param widgetLookObject: PyCEGUI.WidgetLookFeel
        :return:
        """

        # We get all names of all elements
        stateImageryNames = widgetLookObject.getStateImageryNames(True)
        imagerySectionNames = widgetLookObject.getImagerySectionNames(True)

        # If the current mode is unlimited, return all names unaltered
        if self.limitDisplayToStateImagery is None:
            return stateImageryNames, imagerySectionNames

        # We retrieve the StateImagery object that we want to display exclusively
        stateImageryMap = widgetLookObject.getStateImageryMap(True)
        viewedStateImagery = PyCEGUI.Workarounds.StateImageryMapGet(stateImageryMap, self.limitDisplayToStateImagery)

        # We iterate over all layers and all SectionSpecification in the layers, looking for all
        # ImagerySections that are referenced from there and are "owned" by this WidgetLookFeel
        # (They could also be inside another WLF definition, in which case we won't display them)
        # All such ImagerySections will be added to a list
        referencedImagerySections = []
        layerSpecList = viewedStateImagery.getLayerSpecificationPointers()
        for layerSpec in layerSpecList:
            sectionSpecList = layerSpec.getSectionSpecificationPointers()
            for sectionSpec in sectionSpecList:
                ownerWidgetFeel = sectionSpec.getOwnerWidgetLookFeel()
                if ownerWidgetFeel == self.widgetLookObject.getName():
                    referencedImagerySections.append(sectionSpec.getSectionName())

        #We make each ImagerySection unique using a set
        imagerySectionNamesSet = set(referencedImagerySections)
        imagerySectionNames = list(imagerySectionNamesSet)

        return [self.limitDisplayToStateImagery], imagerySectionNames

    def createAndAddCategoryToRoot(self, name, toolTip):
        """
        Creates a category item based on the supplied name and tooltip
        :param name:
        :param toolTip:
        :return: QtGui.QStandardItem
        """
        rootItem = self.invisibleRootItem()

        categoryItem = QtGui.QStandardItem(name)
        categoryItem.setToolTip(toolTip)

        from ceed.propertytree.ui import PropertyCategoryRow
        PropertyCategoryRow.setupCategoryOptions(categoryItem)

        rootItem.appendRow(categoryItem)

        return categoryItem

    @staticmethod
    def createAndAddItem(falagardElement, parentItem):
        """
        Creates an item based on the supplied object and appends it to the rootItem of this tree model
        :param falagardElement:
        :param parentItem: QtGui.QStandardItem
        :return: QtGui.QStandardItem
        """
        if falagardElement is None:
            raise Exception("Invalid parameter supplied to LookNFeelHierarchyTreeModel.createAndAddItem")

        newItem = LookNFeelHierarchyItem(falagardElement)
        parentItem.appendRow(newItem)

        return newItem

    def data(self, index, role=QtCore.Qt.DisplayRole):
        return super(LookNFeelHierarchyTreeModel, self).data(index, role)

    def setData(self, index, value, role=QtCore.Qt.EditRole):
        return super(LookNFeelHierarchyTreeModel, self).setData(index, value, role)

    def flags(self, index):
        return super(LookNFeelHierarchyTreeModel, self).flags(index)

    def mimeData(self, indexes):
        # if the selection contains children of something that is also selected, we don't include that
        # (it doesn't make sense to move it anyways, it will be moved with its parent)

        def isChild(parent, potentialChild):
            i = 0
            # DFS, Qt doesn't have helper methods for this it seems to me :-/
            while i < parent.rowCount():
                child = parent.child(i)

                if child is potentialChild:
                    return True

                if isChild(child, potentialChild):
                    return True

                i += 1

            return False

        topItems = []

        for index in indexes:
            item = self.itemFromIndex(index)
            hasParent = False

            for parentIndex in indexes:
                if parentIndex is index:
                    continue

                potentialParent = self.itemFromIndex(parentIndex)

                if isChild(item, potentialParent):
                    hasParent = True
                    break

            if not hasParent:
                topItems.append(item)

        data = []
        for item in topItems:
            data.append(item.data(QtCore.Qt.UserRole))

        ret = QtCore.QMimeData()
        ret.setData("application/x-ceed-widget-paths", cPickle.dumps(data))

        return ret

    def mimeTypes(self):
        return ["application/x-ceed-widget-paths", "application/x-ceed-widget-type"]

    def dropMimeData(self, data, action, row, column, parent):
        return False