#include "src/editors/imageset/ImagesetCodeMode.h"
#include "src/editors/imageset/ImagesetEditor.h"
#include "src/editors/imageset/ImagesetVisualMode.h"
#include "src/ui/XMLSyntaxHighlighter.h"
#include "qdom.h"

ImagesetCodeMode::ImagesetCodeMode(ImagesetEditor& editor)
    : ViewRestoringCodeEditMode(editor)
{
    setAcceptRichText(false);
    highlighter = new XMLSyntaxHighlighter(this);
}

QString ImagesetCodeMode::getNativeCode()
{
    return static_cast<ImagesetEditor&>(_editor).getSourceCode();
}

bool ImagesetCodeMode::propagateNativeCode(const QString& code)
{
    QDomDocument doc;
    if (!doc.setContent(code)) return false;
    static_cast<ImagesetEditor&>(_editor).getVisualMode()->loadImagesetEntryFromElement(doc.documentElement());
    return true;
}
