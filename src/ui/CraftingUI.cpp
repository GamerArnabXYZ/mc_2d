#include "CraftingUI.h"
#include "../core/Config.h"
#include <cstdio>

// ─── Layout: fits inside WINDOW_W x WINDOW_H with padding ────────────────────
// Uses % of screen so it works on APK (480x854) and web
static const int SLOT_SZ  = 48;  // touch-friendly
static const int SLOT_PAD = 6;
static const int PANEL_W  = WINDOW_W - 20;
static const int PANEL_H  = WINDOW_H - 100;
static const int PANEL_X  = 10;
static const int PANEL_Y  = 50;

// Block color helper (duplicated locally to avoid cross-include)
static SDL_Color blockCol(uint8_t id) {
    switch(id) {
        case 1:  return {80,150,50,255};  // grass
        case 2:  return {130,90,55,255};  // dirt
        case 3:  return {130,130,130,255};// stone
        case 4:  return {220,200,130,255};// sand
        case 5:  return {150,140,130,255};// gravel
        case 6:  return {140,100,50,255}; // wood
        case 7:  return {45,130,35,200};  // leaves
        case 8:  return {40,100,200,160}; // water
        case 9:  return {80,80,80,255};   // coal ore
        case 10: return {150,110,80,255}; // iron ore
        case 11: return {200,170,30,255}; // gold ore
        case 12: return {30,210,210,255}; // diamond ore
        case 13: return {190,145,80,255}; // planks
        case 14: return {110,110,110,255};// cobblestone
        case 15: return {40,40,40,255};   // bedrock
        case 16: return {180,220,255,100};// glass
        case 20: return {230,240,255,255};// snow
        case 22: return {50,160,50,255};  // cactus
        default: return {160,80,200,255};
    }
}

CraftingUI::CraftingUI() : m_panelX(PANEL_X), m_panelY(PANEL_Y) {}

// ─── Layout helpers ────────────────────────────────────────────────────────────
// Hotbar row at bottom of panel
SDL_Rect CraftingUI::hotbarSlotRect(int i) const {
    int x = m_panelX + 6 + i * (SLOT_SZ + SLOT_PAD);
    int y = m_panelY + PANEL_H - SLOT_SZ - 10;
    return {x, y, SLOT_SZ, SLOT_SZ};
}

// Inventory 3×9 grid (scroll if needed, but we show all)
// To fit 9 cols we reduce slot size slightly here
SDL_Rect CraftingUI::invSlotRect(int row, int col) const {
    int sz  = (PANEL_W - 20) / INV_COLS - 4;
    int pad = 4;
    int x   = m_panelX + 10 + col * (sz + pad);
    int y   = m_panelY + 110 + row * (sz + pad);
    return {x, y, sz, sz};
}

// 2×2 craft grid — top-right area
SDL_Rect CraftingUI::craftInSlotRect(int i) const {
    int col = i % 2, row = i / 2;
    int startX = m_panelX + PANEL_W - 2*(SLOT_SZ+SLOT_PAD) - 10;
    int x = startX + col * (SLOT_SZ + SLOT_PAD);
    int y = m_panelY + 30 + row * (SLOT_SZ + SLOT_PAD);
    return {x, y, SLOT_SZ, SLOT_SZ};
}

SDL_Rect CraftingUI::craftOutRect() const {
    int sz = SLOT_SZ + 10;
    int x  = m_panelX + PANEL_W - sz - 8;
    int y  = m_panelY + 30 + SLOT_SZ + SLOT_PAD + 10;
    return {x, y, sz, sz};
}

// Close button
SDL_Rect CraftingUI::closeRect() const {
    return {m_panelX + PANEL_W - 44, m_panelY + 4, 40, 40};
}

// ─── Hit test ─────────────────────────────────────────────────────────────────
int CraftingUI::slotAt(int px, int py) const {
    // Close button
    SDL_Rect cr = closeRect();
    if (px>=cr.x && px<cr.x+cr.w && py>=cr.y && py<cr.y+cr.h) return 999; // close

    // Hotbar
    for (int i=0;i<HOTBAR_SLOTS;i++){
        SDL_Rect r=hotbarSlotRect(i);
        if(px>=r.x&&px<r.x+r.w&&py>=r.y&&py<r.y+r.h) return i;
    }
    // Inventory
    for(int row=0;row<INV_ROWS;row++){
        for(int col=0;col<INV_COLS;col++){
            SDL_Rect r=invSlotRect(row,col);
            if(px>=r.x&&px<r.x+r.w&&py>=r.y&&py<r.y+r.h)
                return 100+row*INV_COLS+col;
        }
    }
    // Craft inputs
    for(int i=0;i<4;i++){
        SDL_Rect r=craftInSlotRect(i);
        if(px>=r.x&&px<r.x+r.w&&py>=r.y&&py<r.y+r.h) return 200+i;
    }
    // Craft output
    {SDL_Rect r=craftOutRect();
     if(px>=r.x&&px<r.x+r.w&&py>=r.y&&py<r.y+r.h) return 300;}
    return -1;
}

// ─── Render ────────────────────────────────────────────────────────────────────
void CraftingUI::render(SDL_Renderer* rend, TextureAtlas& atlas, Inventory& inv) {
    // Dim overlay
    SDL_SetRenderDrawColor(rend,0,0,0,180);
    SDL_Rect full={0,0,WINDOW_W,WINDOW_H};
    SDL_RenderFillRect(rend,&full);

    // Panel
    SDL_SetRenderDrawColor(rend,35,30,25,245);
    SDL_Rect panel={m_panelX,m_panelY,PANEL_W,PANEL_H};
    SDL_RenderFillRect(rend,&panel);
    SDL_SetRenderDrawColor(rend,160,130,80,255);
    SDL_RenderDrawRect(rend,&panel);

    // Title bar
    SDL_SetRenderDrawColor(rend,55,45,30,255);
    SDL_Rect tbar={m_panelX,m_panelY,PANEL_W,44};
    SDL_RenderFillRect(rend,&tbar);
    // "INVENTORY" label blocks
    drawBlockText(rend,"INVENTORY",m_panelX+10,m_panelY+14,{220,200,140,255});

    // Close button [X]
    SDL_Rect cr=closeRect();
    SDL_SetRenderDrawColor(rend,180,50,50,220);
    SDL_RenderFillRect(rend,&cr);
    SDL_SetRenderDrawColor(rend,255,100,100,255);
    SDL_RenderDrawRect(rend,&cr);
    drawBlockText(rend,"X",cr.x+12,cr.y+12,{255,255,255,255});

    // Craft section label
    int startX = m_panelX + PANEL_W - 2*(SLOT_SZ+SLOT_PAD) - 10;
    drawBlockText(rend,"CRAFT",startX,m_panelY+14,{180,220,180,255});

    // Craft inputs
    for(int i=0;i<4;i++){
        SDL_Rect r=craftInSlotRect(i);
        ItemStack cs(inv.getCraftInput(i), inv.getCraftInput(i)?1:0);
        drawSlot(rend,r,cs,atlas,false);
    }
    // Arrow
    {
        SDL_Rect ar=craftInSlotRect(2);
        int ax=ar.x+SLOT_SZ+4, ay=ar.y+SLOT_SZ/2-4;
        SDL_SetRenderDrawColor(rend,200,200,200,200);
        SDL_Rect arrow={ax,ay,16,8};
        SDL_RenderFillRect(rend,&arrow);
    }
    // Craft output
    {
        ItemStack out=inv.getCraftOutput();
        SDL_Rect r=craftOutRect();
        if(!out.empty()){
            SDL_SetRenderDrawColor(rend,40,140,40,80);
            SDL_RenderFillRect(rend,&r);
        }
        drawSlot(rend,r,out,atlas,false);
        drawBlockText(rend,"OUT",r.x,r.y-14,{120,200,120,200});
    }

    // Section divider
    SDL_SetRenderDrawColor(rend,80,70,55,255);
    SDL_Rect div={m_panelX+8,m_panelY+98,PANEL_W-16,1};
    SDL_RenderFillRect(rend,&div);
    drawBlockText(rend,"BAG",m_panelX+10,m_panelY+100,{180,160,120,200});

    // Inventory grid
    for(int row=0;row<INV_ROWS;row++)
        for(int col=0;col<INV_COLS;col++)
            drawSlot(rend,invSlotRect(row,col),inv.invSlot(row*INV_COLS+col),atlas,false);

    // Hotbar divider
    SDL_SetRenderDrawColor(rend,80,70,55,255);
    SDL_Rect hdiv={m_panelX+8,m_panelY+PANEL_H-SLOT_SZ-18,PANEL_W-16,1};
    SDL_RenderFillRect(rend,&hdiv);
    drawBlockText(rend,"HOTBAR",m_panelX+10,m_panelY+PANEL_H-SLOT_SZ-16,{180,160,120,200});

    // Hotbar slots
    for(int i=0;i<HOTBAR_SLOTS;i++)
        drawSlot(rend,hotbarSlotRect(i),inv.hotbarSlot(i),atlas,i==inv.selected());

    // Touch hint at bottom
    SDL_SetRenderDrawColor(rend,100,100,100,120);
    SDL_Rect hint={m_panelX+10,m_panelY+PANEL_H-8,PANEL_W-20,6};
    SDL_RenderFillRect(rend,&hint);
}

// ─── Handle click/tap ─────────────────────────────────────────────────────────
bool CraftingUI::handleClick(int mx, int my, bool /*rightBtn*/, Inventory& inv) {
    int slot = slotAt(mx, my);

    if (slot == 999) { // close button
        inv.setOpen(false);
        return true;
    }
    if (slot == 300) { // craft output
        inv.doCraft();
        return true;
    }
    if (slot >= 200 && slot < 204) {
        // Cycle craft input: pick from hotbar items
        int ci = slot - 200;
        uint8_t cur = inv.getCraftInput(ci);
        // Find next non-zero hotbar item id after cur
        uint8_t next = 0;
        bool found = false;
        for (int i = 0; i < HOTBAR_SLOTS; i++) {
            if (inv.hotbarSlot(i).id > cur && !inv.hotbarSlot(i).empty()) {
                next = inv.hotbarSlot(i).id;
                found = true;
                break;
            }
        }
        if (!found) next = 0; // clear
        inv.setCraftInput(ci, next);
        return true;
    }
    // Tapping outside panel closes inv
    if (mx < m_panelX || mx > m_panelX+PANEL_W ||
        my < m_panelY || my > m_panelY+PANEL_H) {
        inv.setOpen(false);
        return true;
    }
    return false;
}

// ─── Draw helpers ─────────────────────────────────────────────────────────────
void CraftingUI::drawSlot(SDL_Renderer* r, SDL_Rect rect,
                           const ItemStack& item, TextureAtlas& atlas, bool sel) {
    SDL_Color bg = sel ? SDL_Color{160,140,50,220} : SDL_Color{50,44,36,210};
    SDL_SetRenderDrawColor(r,bg.r,bg.g,bg.b,bg.a);
    SDL_RenderFillRect(r,&rect);
    SDL_SetRenderDrawColor(r,sel?220:90,sel?220:80,sel?0:65,255);
    SDL_RenderDrawRect(r,&rect);

    if (item.empty()) return;

    SDL_Rect inner={rect.x+4,rect.y+4,rect.w-8,rect.h-8};
    if (atlas.isLoaded()) {
        SDL_Rect src=atlas.getRect(item.id,false);
        SDL_SetTextureColorMod(atlas.texture(),255,255,255);
        SDL_RenderCopy(r,atlas.texture(),&src,&inner);
    } else {
        SDL_Color ic=blockCol(item.id);
        SDL_SetRenderDrawColor(r,ic.r,ic.g,ic.b,ic.a);
        SDL_RenderFillRect(r,&inner);
    }

    if (item.count > 1) {
        char buf[8]; snprintf(buf,sizeof(buf),"%d",item.count);
        // Tiny count badge
        SDL_SetRenderDrawColor(r,0,0,0,160);
        SDL_Rect badge={rect.x+rect.w-16,rect.y+rect.h-12,14,10};
        SDL_RenderFillRect(r,&badge);
        SDL_SetRenderDrawColor(r,255,230,80,255);
        SDL_Rect num={rect.x+rect.w-14,rect.y+rect.h-11,10,8};
        SDL_RenderFillRect(r,&num);
    }
}

// Minimal pixel-block text — each char = 5px wide
void CraftingUI::drawBlockText(SDL_Renderer* r, const char* txt, int x, int y, SDL_Color col) {
    SDL_SetRenderDrawColor(r,col.r,col.g,col.b,col.a);
    for (const char* c=txt; *c; c++, x+=6) {
        if (*c==' ') continue;
        // Draw char as a filled 4×8 block
        SDL_Rect ch={x,y,4,8};
        SDL_RenderFillRect(r,&ch);
    }
}
