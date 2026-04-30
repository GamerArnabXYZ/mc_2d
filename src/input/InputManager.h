#pragma once
#include <SDL2/SDL.h>
#include "../core/Config.h"

// ─── GameState ────────────────────────────────────────────────────────────────
enum class GameState { HOME, PLAYING, PAUSED };

class InputManager {
public:
    InputManager();

    bool processEvents(GameState state);

    // Movement
    float moveX()         const { return m_moveX; }
    bool  jumpPressed()         { bool v = m_jump; m_jump = false; return v; }

    // Block interaction
    bool  isBreaking()    const { return m_breaking; }
    bool  placedThisFrame()     { bool v = m_placed; m_placed = false; return v; }
    int   targetScreenX() const { return m_targetSX; }
    int   targetScreenY() const { return m_targetSY; }

    // UI
    bool  openInventory()       { bool v = m_openInv; m_openInv = false; return v; }
    int   slotScroll()          { int  v = m_slotScroll; m_slotScroll = 0; return v; }
    int   hotbarDirect()        { int  v = m_hotbarDirect; m_hotbarDirect = -1; return v; }
    bool  quitRequested() const { return m_quit; }

    // Home screen
    bool  startPressed()        { bool v = m_startPressed; m_startPressed = false; return v; }

    // Touch joystick render state
    float joyBaseX()  const { return m_joyBaseX; }
    float joyBaseY()  const { return m_joyBaseY; }
    float joyStickX() const { return m_joyStickX; }
    float joyStickY() const { return m_joyStickY; }
    bool  joyActive() const { return m_joyFingerID != -1; }

    // Screen size (set once at init for proper coordinate scaling)
    void setScreenSize(int w, int h) { m_screenW = w; m_screenH = h; }

    // Inventory touch (called by Game when inv is open)
    // Returns pixel coords of last finger-up tap, (-1,-1) if none
    bool  invTapThisFrame(int& outX, int& outY) {
        if (m_invTap) { outX = m_invTapX; outY = m_invTapY; m_invTap = false; return true; }
        return false;
    }

private:
    float m_moveX;
    bool  m_jump, m_breaking, m_placed;
    int   m_targetSX, m_targetSY;
    bool  m_openInv;
    int   m_slotScroll, m_hotbarDirect;
    bool  m_quit, m_startPressed;

    // Joystick — FIXED position bottom-left
    SDL_FingerID m_joyFingerID;
    float m_joyBaseX, m_joyBaseY;   // fixed center
    float m_joyStickX, m_joyStickY; // current stick tip

    // Action finger (right side)
    SDL_FingerID m_actionFingerID;
    float  m_actionStartX, m_actionStartY;
    Uint32 m_actionDownTime;
    bool   m_actionMoved; // did finger move > threshold?

    // Jump cooldown — prevents spam
    Uint32 m_lastJumpTime;
    static const Uint32 JUMP_COOLDOWN_MS = 400;

    // Inventory tap
    bool m_invTap;
    int  m_invTapX, m_invTapY;

    // Desktop
    bool m_keyLeft, m_keyRight;
    bool m_mouseBreak;
    int  m_mouseX, m_mouseY;

    // Actual screen dimensions (may differ from WINDOW_W/H on mobile)
    int m_screenW, m_screenH;

    void handleKeyDown(SDL_Keycode key);
    void handleKeyUp  (SDL_Keycode key);
    void handleScroll (int delta);
    void handleTouch  (const SDL_TouchFingerEvent& e, bool down, bool motion,
                       GameState state);
    void handleMouse  (const SDL_MouseButtonEvent& e, bool down);
    void updateFromKeyboard();
};
