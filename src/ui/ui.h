#ifndef MC2D_UI_H
#define MC2D_UI_H

#include "../mc2d_types.h"
#include "raylib.h"

/* MD3-style UI state */
typedef struct {
    bool  show_pause;
    bool  show_inventory;
    float fade_alpha;
    int   menu_selection;
} UICtx;

UICtx* ui_create(void);
void   ui_destroy(UICtx* u);

/* Returns GAMESTATE_PLAYING if resume, GAMESTATE_MENU if exit */
GameStateEnum ui_draw_pause   (UICtx* u, int sw, int sh);
void          ui_draw_main_menu(UICtx* u, int sw, int sh, GameStateEnum* out_state);
void          ui_draw_gameover (UICtx* u, int sw, int sh, float respawn_t, GameStateEnum* out_state);

/* MD3 button helper — returns true if clicked */
bool ui_button(const char* label, float x, float y, float w, float h,
               Color bg, Color text_col);

#endif /* MC2D_UI_H */
