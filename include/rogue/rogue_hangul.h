#ifndef ROGUE_HANGUL_H
#define ROGUE_HANGUL_H

#include "global.h"

#define HANGUL_CHAR_OFFSET   0x50
#define HANGUL_GLYPH_SIZE    32

struct HangulEntry
{
    u16 unicode;
    u8  tileIndex;
    u8  _pad;
};

extern const struct HangulEntry gHangulTable[];
extern const u8                 gHangulGlyphs[];
extern const u16                gHangulTableSize;

void Hangul_Init(void);
void Hangul_LoadFont(void);
u8   Hangul_LookupTile(u16 unicode);
u8   Hangul_ConvertUtf8(const u8 *src, u8 *dst, u8 maxLen);

#endif
