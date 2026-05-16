#ifndef DEFINES_H_
#define DEFINES_H_

#define USE_SIMPLE_COLLISIONS

#define FXDATA_STREAMING

#if defined (_WIN32)
//#define STANDARD_FILE_STREAMING
//#define PROGMEM_MAP_STREAMING
//#define EMULATE_GAMEBUINO 1
#define EMULATE_ARDUBOY 1 
//#define EMULATE_HACKVISION 1
//#define EMULATE_UZEBOX 1

#if defined(EMULATE_ARDUBOY)
// Arduboy
#define DISPLAYWIDTH 128
#define DISPLAYHEIGHT 64
#endif

#if defined(EMULATE_UZEBOX)
// Arduboy
#define DISPLAYWIDTH 240
#define DISPLAYHEIGHT 120
#endif

#else // Arduboy
//#define PETIT_FATFS_FILE_STREAMING
//#define PROGMEM_MAP_STREAMING
#define DISPLAYWIDTH 128
#define DISPLAYHEIGHT 64
#endif

#define HALF_DISPLAYWIDTH (DISPLAYWIDTH >> 1)
#define HALF_DISPLAYHEIGHT (DISPLAYHEIGHT >> 1)

// WIN32 specific
#ifdef _WIN32
#define ZOOM_SCALE 2

#define PROGMEM
#define PSTR
#define pgm_read_byte(x) (*((uint8_t*)x))
#define pgm_read_word(x) (*((uint16_t*)x))

#define pgm_read_ptr(x) (*((uintptr_t*)x))
#define strlen_P(x) strlen(x)

#include <stdint.h>
typedef uint32_t __uint24;
using uint24_t = __uint24;

#include <stdio.h>
#define WARNING(msg, ...) printf((msg), __VA_ARGS__)
#define ERROR(msg) printf(msg)
#else
#include <avr/pgmspace.h>
#define WARNING(msg, ...)
#define pgm_read_ptr(x) pgm_read_word(x)

#endif
// end

#if !defined(max) && !defined(min)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define min3(a, b, c) (min(min(a, b), c))
#define max3(a, b, c) (max(max(a, b), c))
#define sign(x) ((x) < 0 ? -1 : 1)
#define mabs(x) ((x) < 0 ? -(x) : (x))

#define FIRST_FONT_GLYPH 32
#define LAST_FONT_GLYPH 95
#define FONT_WIDTH 3
#define FONT_HEIGHT 5
#define FONT_GLYPH_BYTE_SIZE 2

#define TARGET_FRAMERATE 30

#define PLAYERS_PER_TEAM 5
#define NUM_PEOPLE (PLAYERS_PER_TEAM * 2 + 1)

#define TILE_SIZE 8
#define TILE_SIZE_BYTES 8

#define FIXED_SHIFT 4
#define BALL_GRAVITY 12
#define BALL_FRICTION 3
#define KICK_RECOVERY_FRAMES 6

#endif
