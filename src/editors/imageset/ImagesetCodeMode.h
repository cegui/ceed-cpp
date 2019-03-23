#ifndef IMAGESETCODEMODE_H
#define IMAGESETCODEMODE_H

#include "src/editors/CodeEditMode.h"

class ImagesetCodeMode : public ViewRestoringCodeEditMode
{
public:

    ImagesetCodeMode(MultiModeEditor& editor);

    virtual QString getNativeCode() override;
    virtual bool propagateNativeCode(const QString& code) override;
};

#endif // IMAGESETCODEMODE_H
