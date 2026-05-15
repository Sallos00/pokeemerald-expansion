// ============================================================
//  rogue_starter.c  –  스타터 선택 화면
//  pokeemerald-expansion Window API 기반
// ============================================================

#include "global.h"
#include "window.h"
#include "text.h"
#include "bg.h"
#include "palette.h"
#include "sound.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/characters.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_ui.h"

// ── 스타터 풀 ─────────────────────────────────────────────────
static const u16 sStarters[] =
{
    SPECIES_BULBASAUR,  SPECIES_CHARMANDER, SPECIES_SQUIRTLE,
    SPECIES_CHIKORITA,  SPECIES_CYNDAQUIL,  SPECIES_TOTODILE,
    SPECIES_TREECKO,    SPECIES_TORCHIC,    SPECIES_MUDKIP,
    SPECIES_TURTWIG,    SPECIES_CHIMCHAR,   SPECIES_PIPLUP,
    SPECIES_SNIVY,      SPECIES_TEPIG,      SPECIES_OSHAWOTT,
    SPECIES_CHESPIN,    SPECIES_FENNEKIN,   SPECIES_FROAKIE,
    SPECIES_ROWLET,     SPECIES_LITTEN,     SPECIES_POPPLIO,
    SPECIES_GROOKEY,    SPECIES_SCORBUNNY,  SPECIES_SOBBLE,
};
#define STARTER_COUNT  ARRAY_COUNT(sStarters)
#define PAGE_SIZE       9   // 3×3 그리드

// ── 윈도우 레이아웃 ───────────────────────────────────────────
enum { WIN_TITLE, WIN_GRID, WIN_INFO, WIN_HINT, WIN_COUNT };

static const struct WindowTemplate sWinTemplates[WIN_COUNT] =
{
    [WIN_TITLE] = ROGUE_WIN(0,  0,  0, 30,  2, 15, 0x001),
    [WIN_GRID]  = ROGUE_WIN(0,  0,  2, 20, 16, 15, 0x03D),
    [WIN_INFO]  = ROGUE_WIN(0, 20,  2, 10, 14, 15, 0x12D),
    [WIN_HINT]  = ROGUE_WIN(0,  0, 18, 30,  2, 15, 0x18D),
};

static u8 sWinIds[WIN_COUNT];

// ── 내부 상태 ─────────────────────────────────────────────────
static u8 sCursorX, sCursorY, sPage;

static u16 GetSpecies(u8 idx)
{
    if (idx < STARTER_COUNT) return sStarters[idx];
    return SPECIES_BULBASAUR;
}

static void DrawGrid(void)
{
    u8 i;
    u8 pageStart = sPage * PAGE_SIZE;
    FillWindowPixelBuffer(sWinIds[WIN_GRID], PIXEL_FILL(0));

    for (i = 0; i < PAGE_SIZE; i++)
    {
        u8 idx = pageStart + i;
        if (idx >= STARTER_COUNT) break;

        u8 col = i % 3, row = i / 3;
        u8 px  = col * 54 + 2;
        u8 py  = row * 42 + 2;
        bool8 sel = (col == sCursorX && row == sCursorY);

        AddTextPrinterParameterized(sWinIds[WIN_GRID], FONT_SMALL,
            gSpeciesInfo[GetSpecies(idx)].speciesName,
            px, py, TEXT_SKIP_DRAW, NULL);

        if (sel)
        {
            static const u8 sCur[] = { CHAR_RIGHT_ARROW, EOS };
            AddTextPrinterParameterized(sWinIds[WIN_GRID], FONT_SMALL,
                sCur, px > 6 ? px - 6 : 0, py, TEXT_SKIP_DRAW, NULL);
        }
    }
    CopyWindowToVram(sWinIds[WIN_GRID], COPYWIN_FULL);
}

static void DrawInfo(void)
{
    u8 idx = sPage * PAGE_SIZE + sCursorY * 3 + sCursorX;
    if (idx >= STARTER_COUNT) return;

    u16 sp = GetSpecies(idx);
    u8  buf[12];
    FillWindowPixelBuffer(sWinIds[WIN_INFO], PIXEL_FILL(0));

    // 이름
    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        gSpeciesInfo[sp].speciesName, 2, 2, TEXT_SKIP_DRAW, NULL);

    // HP
    static const u8 sLabelHp[]  = { 'H','P',':',' ',EOS };
    static const u8 sLabelAtk[] = { 'A','T','K',':',' ',EOS };
    static const u8 sLabelSpd[] = { 'S','P','D',':',' ',EOS };

    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        sLabelHp, 2, 14, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gSpeciesInfo[sp].baseHP, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL, buf, 26, 14, TEXT_SKIP_DRAW, NULL);

    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        sLabelAtk, 2, 24, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gSpeciesInfo[sp].baseAttack, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL, buf, 32, 24, TEXT_SKIP_DRAW, NULL);

    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        sLabelSpd, 2, 34, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gSpeciesInfo[sp].baseSpeed, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL, buf, 32, 34, TEXT_SKIP_DRAW, NULL);

    CopyWindowToVram(sWinIds[WIN_INFO], COPYWIN_FULL);
}

static void DrawHint(void)
{
    static const u8 sHint[] = {
        'D','-','P','A','D',':',' ','M','O','V','E',
        ' ',' ','A',':',' ','O','K',
        ' ',' ','B',':',' ','B','A','C','K',EOS };
    FillWindowPixelBuffer(sWinIds[WIN_HINT], PIXEL_FILL(0));
    AddTextPrinterParameterized(sWinIds[WIN_HINT], FONT_SMALL,
        sHint, 2, 2, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sWinIds[WIN_HINT], COPYWIN_FULL);
}

void RogueStarter_Init(void)
{
    u8 i;
    static const u8 sTitle[] = {
        '-',' ','S','T','A','R','T','E','R',' ','S','E','L','E','C','T',' ','-',EOS };

    RogueUI_InitBg();
    sCursorX = sCursorY = sPage = 0;

    for (i = 0; i < WIN_COUNT; i++)
    {
        sWinIds[i] = AddWindow(&sWinTemplates[i]);
        FillWindowPixelBuffer(sWinIds[i], PIXEL_FILL(0));
    }

    // 타이틀
    AddTextPrinterParameterized(sWinIds[WIN_TITLE], FONT_NORMAL,
        sTitle, 4, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sWinIds[WIN_TITLE], COPYWIN_FULL);

    DrawGrid();
    DrawInfo();
    DrawHint();
}

void RogueStarter_Update(void)
{
    bool8 moved = FALSE;
    u8 pageIdx = sPage * PAGE_SIZE + sCursorY * 3 + sCursorX;

    if (JOY_NEW(DPAD_LEFT)  && sCursorX > 0)            { sCursorX--; moved = TRUE; }
    if (JOY_NEW(DPAD_RIGHT) && sCursorX < 2
        && pageIdx + 1 < STARTER_COUNT)                  { sCursorX++; moved = TRUE; }
    if (JOY_NEW(DPAD_UP)    && sCursorY > 0)            { sCursorY--; moved = TRUE; }
    if (JOY_NEW(DPAD_DOWN)  && sCursorY < 2
        && pageIdx + 3 < STARTER_COUNT)                  { sCursorY++; moved = TRUE; }
    if (JOY_NEW(L_BUTTON)   && sPage > 0)
    {
        sPage--; sCursorX = sCursorY = 0; moved = TRUE;
    }
    if (JOY_NEW(R_BUTTON)
        && (sPage + 1) * PAGE_SIZE < STARTER_COUNT)
    {
        sPage++; sCursorX = sCursorY = 0; moved = TRUE;
    }

    if (moved)
    {
        PlaySE(SE_SELECT);
        DrawGrid(); DrawInfo();
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        u8 i;
        u16 sp = GetSpecies(sPage * PAGE_SIZE + sCursorY * 3 + sCursorX);
        for (i = 0; i < WIN_COUNT; i++) RemoveWindow(sWinIds[i]);
        PlaySE(SE_PC_LOGIN);
        RogueMain_StartNewRun(sp);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        u8 i;
        for (i = 0; i < WIN_COUNT; i++) RemoveWindow(sWinIds[i]);
        RogueMain_SetState(ROGUE_STATE_TITLE);
    }
}
