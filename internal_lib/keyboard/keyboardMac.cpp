#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include "keyboard.hpp"

// Capture thread using Quartz Event Tap
CGEventRef keyboardCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
    if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
        KeyboardCapture* kCapture = KeyboardCapture::GetInstance();

        // Get keycode (hardware code, not VK like Windows)
        CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        bool isKeyDown = (type == kCGEventKeyDown);
        kCapture->push(isKeyDown ? 1 : 0, static_cast<int>(keyCode));

        // std::cout << (isKeyDown ? "Key Down: " : "Key Up: ") << keyCode << std::endl;
    }

    return event; // pass event along
}

void startKeyboardTrack() {
    // Create event tap
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp);
    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap,       // listen to user session events
        kCGHeadInsertEventTap,    // insert at head
        0,                        // no options
        eventMask,                // listen to key up + key down
        keyboardCallback,
        nullptr
    );

    if (!eventTap) {
        std::cerr << "Failed to create event tap! Run with accessibility permissions." << std::endl;
        return;
    }

    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);

    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    CFRunLoopRun(); // block forever, process events
}

// --- Apply key input (synthesizing events) ---
void WinApplyKeyInput(bool isKeyDown, int vkCode) {
    // Note: vkCode here is a hardware keycode on macOS, not Windows VK
    CGEventRef event = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)vkCode, isKeyDown);
    if (event) {
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
}
