#include "src/editors/imageset/ImagesetCodeMode.h"

ImagesetCodeMode::ImagesetCodeMode(MultiModeEditor& editor)
    : ViewRestoringCodeEditMode(editor)
{

}

QString ImagesetCodeMode::getNativeCode()
{
/*
        element = self.tabbedEditor.visual.imagesetEntry.saveToElement()
        xmledit.indent(element)

        return ElementTree.tostring(element, "utf-8")
*/
    return "";
}

bool ImagesetCodeMode::propagateNativeCode(const QString& code)
{
    /*
            element = None

            try:
                element = ElementTree.fromstring(code)

            except:
                return False

            else:
                self.tabbedEditor.visual.loadImagesetEntryFromElement(element)
                return True
    */
    return true;
}
