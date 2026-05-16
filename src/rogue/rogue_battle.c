// ============================================================
//  rogue_battle.c
//  PokéRogue GBA - 배틀 초기화 / pokeemerald 배틀 엔진 연동
// ============================================================

#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "pokemon.h"
#include "random.h"
#include "sound.h"
#include "main.h"
#include "move.h"
#include "constants/battle.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/moves.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_biome.h"
#include "rogue/rogue_passive.h"
#include "rogue/rogue_wave.h"
#include "rogue/rogue_battle.h"
#include "rogue/rogue_ui.h"

// ============================================================
//  Battle_ApplyEvs (가장 먼저 정의)
// ============================================================
static void Battle_ApplyEvs(struct Pokemon *mon, u8 evAmount)
{
    SetMonData(mon, MON_DATA_HP_EV,    &evAmount);
    SetMonData(mon, MON_DATA_ATK_EV,   &evAmount);
    SetMonData(mon, MON_DATA_DEF_EV,   &evAmount);
    SetMonData(mon, MON_DATA_SPATK_EV, &evAmount);
    SetMonData(mon, MON_DATA_SPDEF_EV, &evAmount);
    SetMonData(mon, MON_DATA_SPEED_EV, &evAmount);
    CalculateMonStats(mon);
}

// ============================================================
//  야생 적 파티 생성
// ============================================================
static void Battle_SetupWild(void)
{
    u8   wave  = gRogueRun.currentWave;
    u8   biome = gRogueRun.currentBiome;
    u32  seed  = gRogueRun.waveSeed;
    struct WaveDifficultyParams diff;

    RogueWave_GetDifficultyParams(wave, &diff);

    u16 species = RogueBiome_RollSpecies((RogueBiome)biome, seed);
    u8  level   = RogueWave_GetEnemyMinLevel(wave)
                  + (u8)((seed & 0xF) % 4);
    if (level > 100) level = 100;

    CreateMonWithIVsPersonality(&gEnemyParty[0],
        species, level, diff.ivBonus * 31 / 100, Random());
    GiveMonInitialMoveset(&gEnemyParty[0]);
    Battle_ApplyEvs(&gEnemyParty[0], diff.evBonus);

    gEnemyPartyCount = 1;
}

// ============================================================
//  보스 파티 생성
// ============================================================
static void Battle_SetupBoss(void)
{
    u8   biome = gRogueRun.currentBiome;
    u8   wave  = gRogueRun.currentWave;
    u32  seed  = gRogueRun.waveSeed;
    const struct RogueBossEntry *boss;
    u8   i;

    boss = RogueBiome_GetBoss((RogueBiome)biome, wave, seed);
    if (boss == NULL)
    {
        Battle_SetupWild();
        return;
    }

    CreateMonWithIVsPersonality(&gEnemyParty[0],
        boss->species, boss->level, 31, Random());

    for (i = 0; i < 4; i++)
    {
        if (boss->moves[i] != MOVE_NONE)
        {
            SetMonMoveSlot(&gEnemyParty[0], boss->moves[i], i);
            u8 pp = gMovesInfo[boss->moves[i]].pp;
            SetMonData(&gEnemyParty[0], MON_DATA_PP1 + i, &pp);
        }
    }

    Battle_ApplyEvs(&gEnemyParty[0], 252);

    if (boss->heldItem != ITEM_NONE)
        SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, &boss->heldItem);

    CalculateMonStats(&gEnemyParty[0]);
    gEnemyPartyCount = 1;
}

// ============================================================
//  엘리트 파티 생성
// ============================================================
static void Battle_SetupElite(void)
{
    u8   wave  = gRogueRun.currentWave;
    u8   biome = gRogueRun.currentBiome;
    u32  seed  = gRogueRun.waveSeed;
    u8   bonus = RogueWave_GetEliteBonus(wave);
    u16  species;
    u8   level;

    species = RogueBiome_RollSpecies((RogueBiome)biome, seed);
    level   = RogueWave_GetEnemyMaxLevel(wave) + bonus;
    if (level > 100) level = 100;

    CreateMonWithIVsPersonality(&gEnemyParty[0],
        species, level, 25, Random());
    GiveMonInitialMoveset(&gEnemyParty[0]);
    Battle_ApplyEvs(&gEnemyParty[0], 180);

    CalculateMonStats(&gEnemyParty[0]);
    gEnemyPartyCount = 1;
}

// ============================================================
//  배틀 종료 복귀 콜백
// ============================================================
static void CB2_RogueBattleReturn(void)
{
    switch (gBattleOutcome)
    {
    case B_OUTCOME_WON:
        gRogueRun.state     = ROGUE_STATE_VICTORY;
        gRogueRun.prevState = ROGUE_STATE_COUNT;
        break;
    case B_OUTCOME_LOST:
        RogueMain_EndRun(FALSE);
        break;
    default:
        RogueMain_SetState(ROGUE_STATE_NODE_MAP);
        break;
    }

    gBattleOutcome = 0;
    SetVBlankCallback(RogueMain_GetVBlankCb());
    SetMainCallback2(RogueMain_GetMainCb());
}

// ============================================================
//  배틀 초기화 실행
// ============================================================
void RogueBattleInit_Run(void)
{
    ZeroEnemyPartyMons();

    if (gRogueRun.isBoss)
        Battle_SetupBoss();
    else if (gRogueRun.isElite)
        Battle_SetupElite();
    else
        Battle_SetupWild();

    if (gRogueRun.isBoss)
        gBattleTypeFlags = BATTLE_TYPE_LEGENDARY;
    else
        gBattleTypeFlags = 0;

    if (gRogueRun.isBoss)
        PlayBGM(MUS_VS_GYM_LEADER);
    else if (gRogueRun.isElite)
        PlayBGM(MUS_VS_TRAINER);
    else
        PlayBGM(MUS_VS_WILD);

    gMain.savedCallback = CB2_RogueBattleReturn;
    BattleSetup_StartWildBattle();

    gRogueRun.state     = ROGUE_STATE_BATTLE;
    gRogueRun.prevState = ROGUE_STATE_BATTLE;
}

// ============================================================
//  승리 화면
// ============================================================
void RogueVictory_Init(void)
{
    gRogueRun.battleWins++;
    PlaySE(SE_EXP_MAX);
    RogueUI_InitBg();
}

void RogueVictory_Update(void)
{
    gRogueRun.uiTimer++;
    if (gRogueRun.uiTimer > 60 || JOY_NEW(A_BUTTON))
        RogueMain_SetState(ROGUE_STATE_REWARD);
}
