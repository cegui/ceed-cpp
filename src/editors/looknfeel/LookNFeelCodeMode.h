#ifndef LOOKNFEELCODEMODE_H
#define LOOKNFEELCODEMODE_H

#include "src/editors/CodeEditMode.h"

class LookNFeelEditor;

class LookNFeelCodeMode : public CodeEditMode
{
public:

    LookNFeelCodeMode(LookNFeelEditor& editor);

    virtual QString getNativeCode() override;
    virtual bool propagateNativeCode(const QString& code) override;
    virtual void refreshFromVisual() override;

    void moveToAndSelectWidgetLookFeel(QString widgetLookFeelName);
};

#endif // LOOKNFEELCODEMODE_H
