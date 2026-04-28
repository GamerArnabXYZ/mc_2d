#pragma once
#include <SDL2/SDL.h>
#include "../core/Config.h"

// ─── InputManager ─────────────────────────────────────────────────────────────
// Abstracts keyboard/mouse (desktop) and touch (mobile/web) into one API.
// Touch controls:
//   Left side joystick  → move left/right + jump (swipe up)
//   Right side tap      → break/place block
//   Top-right corners   → slot scroll, inventory toggle
class InputManager {
public:
    InputManager();

    // Call each frame before game update
    // Returns false if quit was requested
    bool processEvents();

    // ── Movement ──────────────────────────────────────────────────────────────
    float  moveX()    const { return m_moveX; }   // -1..1
    bool   jumpPressed()    { bool v = m_jump; m_jump = false; return v; }

    // ── Block interaction ─────────────────────────────────────────────────────
    bool   isBreaking()     const { return m_breaking; }
    bool   placedThisFrame()      { bool v = m_placed; m_placed = false; return v; }
    int    targetScreenX()  const { return m_targetSX; }
    int    targetScreenY()  const { return m_targetSY; }

    // ── Inventory / UI ────────────────────────────────────────────────────────
    bool   openInventory()        { bool v = m_openInv; m_openInv = false; return v; }
    int    slotScroll()           { int v = m_slotScroll; m_slotScroll = 0; return v; }
    int    hotbarDirect()         { int v = m_hotbarDirect; m_hotbarDirect = -1; return v; }

    bool   quitRequested()  const { return m_quit; }

    // ── Touch virtual joystick state (for rendering) ──────────────────────────
    float  joyBaseX()       const { return m_joyBaseX; }
    float  joyBaseY()       const { return m_joyBaseY; }
    float  joyStickX()      const { return m_joyStickX; }
    float  joyStickY()      const { return m_joyStickY; }
    bool   joyActive()      const { return m_joyFingerID != -1; }

private:
    // State
    float   m_moveX;
    bool    m_jump;
    bool    m_breaking;
    bool    m_placed;
    int     m_targetSX, m_targetSY;
    bool    m_openInv;
    int     m_slotScroll;
    int     m_hotbarDirect;
    bool    m_quit;

    // Touch state
    SDL_FingerID m_joyFingerID;   // -1 = not active
    float        m_joyBaseX, m_joyBaseY;
    float        m_joyStickX, m_joyStickY;

    SDL_FingerID m_actionFingerID;
    float        m_actionStartX, m_actionStartY;
    Uint32       m_actionDownTime;

    // Desktop state
    bool    m_keyLeft, m_keyRight, m_keyJump;
    bool    m_mouseBreak;
    int     m_mouseX, m_mouseY;

    // Helpers
    void handleKeyDown(SDL_Keycode key);
    void handleKeyUp  (SDL_Keycode key);
    void handleTouch  (const SDL_TouchFingerEvent& e, bool down, bool motion);
    void handleMouse  (const SDL_MouseButtonEvent& e, bool down);
    void handleMouseMotion(const SDL_MouseMotionEvent& e);
    void handleScroll (int delta);
    void updateFromKeyboard();
};
