#include "src/editors/anim/AnimationEditor.h"
#include "src/util/DismissableMessage.h"

AnimationEditor::AnimationEditor(const QString& filePath)
    : MultiModeEditor(/*animation_list_compatibility.manager, */ filePath)
{
    DismissableMessage::warning(nullptr, "Animation List Editor is experimental!",
                                "This part of CEED is not considered to be ready for "
                                "production. You have been warned. If everything "
                                "breaks you get to keep the pieces!",
                                "animation_list_editor_experimental");
/*
        self.visual = visual.VisualEditing(self)
        self.addTab(self.visual, "Visual")

        self.code = code.CodeEditing(self)
        self.addTab(self.code, "Code")
*/
}

QString AnimationEditor::getFileTypesDescription() const
{
    return AnimationEditorFactory::animationFileTypesDescription();
}

QStringList AnimationEditor::getFileExtensions() const
{
    return AnimationEditorFactory::animationFileExtensions();
}

QString AnimationEditor::getDefaultFolder(CEGUIProject* /*project*/) const
{
    return "";
}

/*
    def initialise(self, mainWindow):
        super(AnimationListTabbedEditor, self).initialise(mainWindow)

        # We do something most people would not expect here,
        # instead of asking CEGUI to load the animation list as it is,
        # we parse it ourself, mine each and every animation from it,
        # and use these chunks of code to load every animation one at a time

        # the reason we do this is more control (CEGUI just adds the animation
        # list to the pool of existing animations, we don't want to pollute that
        # pool and we want to group all loaded animations)

        root = None
        try:
            root = ElementTree.fromstring(self.nativeData)

        except:
            # things didn't go smooth
            # 2 reasons for that
            #  * the file is empty
            #  * the contents of the file are invalid
            #
            # In the first case we will silently move along (it is probably just a new file),
            # in the latter we will output a message box informing about the situation

            # the file should exist at this point, so we are not checking and letting exceptions
            # fly out of this method
            if os.path.getsize(self.filePath) > 2:
                # the file contains more than just CR LF
                QtGui.QMessageBox.question(self,
                                           "Can't parse given animation list!",
                                           "Parsing '%s' failed, it's most likely not a valid XML file. "
                                           "Constructing empty animation list instead (if you save you will override the invalid data!). "
                                           "Exception details follow:\n%s" % (self.filePath, sys.exc_info()[1]),
                                           QtGui.QMessageBox.Ok)

            # we construct the minimal empty imageset
            root = ElementTree.Element("Animations")

        self.visual.loadFromElement(root)

    def finalise(self):
        # this takes care of destroying the temporary animation instance, if any
        self.visual.setCurrentAnimation(None)

        super(AnimationListTabbedEditor, self).finalise()

    def activate(self):
        super(AnimationListTabbedEditor, self).activate()

        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.animationListDockWidget)
        self.visual.animationListDockWidget.setVisible(True)
        self.mainWindow.addDockWidget(QtCore.Qt.RightDockWidgetArea, self.visual.propertiesDockWidget)
        self.visual.propertiesDockWidget.setVisible(True)
        self.mainWindow.addDockWidget(QtCore.Qt.RightDockWidgetArea, self.visual.keyFramePropertiesDockWidget)
        self.visual.keyFramePropertiesDockWidget.setVisible(True)
        self.mainWindow.addDockWidget(QtCore.Qt.BottomDockWidgetArea, self.visual.timelineDockWidget)
        self.visual.timelineDockWidget.setVisible(True)

    def deactivate(self):
        self.mainWindow.removeDockWidget(self.visual.animationListDockWidget)
        self.mainWindow.removeDockWidget(self.visual.propertiesDockWidget)
        self.mainWindow.removeDockWidget(self.visual.keyFramePropertiesDockWidget)
        self.mainWindow.removeDockWidget(self.visual.timelineDockWidget)

        super(AnimationListTabbedEditor, self).deactivate()

    def saveAs(self, targetPath, updateCurrentPath = True):
        codeMode = self.currentWidget() is self.code

        # if user saved in code mode, we process the code by propagating it to visual
        # (allowing the change propagation to do the code validating and other work for us)

        if codeMode:
            self.code.propagateToVisual()

        self.nativeData = self.visual.generateNativeData()

        return super(AnimationListTabbedEditor, self).saveAs(targetPath, updateCurrentPath)

    def zoomIn(self):
        if self.currentWidget() is self.visual:
            return self.visual.zoomIn()

        return False

    def zoomOut(self):
        if self.currentWidget() is self.visual:
            return self.visual.zoomOut()

        return False

    def zoomReset(self):
        if self.currentWidget() is self.visual:
            return self.visual.zoomReset()

        return False
*/
//---------------------------------------------------------------------

QString AnimationEditorFactory::animationFileTypesDescription()
{
    return "Animation set";
}

QStringList AnimationEditorFactory::animationFileExtensions()
{
    /*
        extensions = animation_list_compatibility.manager.getAllPossibleExtensions()
    */
    return { "anims" };
}

EditorBasePtr AnimationEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<AnimationEditor>(filePath);
}
