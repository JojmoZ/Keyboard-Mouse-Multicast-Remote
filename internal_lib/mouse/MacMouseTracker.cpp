#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <ApplicationServices/ApplicationServices.h>
#include "Mouse.hpp"
#include "MouseTracker.hpp"

#define SLEEP_DURATION 20 // ms

// For scroll accumulation
static std::atomic<int> scrollDelta(0);

// Callback for global mouse events (scroll mainly)
CGEventRef MouseEventCallback(CGEventTapProxy proxy, CGEventType type,
                              CGEventRef event, void* userInfo)
{
    if (type == kCGEventScrollWheel) {
        int64_t delta = CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1);
        scrollDelta += static_cast<int>(delta);
    }
    return event;
}

// Start global event tap for scroll
void startHook() {
    std::thread([](){
        CGEventMask eventMask = (1 << kCGEventScrollWheel);
        CFMachPortRef eventTap = CGEventTapCreate(
            kCGSessionEventTap,
            kCGHeadInsertEventTap,
            0,
            eventMask,
            MouseEventCallback,
            nullptr);

        if (!eventTap) {
            std::cerr << "Failed to create event tap. Run with accessibility permissions enabled.\n";
            return;
        }

        CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(
            kCFAllocatorDefault, eventTap, 0);

        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
        CGEventTapEnable(eventTap, true);

        CFRunLoopRun();

        CFRelease(runLoopSource);
        CFRelease(eventTap);
    }).detach();
}

// Poll mouse position and buttons
void PollMouseMac(MouseCapture& cap) {
    startHook(); // hook for scroll

    CGPoint prevPos = { -1, -1 };
    bool prevLeft = false, prevRight = false, prevMiddle = false;

    while (true) {
        // Position
        CGEventRef event = CGEventCreate(NULL);
        CGPoint pos = CGEventGetLocation(event);
        CFRelease(event);

        // Buttons
        bool leftNow   = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonLeft);
        bool rightNow  = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonRight);
        bool middleNow = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonCenter);

        // Scroll
        int dz = scrollDelta.exchange(0);

        int dx = static_cast<int>(pos.x);
        int dy = static_cast<int>(pos.y);

        if (dx != prevPos.x || dy != prevPos.y ||
            leftNow != prevLeft || rightNow != prevRight ||
            middleNow != prevMiddle || dz != 0) {

            cap.push(dx, dy, dz, leftNow, rightNow, middleNow);

            prevPos = pos;
            prevLeft = leftNow;
            prevRight = rightNow;
            prevMiddle = middleNow;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION));
    }
}

void MacApplyMouseState(const MouseState& state, MouseState& prevState) {
    // --- Move mouse ---
    if (state.dx != 0 || state.dy != 0) {
        CGPoint newPos = CGPointMake(state.dx, state.dy);
        CGWarpMouseCursorPosition(newPos);
        CGAssociateMouseAndMouseCursorPosition(true);
    }

    // --- Scroll ---
    if (state.dScroll != 0) {
        CGEventRef scroll = CGEventCreateScrollWheelEvent(
            NULL, kCGScrollEventUnitLine, 1, state.dScroll);
        CGEventPost(kCGHIDEventTap, scroll);
        CFRelease(scroll);
    }

    // --- Left click ---
    if (state.leftClick != prevState.leftClick) {
        CGEventRef click = CGEventCreateMouseEvent(
            NULL,
            state.leftClick ? kCGEventLeftMouseDown : kCGEventLeftMouseUp,
            CGEventGetLocation(CGEventCreate(NULL)),
            kCGMouseButtonLeft);
        CGEventPost(kCGHIDEventTap, click);
        CFRelease(click);
        prevState.leftClick = state.leftClick;
    }

    // --- Right click ---
    if (state.rightClick != prevState.rightClick) {
        CGEventRef click = CGEventCreateMouseEvent(
            NULL,
            state.rightClick ? kCGEventRightMouseDown : kCGEventRightMouseUp,
            CGEventGetLocation(CGEventCreate(NULL)),
            kCGMouseButtonRight);
        CGEventPost(kCGHIDEventTap, click);
        CFRelease(click);
        prevState.rightClick = state.rightClick;
    }

    // --- Middle click ---
    if (state.midClick != prevState.midClick) {
        CGEventRef click = CGEventCreateMouseEvent(
            NULL,
            state.midClick ? kCGEventOtherMouseDown : kCGEventOtherMouseUp,
            CGEventGetLocation(CGEventCreate(NULL)),
            kCGMouseButtonCenter);
        CGEventPost(kCGHIDEventTap, click);
        CFRelease(click);
        prevState.midClick = state.midClick;
    }
}
