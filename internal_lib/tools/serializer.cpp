#include "serializer.hpp"
#include <iostream>
#include <cstring>
void clearBuff(uint8_t *buf, int len ){
    for (int i=0; i<len; i++){
        buf[i]= 0x00;
    }
}

bool isMouseData(uint8_t *buf, int len){
    if (buf[0] == MOUSE_ACTION){
        return true;
    }
    return false;
}

bool isKeyboardData(uint8_t *buf, int len){
    if (buf[0] == KEYBOARD_ACTION){
        return true;
    }
    return false;
}

void formatMouseData(MouseState &mState, uint8_t *buf, int len) {
    clearBuff(buf, len);
    buf[0] = MOUSE_ACTION;

    if (mState.dx != 0 ||  mState.dy != 0){
        buf[1] |= MOUSE_MOVE;        
        memcpy(buf + 4, &mState.dx, sizeof(mState.dx));   // x
        memcpy(buf + 8, &mState.dy, sizeof(mState.dy));   // y
    }
    if (mState.dScroll !=0 ){
        buf[1] |= MOUSE_SCROLL;
        memcpy(buf + 12, &mState.dScroll, sizeof(mState.dScroll));  // z   
    }
    
    if (mState.leftClick) buf[1] |= MOUSE_LEFT_CLICK;
    if (mState.rightClick) buf[1] |= MOUSE_RIGHT_CLICK;
    if (mState.midClick) buf[1] |= MOUSE_MID_CLICK;

    
}

void parseMouseData(MouseState &mState, const uint8_t *buf, int len) {
    
    uint8_t opCode = buf[0];

    // DEBUG
    if (opCode != MOUSE_ACTION){
        std::cout<<"opCode incorrect for mouse action"<<std::endl;
        return;
    }

    // Reset all fields first
    mState.dx = 0;
    mState.dy = 0;
    mState.dScroll = 0;
    mState.leftClick = false;
    mState.rightClick = false;
    mState.midClick = false;

    // Sub-operation code
    uint8_t subOp = buf[1];

    // Movement
    if ( (subOp & MOUSE_MOVE) == MOUSE_MOVE) {
        memcpy(&mState.dx, buf + 4, sizeof(mState.dx));
        memcpy(&mState.dy, buf + 8, sizeof(mState.dy));
    }

    // Clicks
    if ( (subOp & MOUSE_LEFT_CLICK) == MOUSE_LEFT_CLICK) mState.leftClick = true;
    if ( (subOp & MOUSE_RIGHT_CLICK) == MOUSE_RIGHT_CLICK) mState.rightClick = true;
    if ( (subOp & MOUSE_MID_CLICK) == MOUSE_MID_CLICK) mState.midClick = true;

    // Scroll (if used)
    if ( (subOp & MOUSE_SCROLL) == MOUSE_SCROLL) {
        memcpy(&mState.dScroll, buf + 12, sizeof(mState.dScroll));
    }
}

void formatKeyboardData(KeyboardState &kState, uint8_t *buf, int len){
    clearBuff(buf, len);
    buf[0] |= KEYBOARD_ACTION;
    if (kState.press){
        buf[1] |= KEYBOARD_KEYDOWN;
    }
    // karena keyboard down 0x00, skip aja kalau memang down
    memcpy(buf+4, &kState.code, sizeof(kState.code));  // x
}

void parseKeyboardData(KeyboardState &kState, const uint8_t *buf, int len){
    kState.code =0;
    kState.press =0;
    uint8_t opCode = buf[0];

    // DEBUG
    if (opCode != KEYBOARD_ACTION){
        std::cout<<"opCode incorrect for KeyboadAction"<<std::endl;
        return;
    }
    
    if ( (buf[1] & KEYBOARD_KEYDOWN) == KEYBOARD_KEYDOWN){
        kState.press = 1;
        memcpy(&kState.code, buf+4, sizeof(kState.code));
    }else{
        // temporary: buat sementara pake else dulu karena simple
        kState.press = 0;
        memcpy(&kState.code, buf+4, sizeof(kState.code));
    }

}


void formatCommandData(CommandAction act,  uint8_t *buf , int len){
    clearBuff(buf, len);
    buf[0] = act ; // Command Action
}

CommandAction parseCommandData(const uint8_t *buf, int len){
    if (buf[0] ==  CommandAction::STOP_ACTION) {
        return CommandAction::STOP_ACTION;
    }
    return CommandAction::UNKNOWN_ACTION;
}