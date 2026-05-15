// ============================================================
//  rogue_meta.c  –  메타 진행 / 영구 업그레이드
// ============================================================

#include "global.h"
#include "window.h"
#include "text.h"
#include "bg.h"
#include "sound.h"
#include "string_util.h"
#include "constants/songs.h"
#include "constants/characters.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_save.h"
#include "rogue/rogue_ui.h"

enum { META_WIN_TITLE, META_WIN_BODY, META_WIN_HINT, META_WIN_COUNT };

static const struct WindowTemplate sMetaWinTemplates[META_WIN_COUNT] =
{
    [META_WIN_TITLE] = ROGUE_WIN(0,  0,  0, 30,  2, 15, 0x001),
    [META_WIN_BODY]  = ROGUE_WIN(0,  0,  2, 30, 16, 15, 0x03D),
    [META_WIN_HINT]  = ROGUE_WIN(0,  0, 18, 30,  2, 15, 0x1BD),
};
static u8 sMetaWinIds[META_WIN_COUNT];

static u8 sMetaPage   = 0;  // 0=업그레이드, 1=기록
static u8 sMetaCursor = 0;

// 업그레이드 정의
static const struct {
    const u8 *name;
    u8 maxLv;
    u8 costPerLv;
} sUpgrades[PERM_UPGRADE_COUNT] =
{
    { (const u8 *)"STARTER LV+2",  5, 3 },
    { (const u8 *)"SHOP -5%",     10, 5 },
    { (const u8 *)"+HEAL %",       5, 4 },
    { (const u8 *)"+PASSIVE SLOT", 4, 8 },
};

static u16 Meta_CalcPoints(void)
{
    return (u16)(gRogueMeta.bestWave / 5);
}

static void Meta_DrawUpgrades(void)
{
    u8 i;
    u8 buf[8];
    FillWindowPixelBuffer(sMetaWinIds[META_WIN_BODY], PIXEL_FILL(0));

    // 포인트 표시
    u16 pts = Meta_CalcPoints();
    AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
        (const u8 *)"POINTS:", 2, 2, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, pts, STR_CONV_MODE_LEFT_ALIGN, 4);
    AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
        buf, 60, 2, TEXT_SKIP_DRAW, NULL);

    for (i = 0; i < PERM_UPGRADE_COUNT; i++)
    {
        u8 py  = i * 22 + 20;
        u8 lv  = gRogueMeta.permUpgrades[i];
        u8 max = sUpgrades[i].maxLv;
        bool8 sel = (i == sMetaCursor);

        if (sel)
        {
            static const u8 sCur[] = { CHAR_RIGHT_ARROW, EOS };
            AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
                sCur, 2, py, TEXT_SKIP_DRAW, NULL);
        }

        AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
            sUpgrades[i].name, 14, py, TEXT_SKIP_DRAW, NULL);

        // 레벨 바 (■□)
        u8 j, barX = 130;
        for (j = 0; j < max && j < 5; j++)
        {
            static const u8 sOn[]  = { 0x01, EOS }; // ■ (간략화)
            static const u8 sOff[] = { 0x02, EOS }; // □
            AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
                j < lv ? sOn : sOff, (u8)(barX + j * 8), py, TEXT_SKIP_DRAW, NULL);
        }

        if (lv < max)
        {
            u8 cost = sUpgrades[i].costPerLv * (lv + 1);
            ConvertIntToDecimalStringN(buf, cost, STR_CONV_MODE_LEFT_ALIGN, 3);
            buf[3] = 'P'; buf[4] = EOS;
            AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
                buf, 190, py, TEXT_SKIP_DRAW, NULL);
        }
    }
    CopyWindowToVram(sMetaWinIds[META_WIN_BODY], COPYWIN_FULL);
}

static void Meta_DrawRecord(void)
{
    u8 buf[8];
    FillWindowPixelBuffer(sMetaWinIds[META_WIN_BODY], PIXEL_FILL(0));

    AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
        (const u8 *)"TOTAL RUNS:", 4, 10, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gRogueMeta.totalRuns, STR_CONV_MODE_LEFT_ALIGN, 4);
    AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
        buf, 120, 10, TEXT_SKIP_DRAW, NULL);

    AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
        (const u8 *)"CLEARS:", 4, 30, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gRogueMeta.totalWins, STR_CONV_MODE_LEFT_ALIGN, 4);
    AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
        buf, 120, 30, TEXT_SKIP_DRAW, NULL);

    AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
        (const u8 *)"BEST WAVE:", 4, 50, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gRogueMeta.bestWave, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized(sMetaWinIds[META_WIN_BODY], FONT_SMALL,
        buf, 120, 50, TEXT_SKIP_DRAW, NULL);

    CopyWindowToVram(sMetaWinIds[META_WIN_BODY], COPYWIN_FULL);
}

static void Meta_Refresh(void)
{
    if (sMetaPage == 0) Meta_DrawUpgrades();
    else                Meta_DrawRecord();
}

void RogueMeta_Init(void)
{
    u8 i;
    static const u8 sTitle[] = {'-',' ','M','E','T','A',' ','-',EOS};
    static const u8 sHint[]  = {'L','R',':',' ','T','A','B',' ','A',':',' ','B','U','Y',' ','B',':',' ','E','X','I','T',EOS};

    RogueUI_InitBg();
    sMetaPage = 0; sMetaCursor = 0;

    for (i = 0; i < META_WIN_COUNT; i++)
    {
        sMetaWinIds[i] = AddWindow(&sMetaWinTemplates[i]);
        FillWindowPixelBuffer(sMetaWinIds[i], PIXEL_FILL(0));
    }
    AddTextPrinterParameterized(sMetaWinIds[META_WIN_TITLE], FONT_NORMAL,
        sTitle, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sMetaWinIds[META_WIN_TITLE], COPYWIN_FULL);

    AddTextPrinterParameterized(sMetaWinIds[META_WIN_HINT], FONT_SMALL,
        sHint, 2, 2, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sMetaWinIds[META_WIN_HINT], COPYWIN_FULL);

    Meta_Refresh();
}

void RogueMeta_Update(void)
{
    if (JOY_NEW(L_BUTTON) && sMetaPage > 0)
    {
        sMetaPage--; sMetaCursor = 0;
        PlaySE(SE_SELECT); Meta_Refresh(); return;
    }
    if (JOY_NEW(R_BUTTON) && sMetaPage < 1)
    {
        sMetaPage++; sMetaCursor = 0;
        PlaySE(SE_SELECT); Meta_Refresh(); return;
    }

    if (sMetaPage == 0)
    {
        if (JOY_NEW(DPAD_UP)   && sMetaCursor > 0)
            { sMetaCursor--; PlaySE(SE_SELECT); Meta_Refresh(); }
        if (JOY_NEW(DPAD_DOWN) && sMetaCursor < PERM_UPGRADE_COUNT - 1)
            { sMetaCursor++; PlaySE(SE_SELECT); Meta_Refresh(); }

        if (JOY_NEW(A_BUTTON))
        {
            u8  i   = sMetaCursor;
            u8  lv  = gRogueMeta.permUpgrades[i];
            u8  max = sUpgrades[i].maxLv;
            u16 pts = Meta_CalcPoints();
            u16 cost = (u16)(sUpgrades[i].costPerLv * (lv + 1));

            if (lv >= max || pts < cost)
            { PlaySE(SE_FAILURE); return; }

            // 포인트 소비 (bestWave 감소로 표현)
            u16 waveToSub = (u16)(cost * 5);
            gRogueMeta.bestWave = (gRogueMeta.bestWave > waveToSub)
                                   ? (u16)(gRogueMeta.bestWave - waveToSub) : 0;
            gRogueMeta.permUpgrades[i]++;
            RogueSave_SaveMeta(&gRogueMeta);
            PlaySE(SE_EXP_MAX);
            Meta_Refresh();
        }
    }

    if (JOY_NEW(B_BUTTON))
    {
        u8 i;
        for (i = 0; i < META_WIN_COUNT; i++) RemoveWindow(sMetaWinIds[i]);
        RogueMain_SetState(ROGUE_STATE_TITLE);
    }
}
