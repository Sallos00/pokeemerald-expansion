// ============================================================
//  rogue_reward.c  –  보상 선택 화면
//  pokeemerald-expansion Window API 기반
// ============================================================

#include "global.h"
#include "window.h"
#include "text.h"
#include "bg.h"
#include "sound.h"
#include "pokemon.h"
#include "item.h"
#include "string_util.h"
#include "constants/songs.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/characters.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_biome.h"
#include "rogue/rogue_passive.h"
#include "rogue/rogue_reward.h"
#include "rogue/rogue_ui.h"
#include "data/rogue/reward_data.h"

// ── 윈도우 ────────────────────────────────────────────────────
enum { WIN_TITLE, WIN_C0, WIN_C1, WIN_C2, WIN_HINT, REWARD_WIN_COUNT };

static const struct WindowTemplate sWinTemplates[REWARD_WIN_COUNT] =
{
    [WIN_TITLE] = ROGUE_WIN(0,  0,  0, 30,  2, 15, 0x001),
    [WIN_C0]    = ROGUE_WIN(0,  1,  2,  9, 14, 15, 0x03D),
    [WIN_C1]    = ROGUE_WIN(0, 11,  2,  9, 14, 15, 0x0C1),
    [WIN_C2]    = ROGUE_WIN(0, 21,  2,  9, 14, 15, 0x145),
    [WIN_HINT]  = ROGUE_WIN(0,  0, 18, 30,  2, 15, 0x1C9),
};
static u8 sWinIds[REWARD_WIN_COUNT];

// 카드 윈도우 인덱스 → WIN_Cn 매핑
static const u8 sCardWin[ROGUE_MAX_REWARD_CHOICES] = { WIN_C0, WIN_C1, WIN_C2 };

static u8 sCursor = 0;

// ── 보상 value → 표시 이름 ───────────────────────────────────
static void GetValueName(u8 type, u16 value, u8 *out, u8 maxLen)
{
    u8 tmp[12];
    switch (type)
    {
    case REWARD_ITEM:
    case REWARD_TM:
        StringCopyN(out, GetItemName(value), maxLen);
        break;
    case REWARD_PASSIVE:
        if (value < PASSIVE_ID_COUNT)
            
            StringCopyN(out, gPassiveDefs[value].name ? gPassiveDefs[value].name : (const u8 *)"PASSIVE", maxLen);
        else
            StringCopy(out, (const u8 *)"PASSIVE");
        break;
    case REWARD_POKEMON:
        StringCopyN(out, gSpeciesInfo[value].speciesName, maxLen);
        break;
    case REWARD_MONEY:
        ConvertIntToDecimalStringN(tmp, value, STR_CONV_MODE_LEFT_ALIGN, 4);
        StringCopy(out, tmp);
        out[4] = 'G'; out[5] = EOS;
        break;
    default:
        StringCopy(out, (const u8 *)"???");
        break;
    }
}

static void DrawCard(u8 choiceIdx, bool8 selected)
{
    static const u8 * const sTypeLabel[REWARD_COUNT] =
    {
        [REWARD_ITEM]    = (const u8 *)"[ITEM]",
        [REWARD_PASSIVE] = (const u8 *)"[PASS]",
        [REWARD_MONEY]   = (const u8 *)"[GOLD]",
        [REWARD_POKEMON] = (const u8 *)"[PKMN]",
        [REWARD_TM]      = (const u8 *)"[TM]",
    };

    u8  wid  = sWinIds[sCardWin[choiceIdx]];
    u8  type = gRogueRun.rewardTypes[choiceIdx];
    u16 val  = gRogueRun.rewardValues[choiceIdx];
    u8  nameBuf[20];

    FillWindowPixelBuffer(wid, PIXEL_FILL(selected ? 1 : 0));

    AddTextPrinterParameterized(wid, FONT_SMALL,
        sTypeLabel[type], 4, 4, TEXT_SKIP_DRAW, NULL);

    GetValueName(type, val, nameBuf, 18);
    // 이름이 길면 두 줄로 나눔 (간단 구현: 8자 단위)
    AddTextPrinterParameterized(wid, FONT_SMALL,
        nameBuf, 4, 20, TEXT_SKIP_DRAW, NULL);

    if (selected)
    {
        static const u8 sSel[] = { CHAR_RIGHT_ARROW, EOS };
        AddTextPrinterParameterized(wid, FONT_SMALL,
            sSel, 4, 80, TEXT_SKIP_DRAW, NULL);
    }

    CopyWindowToVram(wid, COPYWIN_FULL);
}

// ── 보상 풀 빌드 ─────────────────────────────────────────────
static void BuildRewards(void)
{
    u8  i;
    u32 seed = gRogueRun.waveSeed ^ 0xFEED0000u;
    u8  wave = gRogueRun.currentWave;
    u8  biome = gRogueRun.currentBiome;

    gRogueRun.rewardChosen = 0xFF;
    gRogueRun.rewardCount  = ROGUE_MAX_REWARD_CHOICES;

    for (i = 0; i < ROGUE_MAX_REWARD_CHOICES; i++)
    {
        seed = seed * 0x41C64E6Du + 0x6073u + i;
        u8  type;
        u16 value;

        // 간단한 타입 선택: 초반 아이템 위주, 후반 패시브 위주
        u8 roll = (u8)(seed & 0xFF);
        if (i == 1 && wave > 5)            type = REWARD_PASSIVE;
        else if (roll < 120)               type = REWARD_ITEM;
        else if (roll < 180)               type = REWARD_PASSIVE;
        else if (roll < 220)               type = REWARD_POKEMON;
        else                               type = REWARD_MONEY;

        switch (type)
        {
        case REWARD_ITEM:
            value = RogueReward_RollItem((RogueBiome)biome, wave, seed);
            break;
        case REWARD_PASSIVE:
        {
            RoguePassiveId pool[PASSIVE_ID_COUNT];
            u8 n = RoguePassive_BuildRewardPool(pool, PASSIVE_ID_COUNT);
            if (n == 0) { type = REWARD_ITEM; value = ITEM_POTION; break; }
            value = (u16)pool[seed % n];
            break;
        }
        case REWARD_POKEMON:
            value = RogueBiome_RollSpecies((RogueBiome)biome, seed);
            break;
        case REWARD_MONEY:
            value = 50 + (u16)(seed % 451);
            break;
        default:
            value = ITEM_POTION;
            break;
        }

        gRogueRun.rewardTypes[i]  = type;
        gRogueRun.rewardValues[i] = value;
    }
}

// ── 보상 적용 ─────────────────────────────────────────────────
static void ApplyReward(u8 idx)
{
    u8  type  = gRogueRun.rewardTypes[idx];
    u16 value = gRogueRun.rewardValues[idx];

    switch (type)
    {
    case REWARD_ITEM:
    case REWARD_TM:
        RogueReward_GiveItem(value);
        break;
    case REWARD_PASSIVE:
        RoguePassive_Add((RoguePassiveId)value);
        break;
    case REWARD_MONEY:
        gRogueRun.money = (u16)(gRogueRun.money + value);
        if (gRogueRun.money > 9999) gRogueRun.money = 9999;
        break;
    case REWARD_POKEMON:
        RogueReward_GivePokemon(value,
            (u8)(5 + gRogueRun.currentWave / 2));
        break;
    default:
        break;
    }
}

// ── STATE: Init / Update ───────────────────────────────────────
void RogueReward_Init(void)
{
    u8 i;
    static const u8 sTitle[] = {
        '-',' ','C','H','O','O','S','E',' ','R','E','W','A','R','D',' ','-',EOS };
    static const u8 sHint[]  = {
        'L','R',':',' ','M','O','V','E',' ',' ','A',':',' ','P','I','C','K',EOS };

    RogueUI_InitBg();
    sCursor = 0;

    BuildRewards();

    for (i = 0; i < REWARD_WIN_COUNT; i++)
    {
        sWinIds[i] = AddWindow(&sWinTemplates[i]);
        FillWindowPixelBuffer(sWinIds[i], PIXEL_FILL(0));
    }

    AddTextPrinterParameterized(sWinIds[WIN_TITLE], FONT_NORMAL,
        sTitle, 4, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sWinIds[WIN_TITLE], COPYWIN_FULL);

    AddTextPrinterParameterized(sWinIds[WIN_HINT], FONT_SMALL,
        sHint, 4, 2, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sWinIds[WIN_HINT], COPYWIN_FULL);

    for (i = 0; i < ROGUE_MAX_REWARD_CHOICES; i++)
        DrawCard(i, i == sCursor);
}

void RogueReward_Update(void)
{
    if (JOY_NEW(L_BUTTON) && sCursor > 0)
    {
        u8 prev = sCursor;
        sCursor--;
        PlaySE(SE_SELECT);
        DrawCard(prev, FALSE);
        DrawCard(sCursor, TRUE);
    }
    else if (JOY_NEW(R_BUTTON) && sCursor < ROGUE_MAX_REWARD_CHOICES - 1)
    {
        u8 prev = sCursor;
        sCursor++;
        PlaySE(SE_SELECT);
        DrawCard(prev, FALSE);
        DrawCard(sCursor, TRUE);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u8 i;
        ApplyReward(sCursor);
        gRogueRun.rewardChosen = sCursor;
        for (i = 0; i < REWARD_WIN_COUNT; i++) RemoveWindow(sWinIds[i]);
        PlaySE(SE_EXP_MAX);
        RogueMain_NextWave();
    }
}

// ── 아이템 지급 ───────────────────────────────────────────────
void RogueReward_GiveItem(u16 itemId)
{
    u8 i;
    for (i = 0; i < gRogueRun.itemCount; i++)
    {
        if (gRogueRun.items[i].itemId == itemId)
        {
            gRogueRun.items[i].count++;
            return;
        }
    }
    if (gRogueRun.itemCount < ROGUE_MAX_ITEMS)
    {
        gRogueRun.items[gRogueRun.itemCount].itemId = itemId;
        gRogueRun.items[gRogueRun.itemCount].count  = 1;
        gRogueRun.itemCount++;
    }
}

// ── 포켓몬 지급 ───────────────────────────────────────────────
void RogueReward_GivePokemon(u16 species, u8 level)
{
    u8 slot;
    if (gPlayerPartyCount >= ROGUE_MAX_TEAM_SIZE) return;
    slot = gPlayerPartyCount;
    CreateMon(&gPlayerParty[slot], species, level,
              Random(), OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(&gPlayerParty[slot]);
    gRogueRun.partyMeta[slot].biomeOrigin = gRogueRun.currentBiome;
    gRogueRun.partyMeta[slot].waveJoined  = gRogueRun.currentWave;
    gPlayerPartyCount++;
}

// ── 아이템 롤 ─────────────────────────────────────────────────
u16 RogueReward_RollItem(RogueBiome biome, u8 wave, u32 seed)
{
    if ((seed & 1) == 0)
    {
        u16 bi = RogueBiome_RollReward(biome, seed >> 1);
        if (bi != ITEM_NONE) return bi;
    }
    return RogueReward_RollGeneralItem(wave, seed);
}
