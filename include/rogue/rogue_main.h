#ifndef ROGUE_MAIN_H
#define ROGUE_MAIN_H

// ============================================================
//  rogue_main.h
//  PokéRogue GBA - 메인 런 구조체 / 상태 머신 API
// ============================================================

#include "global.h"
#include "main.h"
#include "rogue/rogue_defines.h"

// ── 패시브 슬롯 ───────────────────────────────────────────────
struct RoguePassiveSlot
{
    u8 id;      // RoguePassiveId
    u8 stacks;
};

// ── 파티 메타 ─────────────────────────────────────────────────
struct RoguePartyMeta
{
    u8 biomeOrigin;
    u8 waveJoined;
};

// ── 아이템 슬롯 ───────────────────────────────────────────────
struct RogueItemSlot
{
    u16 itemId;
    u8  count;
    u8  _pad;
};

// ── 노드 ──────────────────────────────────────────────────────
struct RogueNode
{
    u8 type;                              // RogueNodeType
    u8 biome;                             // RogueBiome
    u8 layer;
    u8 visited;
    u8 children[ROGUE_MAX_NODE_CHILDREN]; // 0xFF = 없음
    u8 childCount;
    u8 _pad[2];
};

// ── 메인 런 데이터 (EWRAM) ────────────────────────────────────
struct RogueRunData
{
    u8  state;           // RogueState
    u8  prevState;
    u8  currentWave;
    u8  currentBiome;    // RogueBiome

    u8  isBoss;
    u8  isElite;
    u8  battleWins;
    u8  runActive;

    u16 money;
    u8  shopDiscount;
    u8  passiveCount;

    struct RoguePassiveSlot passives[ROGUE_MAX_PASSIVES];
    struct RoguePartyMeta   partyMeta[ROGUE_MAX_TEAM_SIZE];
    struct RogueItemSlot    items[ROGUE_MAX_ITEMS];
    u8  itemCount;
    u8  _pad0[3];

    struct RogueNode nodeMap[ROGUE_MAX_NODES];
    u8  nodeCount;
    u8  currentNode;
    u8  selectedChild;   // 노드 맵 커서
    u8  _pad1;

    // 보상 버퍼
    u8  rewardTypes[ROGUE_MAX_REWARD_CHOICES];
    u16 rewardValues[ROGUE_MAX_REWARD_CHOICES];
    u8  rewardCount;
    u8  rewardChosen;    // 0xFF = 미선택
    u8  _pad2[2];

    u32 runSeed;
    u32 waveSeed;

    // UI 상태
    u8  uiStep;          // 각 상태의 서브스텝
    u8  uiTimer;
    u8  _pad3[2];
};

// ── 메타 데이터 (SRAM 저장) ───────────────────────────────────
struct RogueMetaData
{
    u32 magic;
    u8  version;
    u8  _pad0[3];

    u16 totalRuns;
    u16 totalWins;
    u16 bestWave;
    u8  _pad1[2];

    // 해금 비트맵 (포켓몬 스타터 최대 240종)
    u8  unlockedStarters[30];
    // 해금 바이옴 비트플래그
    u16 unlockedBiomes;
    // 영구 업그레이드 레벨
    u8  permUpgrades[PERM_UPGRADE_COUNT];
    u8  _pad2[2];
};

// ── EWRAM 전역 ────────────────────────────────────────────────
extern struct RogueRunData  gRogueRun;
extern struct RogueMetaData gRogueMeta;

// ── API ───────────────────────────────────────────────────────
void   RogueMain_Init(void);
void   RogueMain_StartNewRun(u16 starterSpecies);
void   RogueMain_EndRun(bool8 victory);
void   RogueMain_SetState(u8 state);
void   RogueMain_NextWave(void);
u32    RogueMain_LcgRand(u32 *seed);
MainCallback RogueMain_GetMainCb(void);
MainCallback RogueMain_GetVBlankCb(void);

// 상태별 초기화 / 업데이트 함수
void   RogueTitle_Init(void);
void   RogueTitle_Update(void);

void   RogueStarter_Init(void);
void   RogueStarter_Update(void);

void   RogueNodeMap_Init(void);
void   RogueNodeMap_Update(void);

void   RogueBattleInit_Run(void);
void   RogueVictory_Init(void);
void   RogueVictory_Update(void);

void   RogueReward_Init(void);
void   RogueReward_Update(void);

void   RogueShop_Init(void);
void   RogueShop_Update(void);

void   RogueHeal_Init(void);
void   RogueHeal_Update(void);

void   RogueEvent_Init(void);
void   RogueEvent_Update(void);

void   RogueBossIntro_Init(void);
void   RogueBossIntro_Update(void);

void   RogueRunEnd_Init(void);
void   RogueRunEnd_Update(void);

void   RogueMeta_Init(void);
void   RogueMeta_Update(void);

#endif // ROGUE_MAIN_H
