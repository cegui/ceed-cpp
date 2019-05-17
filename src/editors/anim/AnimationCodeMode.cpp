#include "src/editors/anim/AnimationCodeMode.h"

AnimationCodeMode::AnimationCodeMode()
{

}

/*

class CodeEditing(multi.CodeEditMode):
    def __init__(self, tabbedEditor):
        super(CodeEditing, self).__init__()

        self.tabbedEditor = tabbedEditor

    def getNativeCode(self):
        return self.tabbedEditor.visual.generateNativeData()

    def propagateNativeCode(self, code):
        element = None

        try:
            element = ElementTree.fromstring(code)

        except:
            return False

        else:
            self.tabbedEditor.visual.loadFromElement(element)
            return True
*/
