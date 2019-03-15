#include "src/editors/NoEditor.h"
#include "qlabel.h"

NoEditor::NoEditor(const QString& filePath, const QString& message)
    : EditorBase(/*nullptr, filePath*/)
{
    auto label = new QLabel(message);
    label->setWordWrap(true);
    widget = new QScrollArea();
    widget->setWidget(label);
}

NoEditor::~NoEditor()
{
    delete widget;
}
