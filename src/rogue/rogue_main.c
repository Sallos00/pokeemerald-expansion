// ============================================================
//  rogue_main.c
//  PokéRogue GBA - 메인 상태 머신
//  pokeemerald-expansion 콜백 구조에 맞게 통합
// ============================================================

#include "global.h"
#include "main.h"
#include "palette.h"
#include "task.h"
#include "sprite.h"
#include "gpu_regs.h"
#include "sound.h"
#include "random.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/songs.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_biome.h"
#include "rogue/rogue_passive.h"
#include "rogue/rogue_save.h"
#include "rogue/rogue_ui.h"

// ── EWRAM 전역 인스턴스 ───────────────────────────────────────
EWRAM_DATA struct RogueRunData  gRogueRun  = {0};
EWRAM_DATA struct RogueMetaData gRogueMeta = {0};

// ── VBlank 콜백 ───────────────────────────────────────────────
static void RogueVBlankCb(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

// ── 메인 콜백 (매 프레임) ─────────────────────────────────────
static void RogueMainCb(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

// ── 상태 디스패치 태스크 ──────────────────────────────────────
typedef struct { u8 stateId; u8 step; } RogueTaskData;

static void Task_RogueUpdate(u8 taskId)
{
    u8 prevState = gRogueRun.prevState;
    u8 curState  = gRogueRun.state;

    // 상태 전환 시 → Init 함수 호출
    if (prevState != curState)
    {
        gRogueRun.prevState = curState;
        gRogueRun.uiStep    = 0;
        gRogueRun.uiTimer   = 0;

        switch (curState)
        {
        case ROGUE_STATE_TITLE:          RogueTitle_Init();      break;
        case ROGUE_STATE_STARTER_SELECT: RogueStarter_Init();    break;
        case ROGUE_STATE_NODE_MAP:       RogueNodeMap_Init();    break;
        case ROGUE_STATE_BATTLE_INIT:    RogueBattleInit_Run();  break;
        case ROGUE_STATE_VICTORY:        RogueVictory_Init();    break;
        case ROGUE_STATE_REWARD:         RogueReward_Init();     break;
        case ROGUE_STATE_SHOP:           RogueShop_Init();       break;
        case ROGUE_STATE_HEAL:           RogueHeal_Init();       break;
        case ROGUE_STATE_EVENT:          RogueEvent_Init();      break;
        case ROGUE_STATE_BOSS_INTRO:     RogueBossIntro_Init();  break;
        case ROGUE_STATE_RUN_END:        RogueRunEnd_Init();     break;
        case ROGUE_STATE_META:           RogueMeta_Init();       break;
        default: break;
        }
        return;
    }

    // 일반 업데이트
    switch (curState)
    {
    case ROGUE_STATE_TITLE:          RogueTitle_Update();      break;
    case ROGUE_STATE_STARTER_SELECT: RogueStarter_Update();   break;
    case ROGUE_STATE_NODE_MAP:       RogueNodeMap_Update();   break;
    case ROGUE_STATE_BATTLE:         /* pokeemerald 배틀 진행 중 */ break;
    case ROGUE_STATE_VICTORY:        RogueVictory_Update();   break;
    case ROGUE_STATE_REWARD:         RogueReward_Update();    break;
    case ROGUE_STATE_SHOP:           RogueShop_Update();      break;
    case ROGUE_STATE_HEAL:           RogueHeal_Update();      break;
    case ROGUE_STATE_EVENT:          RogueEvent_Update();     break;
    case ROGUE_STATE_BOSS_INTRO:     RogueBossIntro_Update(); break;
    case ROGUE_STATE_RUN_END:        RogueRunEnd_Update();    break;
    case ROGUE_STATE_META:           RogueMeta_Update();      break;
    default: break;
    }
}

// ============================================================
//  초기화 (title_screen.c에서 호출)
// ============================================================
void RogueMain_Init(void)
{
    SetVBlankCallback(RogueVBlankCb);
    SetMainCallback2(RogueMainCb);

    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetPaletteFade();
    ScanlineEffect_Stop();

    // 메타 데이터 로드
    RogueSave_LoadMeta(&gRogueMeta);
    if (gRogueMeta.magic != ROGUE_META_SAVE_MAGIC ||
        gRogueMeta.version != ROGUE_META_VERSION)
    {
        RogueSave_InitMeta(&gRogueMeta);
        RogueSave_SaveMeta(&gRogueMeta);
    }

    // 바이옴/패시브 초기화
    RogueBiome_Init();
    RoguePassive_Init();

    // 타이틀 상태로 시작
    memset(&gRogueRun, 0, sizeof(gRogueRun));
    gRogueRun.state     = ROGUE_STATE_TITLE;
    gRogueRun.prevState = ROGUE_STATE_COUNT; // 첫 Init 강제 트리거

    // 태스크 등록
    CreateTask(Task_RogueUpdate, 0);
}

// ============================================================
//  런 시작
// ============================================================
void RogueMain_StartNewRun(u16 starterSpecies)
{
    u8 level = 5 + gRogueMeta.permUpgrades[PERM_UPGRADE_STARTER_LEVEL] * 2;

    // 런 데이터 초기화
    memset(&gRogueRun, 0, sizeof(gRogueRun));
    gRogueRun.money        = 500;
    gRogueRun.currentWave  = 1;
    gRogueRun.currentBiome = BIOME_GRASS;
    gRogueRun.rewardChosen = 0xFF;
    gRogueRun.shopDiscount = gRogueMeta.permUpgrades[PERM_UPGRADE_SHOP_DISCOUNT] * 5;
    gRogueRun.runActive    = TRUE;

    // 시드 생성
    gRogueRun.runSeed  = ((u32)Random() << 16) | Random();
    gRogueRun.waveSeed = gRogueRun.runSeed;

    // 스타터 지급
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], starterSpecies, level,
              Random(), OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(&gPlayerParty[0]);
    gPlayerPartyCount = 1;

    // 노드 맵 생성
    RogueNodeMap_Generate(gRogueRun.runSeed);

    gRogueRun.prevState = ROGUE_STATE_COUNT;
    RogueMain_SetState(ROGUE_STATE_NODE_MAP);
}

// ============================================================
//  런 종료
// ============================================================
void RogueMain_EndRun(bool8 victory)
{
    gRogueMeta.totalRuns++;
    if (victory)
        gRogueMeta.totalWins++;
    if (gRogueRun.currentWave > gRogueMeta.bestWave)
        gRogueMeta.bestWave = gRogueRun.currentWave;

    // 바이옴 해금
    u8 biome = gRogueRun.currentBiome;
    if (biome < BIOME_COUNT)
        gRogueMeta.unlockedBiomes |= (u16)(1 << biome);

    RogueSave_SaveMeta(&gRogueMeta);
    gRogueRun.runActive = FALSE;
    RogueMain_SetState(ROGUE_STATE_RUN_END);
}

// ============================================================
//  상태 전환
// ============================================================
void RogueMain_SetState(u8 state)
{
    gRogueRun.prevState = gRogueRun.state;
    gRogueRun.state     = state;
    gRogueRun.uiStep    = 0;
    gRogueRun.uiTimer   = 0;
}

// ============================================================
//  다음 웨이브
// ============================================================
void RogueMain_NextWave(void)
{
    gRogueRun.currentWave++;
    // 웨이브별 시드 갱신 (LCG)
    gRogueRun.waveSeed = gRogueRun.runSeed
                         ^ ((u32)gRogueRun.currentWave * 0x6C078965u);

    if (gRogueRun.currentWave > ROGUE_MAX_WAVES)
    {
        RogueMain_EndRun(TRUE);
        return;
    }

    if ((gRogueRun.currentWave % ROGUE_BOSS_WAVE_INTERVAL) == 0)
    {
        gRogueRun.isBoss = TRUE;
        RogueMain_SetState(ROGUE_STATE_BOSS_INTRO);
    }
    else
    {
        gRogueRun.isBoss = FALSE;
        RogueMain_SetState(ROGUE_STATE_NODE_MAP);
    }
}

// ============================================================
//  LCG 랜덤 (시드 포함 갱신)
// ============================================================
u32 RogueMain_LcgRand(u32 *seed)
{
    *seed = (*seed) * 0x41C64E6Du + 0x6073u;
    return *seed;
}

// ============================================================
//  보스 인트로 / 런엔드 (간단 구현)
// ============================================================
void RogueBossIntro_Init(void)
{
    RogueUI_InitBg();
    // 보스 등장 BGM
    PlayBGM(MUS_VS_GYM_LEADER);
}

void RogueBossIntro_Update(void)
{
    gRogueRun.uiTimer++;
    if (gRogueRun.uiTimer > 120 || JOY_NEW(A_BUTTON))
    {
        gRogueRun.isBoss = TRUE;
        RogueMain_SetState(ROGUE_STATE_BATTLE_INIT);
    }
}

void RogueRunEnd_Init(void)
{
    RogueUI_InitBg();
}

void RogueRunEnd_Update(void)
{
    if (JOY_NEW(A_BUTTON))
        RogueMain_SetState(ROGUE_STATE_TITLE);
}

// ============================================================
//  콜백 포인터 반환 (battle 복귀 등 외부 사용)
// ============================================================
MainCallback RogueMain_GetMainCb(void)
{
    return RogueMainCb;
}

MainCallback RogueMain_GetVBlankCb(void)
{
    return RogueVBlankCb;
}
