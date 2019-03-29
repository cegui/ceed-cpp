#ifndef IMAGESETCODEMODE_H
#define IMAGESETCODEMODE_H

#include "src/editors/CodeEditMode.h"

class ImagesetEditor;

class ImagesetCodeMode : public ViewRestoringCodeEditMode
{
public:

    ImagesetCodeMode(ImagesetEditor& editor);

    virtual QString getNativeCode() override;
    virtual bool propagateNativeCode(const QString& code) override;
};

#endif // IMAGESETCODEMODE_H
