// ============================================================
//  rogue_ui.c
//  PokéRogue GBA - UI 헬퍼 구현
// ============================================================

#include "global.h"
#include "bg.h"
#include "window.h"
#include "text.h"
#include "palette.h"
#include "gpu_regs.h"
#include "dma3.h"
#include "string_util.h"
#include "text_window.h"
#include "menu.h"
#include "menu_helpers.h"
#include "constants/characters.h"

#include "rogue/rogue_ui.h"

// ── 색상 테이블 (ROM) ─────────────────────────────────────────
const u8 gRogueColorWhite[3]  = {TEXT_COLOR_TRANSPARENT,
                                  TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};
const u8 gRogueColorYellow[3] = {TEXT_COLOR_TRANSPARENT,
                                  TEXT_DYNAMIC_COLOR_1, TEXT_COLOR_DARK_GRAY};
const u8 gRogueColorGray[3]   = {TEXT_COLOR_TRANSPARENT,
                                  TEXT_COLOR_DARK_GRAY, TEXT_COLOR_TRANSPARENT};
const u8 gRogueColorRed[3]    = {TEXT_COLOR_TRANSPARENT,
                                  TEXT_DYNAMIC_COLOR_2, TEXT_COLOR_DARK_GRAY};

// ── 공유 문자열 버퍼 ─────────────────────────────────────────
u8 gRogueStrBuf[64];

// ── BG 설정 ──────────────────────────────────────────────────
// pokeemerald BG0: 텍스트 레이어로 사용 (CBB=0, SBB=31)
static u16 sBgTilemapBuf[32 * 32];  // BG0 타일맵 버퍼

void RogueUI_InitBg(void)
{
    ResetVramOamAndBgCntRegs();
    ResetBgsAndClearDma3BusyFlags(0);

    const struct BgTemplate bgTemplates[] =
    {
        {
            .bg          = 0,
            .charBaseIndex = 0,
            .mapBaseIndex  = 31,
            .screenSize    = 0,
            .paletteMode   = 0,
            .priority      = 0,
            .baseTile      = 0,
        },
    };
    InitBgsFromTemplates(0, bgTemplates, ARRAY_COUNT(bgTemplates));
    SetBgTilemapBuffer(0, sBgTilemapBuf);
    FillBgTilemapBufferRect(0, 0, 0, 0, 32, 32, 0);
    CopyBgTilemapBufferToVram(0);

    // 텍스트 팔레트 로드 (인덱스 15 = 표준 텍스트 팔레트)
    LoadMessageBoxGfx(0, 15, 0xF0);

    ShowBg(0);
}

void RogueUI_ClearScreen(void)
{
    FillBgTilemapBufferRect(0, 0, 0, 0, 32, 32, 0);
}

void RogueUI_Print(u8 windowId, u8 fontId,
                   u8 x, u8 y, const u8 *color,
                   const u8 *str)
{
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    AddTextPrinterParameterized(windowId, fontId, str, x, y,
                                TEXT_SKIP_DRAW, NULL);
    // 색상 적용 (WindowSetPalette 또는 DrawTextBorderOuter)
    // 표준 텍스트 팔레트를 사용하므로 AddTextPrinterParameterized 만으로 충분
    (void)color; // 팔레트는 윈도우 생성 시 이미 지정됨
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

void RogueUI_PrintInt(u8 windowId, u8 fontId,
                      u8 x, u8 y, const u8 *color,
                      s32 value, u8 digits)
{
    u8 buf[12];
    ConvertIntToDecimalStringN(buf, value, STR_CONV_MODE_LEFT_ALIGN, digits);
    RogueUI_Print(windowId, fontId, x, y, color, buf);
}

void RogueUI_FlushWindow(u8 windowId)
{
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

void RogueUI_ClearWindow(u8 windowId)
{
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    CopyWindowToVram(windowId, COPYWIN_GFX);
    ClearWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_MAP);
}

void RogueUI_DrawWindowFrame(u8 windowId)
{
    DrawStdWindowFrame(windowId, FALSE);
}

void RogueUI_PrintCursor(u8 windowId, u8 x, u8 y, bool8 visible)
{
    static const u8 sCursorOn[]  = { CHAR_RIGHT_ARROW, EOS };
    static const u8 sCursorOff[] = { CHAR_SPACE, EOS };
    AddTextPrinterParameterized(windowId, FONT_SMALL,
                                visible ? sCursorOn : sCursorOff,
                                x, y, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_GFX);
}
