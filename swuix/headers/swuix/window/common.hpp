#pragma once
#include <algorithm>
#include <cmath>
#include <stdint.h>

// Debug
#define CLR_DEBUG 0, 255, 0

// Base
#define CLR_TEXT        30,  32,  25  // #1E2019
#define CLR_BACKGROUND 244, 247, 244  // #F4F7F4
#define CLR_PRIMARY     78,  49, 182  // #4E31B6 Inline text hover (please underline on hover for clarity)
#define CLR_SECONDARY  215, 213, 254  // #D7D5FE
#define CLR_ACCENT     159, 155, 253  // #9F9BFD

// Derived neutrals
#define CLR_BORDER_STRONG 109, 111, 105  // #6D6F69
#define CLR_BORDER        139, 140, 136  // #8B8C88 Dividers / splitters / resizers
#define CLR_BORDER_SUBTLE 201, 203, 198  // #C9CBC6 Window chrome hairlines

#define CLR_SURFACE_0 CLR_BACKGROUND  // App background / viewport
#define CLR_SURFACE_1 237, 240, 237   // #EDF0ED Panels / sidebars / inspector
#define CLR_SURFACE_2 231, 234, 231   // #E7EAE7 Cards / popovers / modals
#define CLR_SURFACE_3 224, 227, 224   // #E0E3E0 Menus / tooltips / context menus

// Primary states
#define CLR_ON_PRIMARY   244, 247, 244  // background as on-primary
#define CLR_PRIMARY_HOV   64,  25, 162  // #4019A2
#define CLR_PRIMARY_ACT   55,   1, 149  // #370195
#define CLR_PRIMARY_OUTL 103,  95, 185  // #675FB9
#define CLR_PRIMARY_SURF 190, 190, 239  // #BEBEEF

// Accent as surface + inks
#define CLR_ACCENT_SURF   200, 200, 255  // #C8C8FF
#define CLR_ACCENT_INK    101,  96, 178  // #6560B2 Links (inline text)
#define CLR_ACCENT_STRONG 107, 100, 192  // #6B64C0

// Text roles
#define CLR_TEXT_STRONG   CLR_TEXT       // Body, labels, controls
#define CLR_TEXT_MUTED     81,  83,  77  // #51534D Secondary info (hints, timestamps)
#define CLR_TEXT_SUBTLE   139, 141, 135  // #8B8D87
#define CLR_TEXT_DISABLED 201, 203, 199  // #C9CBC7 Placeholder / disabled text

// Utility alphas
#define ALPHA_SCRIM              128  // ~50%
#define ALPHA_SELECTION_FILL     46   // ~18%
#define ALPHA_SELECTION_FILL_SUB 26   // ~10%
#define ALPHA_HOVER_TINT         15   // ~6%

/**
 * Primary button:
 *   Fill   - CLR_PRIMARY
 *   Hover  - CLR_PRIMARY_HOV
 *   Active - CLR_PRIMARY_ACT
 *   Text   - CLR_ON_PRIMARY
 *   Border - CLR_PRIMARY_OUTL
 *
 * Secondary (neutral) button:
 *   Fill   - CLR_SURFACE_2
 *   Hover  - CLR_SURFACE_2 & ALPHA_HOVER_TINT
 *   Text   - CLR_TEXT_STRONG
 *   Border - CLR_BORDER
 *
 * Toolbar button:
 *   Fill   - *transparent*
 *   Hover  - ALPHA_HOVER_TINT
 *   Text   - CLR_TEXT_STRONG
 *   Selected text - CLR_PRIMARY
 *   Active - CLR_PRIMARY_SURF
 *
 * Inputs:
 *   Background      - CLR_SURFACE_2
 *   Text            - CLR_TEXT_STRONG
 *   Placeholder     - CLR_TEXT_SUBTLE
 *   Border(default) - CLR_BORDER
 *   Border(hover)   - CLR_BORDER_STRONG
 *   Focus ring      - CLR_PRIMARY_OUTL
 *
 * Controls (toggles / switches / checkboxes / radios):
 *   Checked fill     - CLR_PRIMARY
 *   Knob/indicator   - CLR_ON_PRIMARY
 *   Unchecked fill   - CLR_SURFACE_2
 *   Unchecked border - CLR_BORDER
 *   Hover            - +ALPHA_HOVER_TINT
 *   Focus ring       - CLR_PRIMARY_OUTL
 *
 * Tabs & navigation:
 *   Active tab text     - CLR_PRIMARY
 *   Active underline/bg - CLR_PRIMARY_SURF
 *   Inactive text       - CLR_TEXT_MUTED
 *   Hover               - +ALPHA_HOVER_TINT
 *   Focus               - CLR_PRIMARY_OUTL
 *
 * Tables / lists / trees:
 *   Header bg         - CLR_SURFACE_2
 *   Header text       - CLR_TEXT_STRONG
 *   Header border     - CLR_BORDER
 *   Row hover overlay - +ALPHA_HOVER_TINT
 *   Row selected bg   - +ALPHA_SELECTION_FILL + left stripe CLR_PRIMARY
 *   Grid line major   - CLR_BORDER_SUBTLE
 *   Grid line minor   - CLR_BORDER
 *
 * Status & badges:
 *   Info/tag/chip bg     - CLR_ACCENT_SURF
 *   Info/tag/chip text   - CLR_TEXT_STRONG
 *   Info/tag/chip border - BORDER_SUBTLE
 *   Emphasis tag bg      - CLR_PRIMARY_SURF
 *   Emphasis tag text    - CLR_PRIMARY
 *
 * Overlays & feedback:
 *   Modal scrim             - ALPHA_SCRIM
 *   Tooltip bg              - CLR_TEXT_STRONG
 *   Tooltip text            - CLR_BACKGROUND
 *   Tooltip border          - CLR_BORDER_STRONG
 *   Toast/banner bg         - CLR_ACCENT_SURF
 *   Toast/banner text       - CLR_BACKGROUND
 *   Toast/banner accent bar - CLR_PRIMARY
 *
 * Icons
 *   Default         - CLR_TEXT_STRONG
 *   Muted           - CLR_TEXT_SUBTLE
 *   Active/selected - CLR_PRIMARY
 * 
 * Borders & outlines:
 *   Card border      - CLR_BORDER
 *   Emphasis borders - CLR_PRIMARY (selected tool, active panel)
 *   Hairlines        - CLR_BORDER_SUBTLE (grids, rulers)
 *
 * Scrollbar & sliders:
 *   Track        - CLR_SURFACE_2
 *   Thumb        - CLR_BORDER
 *   Thumb hover  - CLR_BORDER_STRONG
 *   Slider track - CLR_BORDER_SUBTLE
 *   Filled slide - CLR_PRIMARY
 *   Slider thumb - CLR_PRIMARY
 *
 * Progress & loaders:
 *   Linear track   - CLR_BORDER_SUBTLE
 *   Bar            - CLR_PRIMARY
 *   Spinner stroke - CLR_PRIMARY
 *   Spinner bg     - CLR_SURFACE_0
 *
 * Cards:
 *   Background - CLR_SURFACE_2
 *   Border     - CLR_BORDER
 *   Title      - CLR_TEXT_STRONG
 *   Meta       - CLR_TEXT_SUBTLE
 *
 * Focus & selection (global):
 *   Keyboard focus ring     - CLR_PRIMARY_OUTL (2px outside)
 *   Text selection (editor) - bg ALPHA_SELECTION_FILL on CLR_TEXT_STRONG
 *
 * Disabled states (controls):
 *   Opacity 60%
 *   Labels  - CLR_TEXT_DISABLED
 *   Borders - CLR_BORDER_SUBTLE
 *
 * Canvas:
 *   Canvas bg                    - CLR_SURFACE_1
 *   Grid line major              - CLR_BORDER
 *   Grid line minor              - CLR_BORDER_SUBTLE
 *   Origin/axes line             - CLR_PRIMARY
 *   Guides/snap line             - CLR_ACCENT_INK
 *   Selection stroke (objects)   - CLR_PRIMARY
 *   Selection fill               - ALPHA_SELECTION_FILL (ALPHA_SELECTION_FILL_SUB for multi-select)
 *   Active handle/vertex         - fill CLR_PRIMARY, stroke CLR_ON_PRIMARY
 *   Inactive handle/vertex       - fill CLR_SURFACE_2, stroke CLR_BORDER
 *   Measure/annotation label     - bg CLR_ACCENT_SURF, text CLR_TEXT_STRONG, border CLR_BORDER_SUBTLE
 *   Bounding box                 - CLR_BORDER_STRONG
 *   Ghost/preview while dragging - stroke CLR_PRIMARY, fill oklab(from CLR_PRIMARY l a b / 0.10)
 */

struct RGBu8 {
    uint8_t r, g, b;
    RGBu8(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) {}
};

#define RGB(rgb) RGBu8(rgb)

struct Lab   { double  L, a, b; };

static inline double  clamp01(double x){ return std::min(1.0, std::max(0.0, x)); }
static inline uint8_t to8    (double x){ return static_cast<uint8_t>(round(clamp01(x) * 255.0)); }

/* sRGB <-> linear */
static inline double srgb_to_linear(double c) {
    c = clamp01(c);
    return (c <= 0.04045) ? (c / 12.92) : std::pow((c + 0.055) / 1.055, 2.4);
}

static inline double linear_to_srgb(double x) {
    x = clamp01(x);
    return (x <= 0.0031308) ? (12.92 * x) : (1.055 * std::pow(x, 1.0/2.4) - 0.055);
}

/* linear RGB <-> OKLab */
static inline Lab rgb_to_oklab(RGBu8 c) {
    double r = srgb_to_linear(c.r / 255.0);
    double g = srgb_to_linear(c.g / 255.0);
    double b = srgb_to_linear(c.b / 255.0);

    double l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b;
    double m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b;
    double s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b;

    double l_ = cbrt(l);
    double m_ = cbrt(m);
    double s_ = cbrt(s);

    Lab lab;
    lab.L = 0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_;
    lab.a = 1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_;
    lab.b = 0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_;
    return lab;
}

static inline RGBu8 oklab_to_rgb(Lab lab) {
    double l_ = lab.L + 0.3963377774 * lab.a + 0.2158037573 * lab.b;
    double m_ = lab.L - 0.1055613458 * lab.a - 0.0638541728 * lab.b;
    double s_ = lab.L - 0.0894841775 * lab.a - 1.2914855480 * lab.b;

    double l = l_ * l_ * l_;
    double m = m_ * m_ * m_;
    double s = s_ * s_ * s_;

    double r_lin =  4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    double g_lin = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    double b_lin = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;

    return RGBu8(
        to8(linear_to_srgb(r_lin)),
        to8(linear_to_srgb(g_lin)),
        to8(linear_to_srgb(b_lin))
    );
}

/* Editing helpers (OKLab space) */

// L is roughly 0..1
static inline RGBu8 OKLabLighten(RGBu8 c, double dL) {
    Lab lab = rgb_to_oklab(c);
    lab.L = clamp01(lab.L + dL);
    return oklab_to_rgb(lab);
}
static inline RGBu8 OKLabDarken(RGBu8 c, double dL){ return OKLabLighten(c, -dL); }
