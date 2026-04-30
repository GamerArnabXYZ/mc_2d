#include "InputManager.h"
#include <cmath>

// Fixed joystick position: bottom-left corner
#define JOY_X  (90.0f)
#define JOY_Y  (WINDOW_H - 110.0f)

InputManager::InputManager()
    : m_moveX(0), m_jump(false), m_breaking(false), m_placed(false)
    , m_targetSX(WINDOW_W/2), m_targetSY(WINDOW_H/2)
    , m_openInv(false), m_slotScroll(0), m_hotbarDirect(-1)
    , m_quit(false), m_startPressed(false)
    , m_joyFingerID(-1)
    , m_joyBaseX(JOY_X), m_joyBaseY(JOY_Y)
    , m_joyStickX(JOY_X), m_joyStickY(JOY_Y)
    , m_actionFingerID(-1)
    , m_actionStartX(0), m_actionStartY(0), m_actionDownTime(0), m_actionMoved(false)
    , m_lastJumpTime(0)
    , m_invTap(false), m_invTapX(0), m_invTapY(0)
    , m_keyLeft(false), m_keyRight(false)
    , m_mouseBreak(false), m_mouseX(WINDOW_W/2), m_mouseY(WINDOW_H/2)
    , m_screenW(WINDOW_W), m_screenH(WINDOW_H)
{}

bool InputManager::processEvents(GameState state) {
    // Reset per-frame flags
    m_jump = false; m_placed = false;
    m_openInv = false; m_slotScroll = 0;
    m_hotbarDirect = -1; m_startPressed = false;
    m_invTap = false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: m_quit = true; return false;
            case SDL_KEYDOWN: handleKeyDown((SDL_Keycode)e.key.keysym.sym); break;
            case SDL_KEYUP:   handleKeyUp  ((SDL_Keycode)e.key.keysym.sym); break;
            case SDL_MOUSEBUTTONDOWN: handleMouse(e.button, true);  break;
            case SDL_MOUSEBUTTONUP:   handleMouse(e.button, false); break;
            case SDL_MOUSEMOTION:
                m_mouseX = e.motion.x; m_mouseY = e.motion.y;
                m_targetSX = m_mouseX; m_targetSY = m_mouseY;
                break;
            case SDL_MOUSEWHEEL: handleScroll(e.wheel.y); break;
            case SDL_FINGERDOWN:
                handleTouch(e.tfinger, true,  false, state); break;
            case SDL_FINGERUP:
                handleTouch(e.tfinger, false, false, state); break;
            case SDL_FINGERMOTION:
                handleTouch(e.tfinger, false, true,  state); break;
            default: break;
        }
    }
    updateFromKeyboard();
    return true;
}

void InputManager::handleKeyDown(SDL_Keycode key) {
    switch (key) {
        case SDLK_a: case SDLK_LEFT:  m_keyLeft  = true; break;
        case SDLK_d: case SDLK_RIGHT: m_keyRight = true; break;
        case SDLK_w: case SDLK_UP: case SDLK_SPACE:
            m_jump = true; break;
        case SDLK_e: m_openInv = true; break;
        case SDLK_RETURN: case SDLK_KP_ENTER: m_startPressed = true; break;
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
    // Only override moveX if no touch joystick active
    if (m_joyFingerID == -1) {
        if (m_keyLeft && !m_keyRight)      m_moveX = -MOVE_SPEED;
        else if (m_keyRight && !m_keyLeft) m_moveX =  MOVE_SPEED;
        else                               m_moveX =  0.0f;
        // Desktop: mouse controls break target
        m_breaking = m_mouseBreak;
        m_targetSX = m_mouseX;
        m_targetSY = m_mouseY;
    }
}

void InputManager::handleScroll(int delta) { m_slotScroll = -delta; }

void InputManager::handleMouse(const SDL_MouseButtonEvent& e, bool down) {
    m_mouseX = e.x; m_mouseY = e.y;
    if (e.button == SDL_BUTTON_LEFT)          m_mouseBreak = down;
    if (e.button == SDL_BUTTON_RIGHT && down) m_placed = true;
    // Home screen: any click = start
    if (down) m_startPressed = true;
}

// ─── Touch handling ────────────────────────────────────────────────────────────
void InputManager::handleTouch(const SDL_TouchFingerEvent& e,
                                bool down, bool motion, GameState state) {
    // SDL finger coords 0..1 — scale to actual screen pixels
    float fx = e.x * (float)m_screenW;
    float fy = e.y * (float)m_screenH;

    // ── HOME screen: any tap = start ─────────────────────────────────────────
    if (state == GameState::HOME) {
        if (down) m_startPressed = true;
        return;
    }

    // ── PLAYING state ─────────────────────────────────────────────────────────

    // Inventory open: route ALL touches to inv tap system
    if (m_openInv) {
        // We track taps for inventory interaction
        if (!down && !motion) {
            // finger up = tap registered
            m_invTap  = true;
            m_invTapX = (int)fx;
            m_invTapY = (int)fy;
        }
        return;
    }

    // ── INV button: top-right 80x80 area ─────────────────────────────────────
    if (down && fx > m_screenW - 88 && fy < 88) {
        m_openInv = true;
        return;
    }

    // ── LEFT ZONE: joystick (fixed position, left 45% of screen) ─────────────
    // Joystick activates if touch is within 2× joystick radius of fixed center
    float jdx = fx - m_joyBaseX;
    float jdy = fy - m_joyBaseY;
    float jDistSq = jdx*jdx + jdy*jdy;
    float activateR = TOUCH_JOYSTICK_R * 2.2f;

    bool inJoyZone = (fx < m_screenW * 0.45f) ||
                     (jDistSq < activateR * activateR && m_joyFingerID == -1);

    if (down && inJoyZone && m_joyFingerID == -1) {
        m_joyFingerID = e.fingerId;
        // Stick starts at touch point but base stays FIXED
        m_joyStickX = fx;
        m_joyStickY = fy;
        m_moveX = 0;
        return;
    }

    if (e.fingerId == m_joyFingerID) {
        if (motion) {
            m_joyStickX = fx;
            m_joyStickY = fy;

            float dx = fx - m_joyBaseX;
            float dy = fy - m_joyBaseY;
            float dist = sqrtf(dx*dx + dy*dy);

            // Clamp stick to radius
            if (dist > TOUCH_JOYSTICK_R) {
                dx = dx / dist * TOUCH_JOYSTICK_R;
                dy = dy / dist * TOUCH_JOYSTICK_R;
                m_joyStickX = m_joyBaseX + dx;
                m_joyStickY = m_joyBaseY + dy;
            }

            // Horizontal movement with dead zone
            float nx = dx / TOUCH_JOYSTICK_R;
            if (fabsf(nx) < 0.18f) nx = 0;
            m_moveX = nx * MOVE_SPEED;

            // Jump: swipe UP significantly AND cooldown passed
            Uint32 now = SDL_GetTicks();
            bool cooldownOk = (now - m_lastJumpTime) > JUMP_COOLDOWN_MS;
            if (dy < -TOUCH_JOYSTICK_R * 0.55f && cooldownOk) {
                m_jump = true;
                m_lastJumpTime = now;
            }
        }
        if (!down) {
            // Finger lifted — reset joystick
            m_joyFingerID = -1;
            m_joyStickX   = m_joyBaseX;
            m_joyStickY   = m_joyBaseY;
            m_moveX       = 0;
        }
        return;
    }

    // ── RIGHT ZONE: break/place ───────────────────────────────────────────────
    if (down && m_actionFingerID == -1) {
        m_actionFingerID = e.fingerId;
        m_actionStartX   = fx;
        m_actionStartY   = fy;
        m_actionDownTime = SDL_GetTicks();
        m_actionMoved    = false;
        m_targetSX = (int)fx;
        m_targetSY = (int)fy;
        m_breaking = true;
        return;
    }

    if (e.fingerId == m_actionFingerID) {
        if (motion) {
            m_targetSX = (int)fx;
            m_targetSY = (int)fy;
            float mdx = fx - m_actionStartX;
            float mdy = fy - m_actionStartY;
            if (sqrtf(mdx*mdx + mdy*mdy) > 12.0f) m_actionMoved = true;
        }
        if (!down) {
            m_actionFingerID = -1;
            m_breaking       = false;
            // Short stationary tap = place block
            Uint32 held = SDL_GetTicks() - m_actionDownTime;
            if (held < 300 && !m_actionMoved) {
                m_placed   = true;
                m_targetSX = (int)m_actionStartX;
                m_targetSY = (int)m_actionStartY;
            }
        }
        return;
    }
}
