// ============================================================
//  rogue_hangul.c  -  한글 폰트 스텁
// ============================================================

#include "global.h"
#include "constants/characters.h"
#include "rogue/rogue_hangul.h"
#include <string.h>

// 더미 데이터 (extract_korean_chars.py 실행 전 사용)
static const struct HangulEntry sDummyTable[1] = {{ 0xAC00, 0x50, 0 }};
static const u8 sDummyGlyphs[32] = {0};

const struct HangulEntry gHangulTable[1] = {{ 0xAC00, 0x50, 0 }};
const u8  gHangulGlyphs[32] = {0};
const u16 gHangulTableSize  = 1;

void Hangul_Init(void) {}

void Hangul_LoadFont(void)
{
    u16 i;
    vu8 *vram = (vu8 *)(BG_CHAR_ADDR(3)) + (0x50u * 32u);
    for (i = 0; i < gHangulTableSize; i++)
        memcpy((void *)(vram + i * 32), gHangulGlyphs + i * 32, 32);
}

u8 Hangul_LookupTile(u16 unicode)
{
    s16 lo = 0, hi = (s16)gHangulTableSize - 1, mid;
    while (lo <= hi)
    {
        mid = (lo + hi) / 2;
        if (gHangulTable[mid].unicode == unicode) return gHangulTable[mid].tileIndex;
        else if (gHangulTable[mid].unicode < unicode) lo = mid + 1;
        else hi = mid - 1;
    }
    return CHAR_QUESTION_MARK;
}

u8 Hangul_ConvertUtf8(const u8 *src, u8 *dst, u8 maxLen)
{
    u8 out = 0;
    while (*src && out < maxLen - 1)
    {
        u8 b = *src;
        if (b < 0x80)      { dst[out++] = b; src++; }
        else if ((b & 0xE0) == 0xC0)
        {
            u16 cp = (u16)((b & 0x1F) << 6) | (src[1] & 0x3F);
            dst[out++] = Hangul_LookupTile(cp);
            src += 2;
        }
        else if ((b & 0xF0) == 0xE0)
        {
            u16 cp = (u16)((b & 0x0F) << 12) | ((src[1] & 0x3F) << 6) | (src[2] & 0x3F);
            dst[out++] = Hangul_LookupTile(cp);
            src += 3;
        }
        else { src++; }
    }
    dst[out] = EOS;
    return out;
}
