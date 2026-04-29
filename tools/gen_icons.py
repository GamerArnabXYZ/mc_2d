#!/usr/bin/env python3
"""
tools/gen_icons.py — Generates Android launcher icons for all densities.
Uses Pillow. Run: python3 tools/gen_icons.py [android_res_dir]
"""
import sys, os
try:
    from PIL import Image, ImageDraw
except ImportError:
    print("ERROR: pip install Pillow")
    sys.exit(1)

RES_DIR = sys.argv[1] if len(sys.argv) > 1 else "android/app/src/main/res"

SIZES = {
    "mipmap-mdpi":    48,
    "mipmap-hdpi":    72,
    "mipmap-xhdpi":   96,
    "mipmap-xxhdpi":  144,
    "mipmap-xxxhdpi": 192,
}

for folder, size in SIZES.items():
    out = os.path.join(RES_DIR, folder, "ic_launcher.png")
    os.makedirs(os.path.dirname(out), exist_ok=True)

    img  = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    m = size // 8
    # Green rounded square
    draw.rounded_rectangle(
        [m, m, size - m, size - m],
        radius=size // 5,
        fill=(76, 153, 0, 255)
    )
    # White cross (pickaxe symbol)
    cx, cy, t = size // 2, size // 2, max(2, size // 16)
    draw.rectangle([cx - t,        cy - size//3,  cx + t,        cy + size//3], fill=(255,255,255,255))
    draw.rectangle([cx - size//3,  cy - t,        cx + size//3,  cy + t      ], fill=(255,255,255,255))

    img.save(out, "PNG")
    print(f"  {folder}/ic_launcher.png  ({size}x{size})")

print("Icons generated OK")
