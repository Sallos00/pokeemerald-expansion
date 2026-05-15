// ============================================================
//  rogue_title.c  –  타이틀 / 힐 / 이벤트 화면
// ============================================================

#include "global.h"
#include "window.h"
#include "text.h"
#include "bg.h"
#include "sound.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/songs.h"
#include "constants/characters.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_reward.h"
#include "rogue/rogue_ui.h"

// ============================================================
//  타이틀 화면
// ============================================================
static u8 sTitleWin;

void RogueTitle_Init(void)
{
    static const struct WindowTemplate sTpl =
        ROGUE_WIN(0, 2, 4, 26, 12, 15, 0x001);

    static const u8 sLine1[] = {
        'P','o','k','e','R','o','g','u','e',' ','G','B','A',EOS };
    static const u8 sLine2[] = {
        'A',':',' ','S','T','A','R','T',EOS };
    static const u8 sLine3[] = {
        'B',':',' ','M','E','T','A',EOS };

    RogueUI_InitBg();
    sTitleWin = AddWindow(&sTpl);
    FillWindowPixelBuffer(sTitleWin, PIXEL_FILL(0));

    AddTextPrinterParameterized(sTitleWin, FONT_NORMAL,
        sLine1, 8, 10, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sTitleWin, FONT_SMALL,
        sLine2, 24, 50, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sTitleWin, FONT_SMALL,
        sLine3, 24, 64, TEXT_SKIP_DRAW, NULL);

    CopyWindowToVram(sTitleWin, COPYWIN_FULL);
    PlayBGM(MUS_VS_WILD);
}

void RogueTitle_Update(void)
{
    if (JOY_NEW(A_BUTTON))
    {
        RemoveWindow(sTitleWin);
        RogueMain_SetState(ROGUE_STATE_STARTER_SELECT);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        RemoveWindow(sTitleWin);
        RogueMain_SetState(ROGUE_STATE_META);
    }
}

// ============================================================
//  회복 화면
// ============================================================
static u8 sHealWin;

void RogueHeal_Init(void)
{
    static const struct WindowTemplate sTpl =
        ROGUE_WIN(0, 2, 3, 26, 14, 15, 0x001);

    u8 i;
    u8 healPct = (u8)(50 + gRogueMeta.permUpgrades[PERM_UPGRADE_HEAL_BONUS] * 10);
    if (healPct > 100) healPct = 100;

    // 파티 전원 회복
    for (i = 0; i < gPlayerPartyCount; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u16 maxHp = (u16)GetMonData(mon, MON_DATA_MAX_HP);
        u16 curHp = (u16)GetMonData(mon, MON_DATA_HP);
        u16 heal  = (u16)((u32)maxHp * healPct / 100);
        u16 newHp = (u16)(curHp + heal);
        if (newHp > maxHp) newHp = maxHp;
        SetMonData(mon, MON_DATA_HP, &newHp);
        // 상태이상 해제
        u32 status = 0;
        SetMonData(mon, MON_DATA_STATUS, &status);
    }

    RogueUI_InitBg();
    sHealWin = AddWindow(&sTpl);
    FillWindowPixelBuffer(sHealWin, PIXEL_FILL(0));

    u8 buf[8];
    static const u8 sMsg1[] = {'H','E','A','L','E','D','!',EOS};
    static const u8 sMsg2[] = {'R','E','C','O','V','E','R','E','D',':',EOS};
    static const u8 sMsg3[] = {'A',':',' ','C','O','N','T','I','N','U','E',EOS};

    AddTextPrinterParameterized(sHealWin, FONT_NORMAL, sMsg1,  20, 10, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sHealWin, FONT_SMALL,  sMsg2,  8,  40, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, healPct, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized(sHealWin, FONT_SMALL,  buf,    90, 40, TEXT_SKIP_DRAW, NULL);
    static const u8 sPct[] = { '%', EOS };
    AddTextPrinterParameterized(sHealWin, FONT_SMALL,  sPct,  114, 40, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sHealWin, FONT_SMALL,  sMsg3,  20, 80, TEXT_SKIP_DRAW, NULL);

    CopyWindowToVram(sHealWin, COPYWIN_FULL);
    PlaySE(SE_EXP);
}

void RogueHeal_Update(void)
{
    if (JOY_NEW(A_BUTTON))
    {
        RemoveWindow(sHealWin);
        RogueMain_SetState(ROGUE_STATE_NODE_MAP);
    }
}

// ============================================================
//  이벤트 화면
// ============================================================
static u8 sEventWin;

static void Event_ShrineHeal(void)
{
    u8 i;
    for (i = 0; i < gPlayerPartyCount; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u16 maxHp = (u16)GetMonData(mon, MON_DATA_MAX_HP);
        SetMonData(mon, MON_DATA_HP, &maxHp);
        u32 status = 0;
        SetMonData(mon, MON_DATA_STATUS, &status);
    }
}

void RogueEvent_Init(void)
{
    static const struct WindowTemplate sTpl =
        ROGUE_WIN(0, 2, 3, 26, 14, 15, 0x001);

    RogueUI_InitBg();
    sEventWin = AddWindow(&sTpl);
    FillWindowPixelBuffer(sEventWin, PIXEL_FILL(0));

    u8 eventType = (u8)(gRogueRun.waveSeed % EVENT_COUNT);
    const u8 *msg;

    switch (eventType)
    {
    case EVENT_SHRINE:
        Event_ShrineHeal();
        msg = (const u8 *)"SACRED SHRINE!\nFULL HEAL!";
        PlaySE(SE_EXP);
        break;
    case EVENT_MYSTERY_ITEM:
    {
        u16 item = RogueReward_RollItem(
            (RogueBiome)gRogueRun.currentBiome,
            gRogueRun.currentWave,
            gRogueRun.waveSeed ^ 0xEEEEu);
        RogueReward_GiveItem(item);
        msg = (const u8 *)"MYSTERY BOX!\nGOT AN ITEM!";
        PlaySE(SE_EXP_MAX);
        break;
    }
    default:
        msg = (const u8 *)"STRANGE PLACE...\nNOTHING HAPPENED.";
        break;
    }

    AddTextPrinterParameterized(sEventWin, FONT_NORMAL,
        msg, 8, 20, TEXT_SKIP_DRAW, NULL);

    static const u8 sCont[] = {'A',':',' ','C','O','N','T','I','N','U','E',EOS};
    AddTextPrinterParameterized(sEventWin, FONT_SMALL,
        sCont, 20, 80, TEXT_SKIP_DRAW, NULL);

    CopyWindowToVram(sEventWin, COPYWIN_FULL);
}

void RogueEvent_Update(void)
{
    if (JOY_NEW(A_BUTTON))
    {
        RemoveWindow(sEventWin);
        RogueMain_SetState(ROGUE_STATE_NODE_MAP);
    }
}
