# Vertical two-colour gradient as an ASCII PPM (P3), ordered-dithered (8x8 Bayer) so every
# pixel is an exact RGB565 colour. ESPHome's RGB565 image conversion only truncates, which
# bands visibly on a smooth gradient; pre-dithering hides that.
#
# Usage (from the example folder):
#   awk -v W=8 -v H=460 -v top=1B0A3C -v bot=003B5C -f tools/dithered_gradient.awk > images/osptek_bg_gradient.ppm
#
# W should be a multiple of 8 so the strip tiles seamlessly across the page.

function hexbyte(s,   d) {
    d = "0123456789abcdef"
    s = tolower(s)
    return (index(d, substr(s, 1, 1)) - 1) * 16 + index(d, substr(s, 2, 1)) - 1
}

# Quantise 8-bit channel value c to `bits` bits at pixel (x, y), returned as an 8-bit value
# (low bits zero, so ESPHome's truncation keeps it exactly).
function quant(c, bits, x, y,   mx, level, i, frac) {
    mx = 2 ^ bits - 1
    level = c * mx / 255
    i = int(level)
    frac = level - i
    if (frac > (63 - bayer[y % 8, x % 8] + 0.5) / 64) i++
    if (i > mx) i = mx
    return i * 2 ^ (8 - bits)
}

BEGIN {
    if (W == "" || H == "" || length(top) != 6 || length(bot) != 6) {
        print "usage: awk -v W=8 -v H=460 -v top=RRGGBB -v bot=RRGGBB -f dithered_gradient.awk" > "/dev/stderr"
        exit 1
    }
    n = split("0 32 8 40 2 34 10 42 48 16 56 24 50 18 58 26 12 44 4 36 14 46 6 38 60 28 52 20 62 30 54 22 " \
              "3 35 11 43 1 33 9 41 51 19 59 27 49 17 57 25 15 47 7 39 13 45 5 37 63 31 55 23 61 29 53 21", m, " ")
    for (k = 0; k < n; k++) bayer[int(k / 8), k % 8] = m[k + 1]

    r0 = hexbyte(substr(top, 1, 2)); g0 = hexbyte(substr(top, 3, 2)); b0 = hexbyte(substr(top, 5, 2))
    r1 = hexbyte(substr(bot, 1, 2)); g1 = hexbyte(substr(bot, 3, 2)); b1 = hexbyte(substr(bot, 5, 2))

    print "P3"
    print "# osptek-esp32-p4-amoled background: #" top " to #" bot ", Bayer-dithered to RGB565"
    print W, H
    print 255
    for (y = 0; y < H; y++) {
        t = (H > 1) ? y / (H - 1) : 0
        r = r0 + (r1 - r0) * t; g = g0 + (g1 - g0) * t; b = b0 + (b1 - b0) * t
        line = ""
        for (x = 0; x < W; x++)
            line = line (x ? " " : "") quant(r, 5, x, y) " " quant(g, 6, x, y) " " quant(b, 5, x, y)
        print line
    }
}
