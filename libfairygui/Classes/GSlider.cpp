#include "GSlider.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN
USING_NS_CC;

using namespace tinyxml2;

GSlider::GSlider() :
    changeOnClick(false),
    canDrag(false),
    _max(100),
    _value(0),
    _titleType(ProgressTitleType::PERCENT),
    _titleObject(nullptr),
    _barObjectH(nullptr),
    _barObjectV(nullptr),
    _barMaxWidth(0),
    _barMaxHeight(0),
    _barMaxWidthDelta(0),
    _barMaxHeightDelta(0),
    _gripObject(nullptr),
    _clickPercent(0),
    _barStartX(0),
    _barStartY(0)
{
}

GSlider::~GSlider()
{

}

void GSlider::setTitleType(ProgressTitleType value)
{
    if (_titleType != value)
    {
        _titleType = value;
        update();
    }
}

void GSlider::setMax(double value)
{
    if (_max != value)
    {
        _max = value;
        update();
    }
}

void GSlider::setValue(double value)
{
    if (_value != value)
    {
        _value = value;
        update();
    }
}

void GSlider::update()
{
    float percent = MIN(_value / _max, 1);
    updateWidthPercent(percent);
}

void GSlider::updateWidthPercent(float percent)
{
    if (_titleObject != nullptr)
    {
        std::ostringstream oss;
        switch (_titleType)
        {
        case ProgressTitleType::PERCENT:
            oss << round(percent * 100) << "%";
            break;

        case ProgressTitleType::VALUE_MAX:
            oss << round(_value) << "/" << round(_max);
            break;

        case ProgressTitleType::VALUE:
            oss << _value;
            break;

        case ProgressTitleType::MAX:
            oss << _max;
            break;
        }
        _titleObject->setText(oss.str());
    }

    float fullWidth = this->getWidth() - _barMaxWidthDelta;
    float fullHeight = this->getHeight() - _barMaxHeightDelta;
    if (!_reverse)
    {
        if (_barObjectH != nullptr)
        {
            /*if (dynamic_cast<GImage*>(_barObjectH) && ((GImage *)_barObjectH)->getFillMethod() != FillMethod::None)
            ((GImage *)_barObjectH).fillAmount = percent;
            else if (dynamic_cast<GLoader*>(_barObjectH) && ((GLoader*)_barObjectH)->getFillMethod() != FillMethod::None)
            ((GLoader *)_barObjectH).fillAmount = percent;
            else*/
            _barObjectH->setWidth(round(fullWidth * percent));
        }
        if (_barObjectV != nullptr)
        {
            /*if (dynamic_cast<GImage*>(_barObjectV) && ((GImage *)_barObjectV)->getFillMethod() != FillMethod::None)
            ((GImage *)_barObjectV).fillAmount = percent;
            else if (dynamic_cast<GLoader*>(_barObjectV) && ((GLoader*)_barObjectV)->getFillMethod() != FillMethod::None)
            ((GLoader *)_barObjectV).fillAmount = percent;
            else*/
            _barObjectV->setHeight(round(fullHeight * percent));
        }
    }
    else
    {
        if (_barObjectH != nullptr)
        {
            /*if (dynamic_cast<GImage*>(_barObjectH) && ((GImage *)_barObjectH)->getFillMethod() != FillMethod::None)
            ((GImage *)_barObjectH).fillAmount = 1 - percent;
            else if (dynamic_cast<GLoader*>(_barObjectH) && ((GLoader*)_barObjectH)->getFillMethod() != FillMethod::None)
            ((GLoader *)_barObjectH).fillAmount = 1 - percent;
            else*/
            {
                _barObjectH->setWidth(round(fullWidth * percent));
                _barObjectH->setX(_barStartX + (fullWidth - _barObjectH->getWidth()));
            }
        }
        if (_barObjectV != nullptr)
        {
            /*if (dynamic_cast<GImage*>(_barObjectV) && ((GImage *)_barObjectV)->getFillMethod() != FillMethod::None)
            ((GImage *)_barObjectV).fillAmount = 1 - percent;
            else if (dynamic_cast<GLoader*>(_barObjectV) && ((GLoader*)_barObjectV)->getFillMethod() != FillMethod::None)
            ((GLoader *)_barObjectV).fillAmount = 1 - percent;
            else*/
            {
                _barObjectV->setHeight(round(fullHeight * percent));
                _barObjectV->setY(_barStartY + (fullHeight - _barObjectV->getHeight()));
            }
        }
    }
}


void GSlider::handleSizeChanged()
{
    GComponent::handleSizeChanged();

    if (_barObjectH != nullptr)
        _barMaxWidth = getWidth() - _barMaxWidthDelta;
    if (_barObjectV != nullptr)
        _barMaxHeight = getHeight() - _barMaxHeightDelta;

    if (!_underConstruct)
        update();
}

void GSlider::constructFromXML(tinyxml2::XMLElement * xml)
{
    xml = xml->FirstChildElement("Slider");
    CCASSERT(xml, "type mismatch");

    const char *p;
    p = xml->Attribute("titleType");
    if (p)
        _titleType = ToolSet::parseProgressTitleType(p);
    _reverse = xml->BoolAttribute("reverse");

    _titleObject = getChild("title");
    _barObjectH = getChild("bar");
    _barObjectV = getChild("bar_v");
    _gripObject = getChild("grip");

    if (_barObjectH != nullptr)
    {
        _barMaxWidth = _barObjectH->getWidth();
        _barMaxWidthDelta = getWidth() - _barMaxWidth;
        _barStartX = _barObjectH->getX();
    }
    if (_barObjectV != nullptr)
    {
        _barMaxHeight = _barObjectV->getHeight();
        _barMaxHeightDelta = getHeight() - _barMaxHeight;
        _barStartY = _barObjectV->getY();
    }

    if (_gripObject != nullptr)
    {
        _gripObject->addEventListener(UIEventType::TouchBegin, CC_CALLBACK_1(GSlider::onGripTouchBegin, this));
        _gripObject->addEventListener(UIEventType::TouchMove, CC_CALLBACK_1(GSlider::onGripTouchMove, this));
    }

    addEventListener(UIEventType::TouchBegin, CC_CALLBACK_1(GSlider::onTouchBegin, this));
}

void GSlider::setup_AfterAdd(tinyxml2::XMLElement * xml)
{
    GComponent::setup_AfterAdd(xml);

    xml = xml->FirstChildElement("Slider");
    if (xml != nullptr)
    {
        int tmp;
        if (xml->QueryIntAttribute("value", &tmp) == 0)
            _value = tmp;

        if (xml->QueryIntAttribute("max", &tmp) == 0)
            _max = tmp;
    }
    update();
}

void GSlider::onTouchBegin(EventContext * context)
{
    if (!changeOnClick)
        return;

    InputEvent* evt = context->getInput();
    Vec2 pt = _gripObject->globalToLocal(evt->getPosition());
    float percent = (float)(_value / _max);
    float delta = 0;
    if (_barObjectH != nullptr)
        delta = (pt.x - _gripObject->getWidth() / 2) / _barMaxWidth;
    if (_barObjectV != nullptr)
        delta = (pt.y - _gripObject->getHeight() / 2) / _barMaxHeight;
    if (_reverse)
        percent -= delta;
    else
        percent += delta;
    if (percent > 1)
        percent = 1;
    else if (percent < 0)
        percent = 0;
    double newValue = percent * _max;
    if (newValue != _value)
    {
        _value = newValue;
        dispatchEvent(UIEventType::Changed);
    }
    updateWidthPercent(percent);
}

void GSlider::onGripTouchBegin(EventContext * context)
{
    canDrag = true;
    context->stopPropagation();
    context->captureTouch();

    _clickPos = globalToLocal(context->getInput()->getPosition());
    _clickPercent = (float)(_value / _max);
}

void GSlider::onGripTouchMove(EventContext * context)
{
    if (!canDrag)
        return;

    Vec2 pt = globalToLocal(context->getInput()->getPosition());

    float deltaX = pt.x - _clickPos.x;
    float deltaY = pt.y - _clickPos.y;
    if (_reverse)
    {
        deltaX = -deltaX;
        deltaY = -deltaY;
    }

    float percent;
    if (_barObjectH != nullptr)
        percent = _clickPercent + deltaX / _barMaxWidth;
    else
        percent = _clickPercent + deltaY / _barMaxHeight;
    if (percent > 1)
        percent = 1;
    else if (percent < 0)
        percent = 0;

    double newValue = percent * _max;
    if (newValue != _value)
    {
        _value = newValue;
        dispatchEvent(UIEventType::Changed);
    }
    updateWidthPercent(percent);
}

NS_FGUI_END
