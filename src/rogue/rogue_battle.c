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
#include "task.h"
#include "main.h"
#include "constants/battle.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/moves.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_biome.h"
#include "rogue/rogue_passive.h"
#include "rogue/rogue_wave.h"
#include "rogue/rogue_battle.h"
#include "move.h"

// ── 전방 선언 ─────────────────────────────────────────────────
static void Battle_SetupWild(void);
static void Battle_SetupBoss(void);
static void Battle_SetupElite(void);
static void Battle_ApplyEvs(struct Pokemon *mon, u8 evAmount);

// ── 배틀 복귀 콜백 (배틀 종료 후 pokeemerald가 호출) ──────────
static void CB2_RogueBattleReturn(void);

// ============================================================
//  배틀 초기화 실행 (즉시 실행, 상태머신의 BATTLE_INIT에서 호출)
// ============================================================
void RogueBattleInit_Run(void)
{
    // 1. 적 파티 초기화
    ZeroEnemyPartyMons();

    // 2. 적 생성
    if (gRogueRun.isBoss)
        Battle_SetupBoss();
    else if (gRogueRun.isElite)
        Battle_SetupElite();
    else
        Battle_SetupWild();

    // 3. 배틀 타입 설정
    // 보스/엘리트도 야생 배틀로 처리 (트레이너 데이터 불필요)
    // BATTLE_TYPE_LEGENDARY 플래그로 AI 강화
    if (gRogueRun.isBoss)
        gBattleTypeFlags = BATTLE_TYPE_LEGENDARY;
    else
        gBattleTypeFlags = 0;

    // 4. BGM
    if (gRogueRun.isBoss)
        PlayBGM(MUS_VS_GYM_LEADER);
    else if (gRogueRun.isElite)
        PlayBGM(MUS_VS_TRAINER);
    else
        PlayBGM(MUS_VS_WILD);

    // 5. 배틀 종료 후 복귀할 콜백을 우리 함수로 지정
    gMain.savedCallback = CB2_RogueBattleReturn;
    BattleSetup_StartWildBattle();

    // 6. 상태를 BATTLE로 (배틀 진행 중 표시)
    gRogueRun.state     = ROGUE_STATE_BATTLE;
    gRogueRun.prevState = ROGUE_STATE_BATTLE;
}

// ============================================================
//  배틀 종료 복귀 콜백
//  pokeemerald battle_main.c → SetMainCallback2(gMain.savedCallback)
//  로 이 함수가 호출됨
// ============================================================
static void CB2_RogueBattleReturn(void)
{
    // 배틀 결과 처리
    switch (gBattleOutcome)
    {
    case B_OUTCOME_WON:
        gRogueRun.state     = ROGUE_STATE_VICTORY;
        gRogueRun.prevState = ROGUE_STATE_COUNT; // Init 강제 트리거
        break;
    case B_OUTCOME_LOST:
        RogueMain_EndRun(FALSE);
        break;
    case B_OUTCOME_RAN:
    case B_OUTCOME_DREW:
    default:
        RogueMain_SetState(ROGUE_STATE_NODE_MAP);
        break;
    }

    // gBattleOutcome 초기화
    gBattleOutcome = 0;

    // 메인 콜백을 rogue 루프로 복귀
    SetVBlankCallback(RogueMain_GetVBlankCb());
    SetMainCallback2(RogueMain_GetMainCb());
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

    // 파티 크기: 야생은 1마리 (pokeemerald 야생배틀 기준)
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

    boss = RogueBiome_GetBoss((RogueBiome)biome, wave, seed);
    if (boss == NULL)
    {
        // 폴백: 강한 야생 포켓몬
        Battle_SetupWild();
        return;
    }

    // 보스는 최대 IV
    CreateMonWithIVsPersonality(&gEnemyParty[0],
        boss->species, boss->level, /*fixedIV=*/31, Random());

    // 고정 기술 세팅
    {
        u8 i;
        for (i = 0; i < 4; i++)
        {
            if (boss->moves[i] != MOVE_NONE)
            {
                SetMonMoveSlot(&gEnemyParty[0], boss->moves[i], i);
                u8 pp = gMovesInfo[boss->moves[i]].pp;
                SetMonData(&gEnemyParty[0], MON_DATA_PP1 + i, &pp);
            }
        }
    }

    // EV 최대화
    Battle_ApplyEvs(&gEnemyParty[0], 252);

    // 지정 아이템
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

    // 엘리트는 높은 IV
    CreateMonWithIVsPersonality(&gEnemyParty[0],
        species, level, /*fixedIV=*/25, Random());
    GiveMonInitialMoveset(&gEnemyParty[0]);
    Battle_ApplyEvs(&gEnemyParty[0], 180);

    CalculateMonStats(&gEnemyParty[0]);
    gEnemyPartyCount = 1;
}

// ============================================================
//  EV 적용
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
