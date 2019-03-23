from PySide import QtCore
from PySide import QtGui
from PySide import QtOpenGL

from OpenGL import GL

import os.path

import PyCEGUI
import PyCEGUIOpenGLRenderer

class RedirectingCEGUILogger(PyCEGUI.Logger):
    """Allows us to register subscribers that want CEGUI log info

    This prevents writing CEGUI.log into CWD and will allow log display inside
    the app in the future
    """

    def __init__(self):
        # don't use super here, PyCEGUI.Logger is an old-style class
        PyCEGUI.Logger.__init__(self)

        self.subscribers = set()

    def registerSubscriber(self, subscriber):
        assert(callable(subscriber))

        self.subscribers.add(subscriber)

    def logEvent(self, message, level):
        for subscriber in self.subscribers:
            subscriber(message, level)

    def setLogFilename(self, name, append):
        # this is just a NOOP to satisfy CEGUI pure virtual method of the same name
        pass
