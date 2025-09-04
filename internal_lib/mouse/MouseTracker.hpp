#pragma once
#include "Mouse.hpp"
#include <ApplicationServices/ApplicationServices.h>

void PollMouseMac(MouseCapture& cap);

void startHook();

void MacApplyMouseState(const MouseState& state, MouseState &prevMouseState);
