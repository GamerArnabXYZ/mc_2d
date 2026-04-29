#include "InputManager.h"
#include <cmath>

InputManager::InputManager()
    : m_moveX(0), m_jump(false)
    , m_breaking(false), m_placed(false)
    , m_targetSX(WINDOW_W/2), m_targetSY(WINDOW_H/2)
    , m_openInv(false), m_slotScroll(0), m_hotbarDirect(-1)
    , m_quit(false)
    , m_joyFingerID(-1)
    , m_joyBaseX(TOUCH_JOYSTICK_R + 20), m_joyBaseY(WINDOW_H - TOUCH_JOYSTICK_R - 20)
    , m_joyStickX(TOUCH_JOYSTICK_R + 20), m_joyStickY(WINDOW_H - TOUCH_JOYSTICK_R - 20)
    , m_actionFingerID(-1), m_actionStartX(0), m_actionStartY(0), m_actionDownTime(0)
    , m_keyLeft(false), m_keyRight(false)
    , m_mouseBreak(false), m_mouseX(WINDOW_W/2), m_mouseY(WINDOW_H/2)
{}

bool InputManager::processEvents() {
    m_jump = false;
    m_placed = false;
    m_openInv = false;
    m_slotScroll = 0;
    m_hotbarDirect = -1;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: m_quit = true; return false;
            case SDL_KEYDOWN: handleKeyDown((SDL_Keycode)e.key.keysym.sym); break;
            case SDL_KEYUP:   handleKeyUp  ((SDL_Keycode)e.key.keysym.sym); break;
            case SDL_MOUSEBUTTONDOWN: handleMouse(e.button, true);  break;
            case SDL_MOUSEBUTTONUP:   handleMouse(e.button, false); break;
            case SDL_MOUSEMOTION:     m_mouseX = e.motion.x; m_mouseY = e.motion.y; break;
            case SDL_MOUSEWHEEL:      handleScroll(e.wheel.y); break;
            case SDL_FINGERDOWN:   handleTouch(e.tfinger, true,  false); break;
            case SDL_FINGERUP:     handleTouch(e.tfinger, false, false); break;
            case SDL_FINGERMOTION: handleTouch(e.tfinger, false, true);  break;
            default: break;
        }
    }
    updateFromKeyboard();
    return true;
}

void InputManager::handleKeyDown(SDL_Keycode key) {
    switch (key) {
        case SDLK_a: case SDLK_LEFT:  m_keyLeft  = true;  break;
        case SDLK_d: case SDLK_RIGHT: m_keyRight = true;  break;
        case SDLK_w: case SDLK_UP: case SDLK_SPACE: m_jump = true; break;
        case SDLK_e: m_openInv = true; break;
        case SDLK_ESCAPE: m_quit = true; break;
        case SDLK_1: m_hotbarDirect=0; break; case SDLK_2: m_hotbarDirect=1; break;
        case SDLK_3: m_hotbarDirect=2; break; case SDLK_4: m_hotbarDirect=3; break;
        case SDLK_5: m_hotbarDirect=4; break; case SDLK_6: m_hotbarDirect=5; break;
        case SDLK_7: m_hotbarDirect=6; break; case SDLK_8: m_hotbarDirect=7; break;
        case SDLK_9: m_hotbarDirect=8; break;
        default: break;
    }
}

void InputManager::handleKeyUp(SDL_Keycode key) {
    switch (key) {
        case SDLK_a: case SDLK_LEFT:  m_keyLeft  = false; break;
        case SDLK_d: case SDLK_RIGHT: m_keyRight = false; break;
        default: break;
    }
}

void InputManager::updateFromKeyboard() {
    if (m_keyLeft && !m_keyRight)      m_moveX = -MOVE_SPEED;
    else if (m_keyRight && !m_keyLeft) m_moveX =  MOVE_SPEED;
    else if (!m_joyFingerID == -1)     {} // touch controls moveX already set
    else if (m_joyFingerID == -1)      m_moveX = 0.0f; // no touch, no keys

    m_breaking = m_mouseBreak;
    m_targetSX = m_mouseX;
    m_targetSY = m_mouseY;
}

void InputManager::handleScroll(int delta) { m_slotScroll = -delta; }

void InputManager::handleMouse(const SDL_MouseButtonEvent& e, bool down) {
    m_mouseX = e.x; m_mouseY = e.y;
    if (e.button == SDL_BUTTON_LEFT)             m_mouseBreak = down;
    if (e.button == SDL_BUTTON_RIGHT && down)    m_placed = true;
}

// ─── Touch layout ─────────────────────────────────────────────────────────────
// LEFT 40% of screen → joystick (move + jump on swipe up)
// RIGHT 60% of screen → break (hold) / place (tap)
// TOP-RIGHT 70px × 70px → inventory button
// BOTTOM-RIGHT row → hotbar shortcuts (optional future)
void InputManager::handleTouch(const SDL_TouchFingerEvent& e, bool down, bool motion) {
    float fx = e.x * WINDOW_W;
    float fy = e.y * WINDOW_H;

    // ── Inventory button: top-right corner ────────────────────────────────────
    if (down && fx > WINDOW_W - 80 && fy < 80) {
        m_openInv = true;
        return;
    }

    // ── LEFT side: joystick ───────────────────────────────────────────────────
    bool isLeftSide = (fx < WINDOW_W * 0.42f);

    if (down && isLeftSide && m_joyFingerID == -1) {
        m_joyFingerID = e.fingerId;
        m_joyBaseX  = fx;
        m_joyBaseY  = fy;
        m_joyStickX = fx;
        m_joyStickY = fy;
        m_moveX = 0;
        return;
    }

    if (motion && e.fingerId == m_joyFingerID) {
        m_joyStickX = fx;
        m_joyStickY = fy;
        float dx = fx - m_joyBaseX;
        float dy = fy - m_joyBaseY;
        float dist = sqrtf(dx*dx + dy*dy);
        float clamped = (dist > TOUCH_JOYSTICK_R) ? TOUCH_JOYSTICK_R : dist;
        float ratio   = (dist > 0) ? clamped / dist : 0;
        dx *= ratio; dy *= ratio;

        // Horizontal → movement
        float nx = dx / TOUCH_JOYSTICK_R;
        // Dead zone 15%
        if (fabsf(nx) < 0.15f) nx = 0;
        m_moveX = nx * MOVE_SPEED;

        // Swipe up → jump
        if (dy < -TOUCH_JOYSTICK_R * 0.45f) {
            m_jump = true;
        }
        return;
    }

    if (!down && e.fingerId == m_joyFingerID) {
        m_joyFingerID = -1;
        m_moveX = 0;
        return;
    }

    // ── RIGHT side: action (break/place) ─────────────────────────────────────
    bool isRightSide = (fx >= WINDOW_W * 0.42f);

    if (down && isRightSide && m_actionFingerID == -1) {
        m_actionFingerID = e.fingerId;
        m_actionStartX   = fx;
        m_actionStartY   = fy;
        m_actionDownTime = SDL_GetTicks();
        m_targetSX = (int)fx;
        m_targetSY = (int)fy;
        m_breaking = true;
        return;
    }

    if (motion && e.fingerId == m_actionFingerID) {
        m_targetSX = (int)fx;
        m_targetSY = (int)fy;
        return;
    }

    if (!down && e.fingerId == m_actionFingerID) {
        m_actionFingerID = -1;
        m_breaking = false;
        // Short tap (< 250ms) = place block
        Uint32 held = SDL_GetTicks() - m_actionDownTime;
        if (held < 250) {
            m_placed   = true;
            m_targetSX = (int)m_actionStartX;
            m_targetSY = (int)m_actionStartY;
        }
        return;
    }
}
