#ifndef __GPROGRESSBAR_H__
#define __GPROGRESSBAR_H__

#include "cocos2d.h"
#include "FairyGUIMacros.h"
#include "GComponent.h"

NS_FGUI_BEGIN

class GProgressBar : public GComponent
{
public:
    GProgressBar();
    ~GProgressBar();

    CREATE_FUNC(GProgressBar);

    ProgressTitleType getTitleType() const { return _titleType; }
    void setTitleType(ProgressTitleType value);

    double getMax() const { return _max; }
    void setMax(double value);

    double getValue() const { return _value; }
    void setValue(double value);

protected:
    virtual void handleSizeChanged() override;
    virtual void constructFromXML(tinyxml2::XMLElement* xml) override;
    virtual void setup_AfterAdd(tinyxml2::XMLElement* xml) override;

    void update();

private:
    double _max;
    double _value;
    ProgressTitleType _titleType;
    bool _reverse;

    GObject* _titleObject;
    GObject* _barObjectH;
    GObject* _barObjectV;
    float _barMaxWidth;
    float _barMaxHeight;
    float _barMaxWidthDelta;
    float _barMaxHeightDelta;
    float _barStartX;
    float _barStartY;
};

NS_FGUI_END

#endif
