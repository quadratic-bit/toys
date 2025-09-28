#pragma once
#include <cstddef>

typedef size_t Slot;
typedef size_t CellHandle;
typedef double Time;
typedef int    ParticleID;

#define RGB_BLACK 0
#define RGB_WHITE 255
#define RGB_LIGHT_GRAY 180

#define CLR_MONO(v) v, v, v

#define CLR_BLACK CLR_MONO(RGB_BLACK)
#define CLR_WHITE CLR_MONO(RGB_WHITE)
#define CLR_GRID CLR_MONO(RGB_LIGHT_GRAY)
#define CLR_LIGHT_GRAY CLR_MONO(RGB_LIGHT_GRAY)

#define CLR_BG CLR_BLACK

#define CLR_AMBIENT CLR_MONO(RGB_AMBIENT)
#define CLR_VOID CLR_MONO(RGB_VOID)

// #EAEAEA (234, 234, 234) -- Platinum       (white)
// #D1D1D1 (209, 209, 209) -- Timberwolf     (gray-ish)
// #000F08 (0  , 15 , 8  ) -- Night          (black)
// #348AA7 (52 , 138, 167) -- Blue           (blue)
// #AA4465 (170, 68 , 101) -- Raspberry rose (red)
// #C37D92 (195, 125, 146) -- Puce           (pink)

#define CLR_PLATINUM   CLR_MONO(234)
#define CLR_TIMBERWOLF CLR_MONO(209)
#define CLR_NIGHT      0  , 15 , 8
#define CLR_BLUE       52 , 138, 167
#define CLR_RASPBERRY  170, 68 , 101
#define CLR_PUCE       195, 125, 146

// DEBUG
// #06BA63 (6  , 186, 99 ) -- Emerald

#define CLR_DEBUG 6  , 186, 99
