#ifndef ROGUE_BIOME_H
#define ROGUE_BIOME_H

// ============================================================
//  rogue_biome.h
//  PokéRogue GBA - 바이옴 스폰 / 보스 / 보상 테이블
// ============================================================

#include "global.h"
#include "rogue/rogue_defines.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"

// ── 스폰 엔트리 ───────────────────────────────────────────────
struct RogueSpawnEntry
{
    u16 species;
    u8  weight;        // 웨이트 합계 기반 랜덤 (0–255)
    u8  minLevel;
    u8  maxLevel;
    u8  form;          // 폼 인덱스 (0 = 기본)
    u8  isShinyLocked; // 1 = 이 슬롯은 샤이니 불가
    u8  _pad;
};

// ── 보스 엔트리 ───────────────────────────────────────────────
struct RogueBossEntry
{
    u16 species;
    u8  level;
    u8  form;
    u16 moves[4];      // 기술 ID (0 = 공백)
    u16 heldItem;
    u8  nature;
    u8  _pad;
};

// ── 바이옴 보상 ───────────────────────────────────────────────
struct RogueBiomeReward
{
    u16 itemId;
    u8  weight;
    u8  _pad;
};

// ── 바이옴 정의 ───────────────────────────────────────────────
struct RogueBiomeDef
{
    const struct RogueSpawnEntry *spawnTable;
    u8   spawnCount;

    const struct RogueBossEntry  *bossTable;
    u8   bossCount;

    const struct RogueBiomeReward *rewardTable;
    u8   rewardCount;

    // 다음 바이옴 연결 (가중치 기반 분기)
    u8   nextBiomes[4];
    u8   nextWeights[4];
    u8   nextCount;

    u8   minWave;     // 이 바이옴이 등장하는 최소 웨이브
    u8   _pad[2];

};

// ── 전역 바이옴 테이블 (ROM) ──────────────────────────────────
extern const struct RogueBiomeDef gBiomeDefs[BIOME_COUNT];

// ── API ───────────────────────────────────────────────────────
void        RogueBiome_Init(void);
RogueBiome  RogueBiome_SelectNext(RogueBiome current, u32 seed);
u16         RogueBiome_RollSpecies(RogueBiome biome, u32 seed);
u8          RogueBiome_RollLevel(RogueBiome biome, u16 species, u8 wave, u32 seed);
const struct RogueBossEntry *RogueBiome_GetBoss(RogueBiome biome, u8 wave, u32 seed);
u16         RogueBiome_RollReward(RogueBiome biome, u32 seed);
bool8       RogueBiome_IsUnlocked(RogueBiome biome);

// 내부 유틸
u32  RogueBiome_WeightedRandom(const u8 *weights, u8 count, u32 seed);
u8   RogueBiome_ScaleLevel(u8 baseMin, u8 baseMax, u8 wave);

#endif // ROGUE_BIOME_H
