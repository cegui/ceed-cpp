#include "src/util/SettingsEntry.h"

SettingsEntry::SettingsEntry()
{

}

/*
    value = property(fset = lambda entry, value: entry._setValue(value),
                     fget = lambda entry: entry._value)
    editedValue = property(fset = lambda entry, value: entry._setEditedValue(value),
                           fget = lambda entry: entry._editedValue)

    def __init__(self, section, name, type_, defaultValue, label = None, help_ = "", widgetHint = "string", sortingWeight = 0, changeRequiresRestart = False, optionList = None):
        self.section = section

        if label is None:
            label = name

        self.name = name
        self.type = type_

        defaultValue = self.sanitizeValue(defaultValue)
        self.defaultValue = defaultValue
        self._value = defaultValue
        self._editedValue = defaultValue

        self.label = label
        self.help = help_
        self.hasChanges = False
        self.widgetHint = widgetHint

        self.sortingWeight = sortingWeight

        self.changeRequiresRestart = changeRequiresRestart

        self.optionList = optionList

        self.subscribers = []

    def getPath(self):
        """Retrieves a unique path in the qsettings tree, this can be used by persistence providers for example
        """

        return "%s/%s" % (self.section.getPath(), self.name)

    def getPersistenceProvider(self):
        return self.section.getPersistenceProvider()

    def getSettings(self):
        return self.section.getSettings()

    def sanitizeValue(self, value):
        if not isinstance(value, self.type):
            value = self.type(value)

        return value

    def subscribe(self, callable_):
        """Subscribes a callable that gets called when the value changes (the real value, not edited value!)
        (with current value as the argument)
        """

        self.subscribers.append(callable_)

    def unsubscribe(self, callable_):
        self.subscribers.remove(callable_)

    def _setValue(self, value, uploadImmediately = True):
        value = self.sanitizeValue(value)

        self._value = value
        self._editedValue = value
        if uploadImmediately:
            self.upload()

        for callable_ in self.subscribers:
            callable_(value)

    def _setEditedValue(self, value):
        value = self.sanitizeValue(value)

        self._editedValue = value
        self.hasChanges = True

    def markAsChanged(self):
        if not self.label.startswith("* "):
            self.label = " ".join(["*", self.label])

    def markAsUnchanged(self):
        if self.label.startswith("* "):
            self.label = self.label[2:]

    def applyChanges(self):
        if self.value != self.editedValue:
            self.value = self.editedValue

            if self.changeRequiresRestart:
                self.getSettings().markRequiresRestart()

    def discardChanges(self):
        self.editedValue = self.value

        # - This is set via the property, but from this context we know that
        #   we really don't have any changes.
        self.hasChanges = False

    def upload(self):
        self.getPersistenceProvider().upload(self, self._value)
        self.hasChanges = False

    def download(self):
        persistedValue = self.getPersistenceProvider().download(self)
        if persistedValue is None:
            persistedValue = self.defaultValue

        # http://bugs.pyside.org/show_bug.cgi?id=345
        if self.widgetHint == "checkbox":
            if persistedValue == "false":
                persistedValue = False
            elif persistedValue == "true":
                persistedValue = True

        self._setValue(persistedValue, False)
        self.hasChanges = False
*/
