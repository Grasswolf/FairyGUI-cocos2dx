#include "UIEventDispatcher.h"
#include "GComponent.h"
#include "InputProcessor.h"

USING_NS_CC;
NS_FGUI_BEGIN

UIEventDispatcher::UIEventDispatcher() :_dispatching(0)
{
}

UIEventDispatcher::~UIEventDispatcher()
{

}

void UIEventDispatcher::addEventListener(int eventType, const EventCallback& callback, int tag)
{
    if (tag != Node::INVALID_TAG)
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); it++)
        {
            if (it->eventType == eventType && it->tag == tag)
            {
                it->callback = callback;
                return;
            }
        }
    }

    EventCallbackItem item;
    item.callback = callback;
    item.eventType = eventType;
    item.tag = tag;
    item.dispatching = 0;
    _callbacks.push_back(item);
}

void UIEventDispatcher::removeEventListener(int eventType, int tag)
{
    for (auto it = _callbacks.begin(); it != _callbacks.end(); )
    {
        if (it->eventType == eventType && (it->tag == tag || tag == Node::INVALID_TAG))
        {
            if (_dispatching > 0)
            {
                it->callback = nullptr;
                it++;
            }
            else
                it = _callbacks.erase(it);
        }
        else
            it++;
    }
}

void UIEventDispatcher::removeEventListeners()
{
    if (_dispatching > 0)
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); ++it)
            it->callback = nullptr;
    }
    else
        _callbacks.clear();
}

bool UIEventDispatcher::hasEventListener(int eventType, int tag) const
{
    for (auto it = _callbacks.cbegin(); it != _callbacks.cend(); ++it)
    {
        if (it->eventType == eventType && (it->tag == tag || tag == Node::INVALID_TAG) && it->callback != nullptr)
            return true;
    }
    return false;
}

bool UIEventDispatcher::dispatchEvent(int eventType, void* data)
{
    if (_callbacks.size() == 0)
        return false;

    EventContext context;
    context._sender = this;
    context._inputEvent = InputProcessor::getRecentInput();
    context._data = data;

    doDispatch(eventType, &context);

    return context._defaultPrevented;
}

bool UIEventDispatcher::bubbleEvent(int eventType, void* data)
{
    EventContext context;
    context._inputEvent = InputProcessor::getRecentInput();
    context._data = data;

    doBubble(eventType, &context);

    return context._defaultPrevented;
}

bool UIEventDispatcher::isDispatchingEvent(int eventType)
{
    for (auto it = _callbacks.begin(); it != _callbacks.end(); ++it)
    {
        if (it->eventType == eventType)
            return it->dispatching > 0;
    }
    return false;
}

void UIEventDispatcher::doDispatch(int eventType, EventContext* context)
{
    _dispatching++;

    retain();
    context->_sender = this;
    int cnt = _callbacks.size(); //dont use iterator, because new item would be added in callback.
    for (int i = 0; i < cnt; i++)
    {
        EventCallbackItem& ci = _callbacks[i];
        if (ci.eventType == eventType && ci.callback != nullptr)
        {
            ci.dispatching++;
            context->_touchCapture = 0;
            ci.callback(context);
            ci.dispatching--;
            if (context->_touchCapture != 0)
            {
                if (context->_touchCapture == 1 && eventType == UIEventType::TouchBegin)
                    ((InputProcessor*)context->_data)->addTouchMonitor(context->getInput()->getTouchId(), this);
                else if (context->_touchCapture == 2)
                    ((InputProcessor*)context->_data)->removeTouchMonitor(this);
            }
        }
    }
    for (auto it = _callbacks.begin(); it != _callbacks.end(); )
    {
        if (it->callback == nullptr)
            it = _callbacks.erase(it);
        else
            it++;
    }
    release();

    _dispatching--;
}

void UIEventDispatcher::doBubble(int eventType, EventContext* context)
{
    if (!_callbacks.empty())
    {
        context->_isStopped = false;
        doDispatch(eventType, context);
        if (context->_isStopped)
            return;
    }

    GComponent* p = ((GObject*)this)->getParent();
    if (p)
        p->doBubble(eventType, context);
}

NS_FGUI_END