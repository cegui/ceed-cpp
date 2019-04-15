#include "src/cegui/CEGUIPropertyManager.h"

CEGUIPropertyManager::CEGUIPropertyManager()
{

}

/*

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
        "USize": ceguitypes.USize,
        "UVector2": ceguitypes.UVector2,
        "URect": ceguitypes.URect,
        "AspectMode": ceguitypes.AspectMode,
        "HorizontalAlignment": ceguitypes.HorizontalAlignment,
        "VerticalAlignment": ceguitypes.VerticalAlignment,
        "WindowUpdateMode": ceguitypes.WindowUpdateMode,
        "Quaternion": ceguitypes.Quaternion,
        "HorizontalFormatting": ceguitypes.HorizontalFormatting,
        "VerticalFormatting": ceguitypes.VerticalFormatting,
        "HorizontalTextFormatting": ceguitypes.HorizontalTextFormatting,
        "VerticalTextFormatting": ceguitypes.VerticalTextFormatting,
        "SortMode": ceguitypes.SortMode,
        "Colour": ceguitypes.Colour,
        "ColourRect": ceguitypes.ColourRect,
        "Font": ceguitypes.FontRef,
        "Image": ceguitypes.ImageRef
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
        categories = .propertytree.PropertyCategory.categorisePropertyList(propertyList)

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
    def createProperty(ceguiProperty, ceguiSets, propertyMap, multiWrapperType=.propertytree.MultiPropertyWrapper):
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

        if issubclass(pythonDataType, ceguitypes.Base):
            # if it is a subclass of our ceguitypes, do some special handling
            valueCreator = pythonDataType.fromString
            propertyType = pythonDataType.getPropertyType()
        else:
            if pythonDataType is bool:
                # The built-in bool parses "false" as True
                # so we replace the default value creator.
                valueCreator = .propertytree.utility.boolFromString
            else:
                valueCreator = pythonDataType
            propertyType = .propertytree.Property

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
                callback()*/
