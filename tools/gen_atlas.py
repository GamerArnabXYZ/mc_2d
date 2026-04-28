#!/usr/bin/env python3
"""
gen_atlas.py — Generates a placeholder 256×256 texture atlas (16×16 grid of 16×16 cells).
Each cell is a solid color representing a block type.
Usage: python3 tools/gen_atlas.py [output_path]
Requires: Pillow (pip install Pillow)
"""
import sys
import os

try:
    from PIL import Image, ImageDraw
except ImportError:
    print("Pillow not found. Install: pip install Pillow")
    sys.exit(1)

ATLAS_COLS = 16
CELL = 16
SIZE = ATLAS_COLS * CELL  # 256×256

# Block colors [R,G,B] indexed by BlockID
# Matches Block.h order: AIR, GRASS, DIRT, STONE, SAND, GRAVEL, WOOD, LEAVES,
#                        WATER, COAL, IRON, GOLD, DIAMOND, PLANKS, COBBLE, BEDROCK,
#                        GLASS, TORCH, CRAFTING, CHEST, SNOW, ICE, CACTUS
BLOCK_COLORS = [
    (0,   0,   0,   0),    # 0  AIR       (transparent)
    (58,  122, 28,  255),  # 1  GRASS
    (122, 82,  48,  255),  # 2  DIRT
    (138, 138, 138, 255),  # 3  STONE
    (232, 210, 138, 255),  # 4  SAND
    (154, 144, 144, 255),  # 5  GRAVEL
    (139, 105, 20,  255),  # 6  WOOD (side)
    (42,  138, 28,  128),  # 7  LEAVES (semi-transparent)
    (34,  85,  170, 180),  # 8  WATER
    (70,  70,  70,  255),  # 9  COAL ORE
    (140, 110, 90,  255),  # 10 IRON ORE
    (232, 200, 32,  255),  # 11 GOLD ORE
    (0,   212, 212, 255),  # 12 DIAMOND ORE
    (200, 160, 96,  255),  # 13 PLANKS
    (110, 108, 100, 255),  # 14 COBBLESTONE
    (40,  40,  40,  255),  # 15 BEDROCK
    # Row 1
    (58,  122, 28,  255),  # 0,1 GRASS TOP
    (150, 110, 30,  255),  # 1,1 WOOD TOP
    (230, 240, 255, 255),  # 2,1 SNOW
    (180, 220, 255, 200),  # 3,1 ICE
    (50,  160, 50,  255),  # 4,1 CACTUS
    (255, 220, 60,  255),  # 5,1 TORCH
    (180, 120, 50,  255),  # 6,1 CRAFTING TOP
    (200, 160, 90,  255),  # 7,1 CHEST TOP
]

def darken(color, factor=0.7):
    r, g, b, a = color
    return (int(r*factor), int(g*factor), int(b*factor), a)

def draw_ore_dot(draw, x, y, dot_color):
    """Draw ore veins as small dots on stone background"""
    cx, cy = x + CELL//2, y + CELL//2
    draw.ellipse([cx-3, cy-3, cx+3, cy+3], fill=dot_color)
    draw.ellipse([cx-5, cy+1, cx-1, cy+5], fill=dot_color)
    draw.ellipse([cx+1, cy-5, cx+5, cy-1], fill=dot_color)

def make_atlas(out_path):
    img  = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # ── Row 0: main block faces ───────────────────────────────────────────────
    for col in range(ATLAS_COLS):
        x, y = col * CELL, 0
        if col < len(BLOCK_COLORS) and col < ATLAS_COLS:
            c = BLOCK_COLORS[col]
            draw.rectangle([x, y, x+CELL-1, y+CELL-1], fill=c)

            # Add texture details per block
            if col == 1:  # GRASS: green top strip
                draw.rectangle([x, y, x+CELL-1, y+3], fill=(80, 160, 40, 255))
            elif col == 3:  # STONE: subtle grid lines
                draw.line([x, y+7, x+CELL-1, y+7], fill=(100, 100, 100, 255))
                draw.line([x+7, y, x+7, y+CELL-1], fill=(100, 100, 100, 255))
            elif col in [9, 10, 11, 12]:  # Ores: dots on stone bg
                draw.rectangle([x, y, x+CELL-1, y+CELL-1], fill=(138, 138, 138, 255))
                dot_colors = [(40,40,40,255),(180,100,60,255),(240,200,20,255),(0,200,200,255)]
                draw_ore_dot(draw, x, y, dot_colors[col - 9])
            elif col == 7:  # LEAVES: add leaf pattern
                for lx in range(0, CELL, 4):
                    for ly in range(0, CELL, 4):
                        if (lx + ly) % 8 == 0:
                            draw.rectangle([x+lx, y+ly, x+lx+2, y+ly+2],
                                           fill=(30, 120, 20, 200))
            elif col == 8:  # WATER: horizontal lines
                for wy in range(2, CELL, 4):
                    draw.line([x, y+wy, x+CELL-1, y+wy],
                              fill=(60, 120, 200, 180))
            # Border pixel
            draw.rectangle([x, y, x+CELL-1, y+CELL-1], outline=(0,0,0,40))

    # ── Row 1: top faces ──────────────────────────────────────────────────────
    extras = BLOCK_COLORS[ATLAS_COLS:]
    for col in range(min(len(extras), ATLAS_COLS)):
        x, y = col * CELL, CELL
        c = extras[col]
        draw.rectangle([x, y, x+CELL-1, y+CELL-1], fill=c)
        draw.rectangle([x, y, x+CELL-1, y+CELL-1], outline=(0,0,0,40))

    # ── Fill remaining rows with magenta (debug: unused cells) ───────────────
    # (kept transparent = fine for the game)

    os.makedirs(os.path.dirname(out_path) if os.path.dirname(out_path) else '.', exist_ok=True)
    img.save(out_path, "PNG")
    print(f"Atlas saved: {out_path} ({SIZE}x{SIZE})")

if __name__ == "__main__":
    out = sys.argv[1] if len(sys.argv) > 1 else "assets/textures/atlas.png"
    make_atlas(out)
