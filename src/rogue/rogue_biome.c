// ============================================================
//  rogue_biome.c
//  PokéRogue GBA - 바이옴 스폰 / 보스 / 보상 구현
//
//  중요: import_biomes.py 실행 시 src/data/rogue/biome_tables.h 가
//        자동 생성됨. 아래 #include로 실제 데이터 주입.
// ============================================================

#include "global.h"
#include "random.h"
#include "pokemon.h"
#include "data.h"
#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"

#include "rogue/rogue_biome.h"
#include "rogue/rogue_main.h"

// ── 인라인 데이터 테이블 (Python 자동 생성 또는 수동 작성) ─────
#include "data/rogue/biome_tables.h"

// ── 전역 바이옴 정의 테이블 ───────────────────────────────────
const struct RogueBiomeDef gBiomeDefs[BIOME_COUNT] =
{
    // BIOME_TOWN
    [BIOME_TOWN] =
    {
        .spawnTable  = gBiomeTownSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeTownSpawn),
        .bossTable   = gBiomeTownBoss,
        .bossCount   = ARRAY_COUNT(gBiomeTownBoss),
        .rewardTable = gBiomeTownReward,
        .rewardCount = ARRAY_COUNT(gBiomeTownReward),
        .nextBiomes  = {BIOME_GRASS, BIOME_FOREST, 0, 0},
        .nextWeights = {60, 40, 0, 0},
        .nextCount   = 2,
        .minWave     = 1,
    },

    // BIOME_GRASS
    [BIOME_GRASS] =
    {
        .spawnTable  = gBiomeGrassSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeGrassSpawn),
        .bossTable   = gBiomeGrassBoss,
        .bossCount   = ARRAY_COUNT(gBiomeGrassBoss),
        .rewardTable = gBiomeGrassReward,
        .rewardCount = ARRAY_COUNT(gBiomeGrassReward),
        .nextBiomes  = {BIOME_FOREST, BIOME_CAVE, BIOME_MOUNTAIN, 0},
        .nextWeights = {40, 35, 25, 0},
        .nextCount   = 3,
        .minWave     = 1,
    },

    // BIOME_FOREST
    [BIOME_FOREST] =
    {
        .spawnTable  = gBiomeForestSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeForestSpawn),
        .bossTable   = gBiomeForestBoss,
        .bossCount   = ARRAY_COUNT(gBiomeForestBoss),
        .rewardTable = gBiomeForestReward,
        .rewardCount = ARRAY_COUNT(gBiomeForestReward),
        .nextBiomes  = {BIOME_CAVE, BIOME_SWAMP, BIOME_MOUNTAIN, 0},
        .nextWeights = {40, 30, 30, 0},
        .nextCount   = 3,
        .minWave     = 3,
    },

    // BIOME_CAVE
    [BIOME_CAVE] =
    {
        .spawnTable  = gBiomeCaveSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeCaveSpawn),
        .bossTable   = gBiomeCaveBoss,
        .bossCount   = ARRAY_COUNT(gBiomeCaveBoss),
        .rewardTable = gBiomeCaveReward,
        .rewardCount = ARRAY_COUNT(gBiomeCaveReward),
        .nextBiomes  = {BIOME_MOUNTAIN, BIOME_VOLCANO, BIOME_RUINS, 0},
        .nextWeights = {50, 25, 25, 0},
        .nextCount   = 3,
        .minWave     = 5,
    },

    // BIOME_MOUNTAIN
    [BIOME_MOUNTAIN] =
    {
        .spawnTable  = gBiomeMountainSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeMountainSpawn),
        .bossTable   = gBiomeMountainBoss,
        .bossCount   = ARRAY_COUNT(gBiomeMountainBoss),
        .rewardTable = gBiomeMountainReward,
        .rewardCount = ARRAY_COUNT(gBiomeMountainReward),
        .nextBiomes  = {BIOME_VOLCANO, BIOME_ICE, BIOME_OCEAN, 0},
        .nextWeights = {40, 35, 25, 0},
        .nextCount   = 3,
        .minWave     = 7,
    },

    // BIOME_OCEAN
    [BIOME_OCEAN] =
    {
        .spawnTable  = gBiomeOceanSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeOceanSpawn),
        .bossTable   = gBiomeOceanBoss,
        .bossCount   = ARRAY_COUNT(gBiomeOceanBoss),
        .rewardTable = gBiomeOceanReward,
        .rewardCount = ARRAY_COUNT(gBiomeOceanReward),
        .nextBiomes  = {BIOME_BEACH, BIOME_FACTORY, 0, 0},
        .nextWeights = {55, 45, 0, 0},
        .nextCount   = 2,
        .minWave     = 8,
    },

    // BIOME_BEACH
    [BIOME_BEACH] =
    {
        .spawnTable  = gBiomeBeachSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeBeachSpawn),
        .bossTable   = gBiomeBeachBoss,
        .bossCount   = ARRAY_COUNT(gBiomeBeachBoss),
        .rewardTable = gBiomeBeachReward,
        .rewardCount = ARRAY_COUNT(gBiomeBeachReward),
        .nextBiomes  = {BIOME_OCEAN, BIOME_DESERT, 0, 0},
        .nextWeights = {50, 50, 0, 0},
        .nextCount   = 2,
        .minWave     = 6,
    },

    // BIOME_DESERT
    [BIOME_DESERT] =
    {
        .spawnTable  = gBiomeDesertSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeDesertSpawn),
        .bossTable   = gBiomeDesertBoss,
        .bossCount   = ARRAY_COUNT(gBiomeDesertBoss),
        .rewardTable = gBiomeDesertReward,
        .rewardCount = ARRAY_COUNT(gBiomeDesertReward),
        .nextBiomes  = {BIOME_RUINS, BIOME_VOLCANO, BIOME_FACTORY, 0},
        .nextWeights = {40, 35, 25, 0},
        .nextCount   = 3,
        .minWave     = 8,
    },

    // BIOME_VOLCANO
    [BIOME_VOLCANO] =
    {
        .spawnTable  = gBiomeVolcanoSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeVolcanoSpawn),
        .bossTable   = gBiomeVolcanoBoss,
        .bossCount   = ARRAY_COUNT(gBiomeVolcanoBoss),
        .rewardTable = gBiomeVolcanoReward,
        .rewardCount = ARRAY_COUNT(gBiomeVolcanoReward),
        .nextBiomes  = {BIOME_FACTORY, BIOME_SPACE, 0, 0},
        .nextWeights = {60, 40, 0, 0},
        .nextCount   = 2,
        .minWave     = 12,
    },

    // BIOME_ICE
    [BIOME_ICE] =
    {
        .spawnTable  = gBiomeIceSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeIceSpawn),
        .bossTable   = gBiomeIceBoss,
        .bossCount   = ARRAY_COUNT(gBiomeIceBoss),
        .rewardTable = gBiomeIceReward,
        .rewardCount = ARRAY_COUNT(gBiomeIceReward),
        .nextBiomes  = {BIOME_MOUNTAIN, BIOME_SPACE, 0, 0},
        .nextWeights = {50, 50, 0, 0},
        .nextCount   = 2,
        .minWave     = 10,
    },

    // BIOME_SWAMP
    [BIOME_SWAMP] =
    {
        .spawnTable  = gBiomeSwampSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeSwampSpawn),
        .bossTable   = gBiomeSwampBoss,
        .bossCount   = ARRAY_COUNT(gBiomeSwampBoss),
        .rewardTable = gBiomeSwampReward,
        .rewardCount = ARRAY_COUNT(gBiomeSwampReward),
        .nextBiomes  = {BIOME_RUINS, BIOME_OCEAN, 0, 0},
        .nextWeights = {60, 40, 0, 0},
        .nextCount   = 2,
        .minWave     = 9,
    },

    // BIOME_FACTORY
    [BIOME_FACTORY] =
    {
        .spawnTable  = gBiomeFactorySpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeFactorySpawn),
        .bossTable   = gBiomeFactoryBoss,
        .bossCount   = ARRAY_COUNT(gBiomeFactoryBoss),
        .rewardTable = gBiomeFactoryReward,
        .rewardCount = ARRAY_COUNT(gBiomeFactoryReward),
        .nextBiomes  = {BIOME_SPACE, BIOME_RUINS, 0, 0},
        .nextWeights = {65, 35, 0, 0},
        .nextCount   = 2,
        .minWave     = 15,
    },

    // BIOME_RUINS
    [BIOME_RUINS] =
    {
        .spawnTable  = gBiomeRuinsSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeRuinsSpawn),
        .bossTable   = gBiomeRuinsBoss,
        .bossCount   = ARRAY_COUNT(gBiomeRuinsBoss),
        .rewardTable = gBiomeRuinsReward,
        .rewardCount = ARRAY_COUNT(gBiomeRuinsReward),
        .nextBiomes  = {BIOME_SPACE, BIOME_FINAL, 0, 0},
        .nextWeights = {70, 30, 0, 0},
        .nextCount   = 2,
        .minWave     = 18,
    },

    // BIOME_SPACE
    [BIOME_SPACE] =
    {
        .spawnTable  = gBiomeSpaceSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeSpaceSpawn),
        .bossTable   = gBiomeSpaceBoss,
        .bossCount   = ARRAY_COUNT(gBiomeSpaceBoss),
        .rewardTable = gBiomeSpaceReward,
        .rewardCount = ARRAY_COUNT(gBiomeSpaceReward),
        .nextBiomes  = {BIOME_FINAL, 0, 0, 0},
        .nextWeights = {100, 0, 0, 0},
        .nextCount   = 1,
        .minWave     = 25,
    },

    // BIOME_FINAL
    [BIOME_FINAL] =
    {
        .spawnTable  = gBiomeFinalSpawn,
        .spawnCount  = ARRAY_COUNT(gBiomeFinalSpawn),
        .bossTable   = gBiomeFinalBoss,
        .bossCount   = ARRAY_COUNT(gBiomeFinalBoss),
        .rewardTable = gBiomeFinalReward,
        .rewardCount = ARRAY_COUNT(gBiomeFinalReward),
        .nextBiomes  = {0, 0, 0, 0},
        .nextWeights = {0, 0, 0, 0},
        .nextCount   = 0,
        .minWave     = 40,
    },
};

// ============================================================
//  초기화
// ============================================================
void RogueBiome_Init(void)
{
    // 현재는 데이터 유효성 체크만 수행
    // (추후 바이옴별 BGM 프리로드 등 추가 가능)
}

// ============================================================
//  다음 바이옴 선택
// ============================================================
RogueBiome RogueBiome_SelectNext(RogueBiome current, u32 seed)
{
    const struct RogueBiomeDef *def = &gBiomeDefs[current];
    u8  i, total = 0, roll;
    u8  candidateCount = 0;
    u8  candidates[4];
    u8  candWeights[4];

    // 해금 여부 필터링
    for (i = 0; i < def->nextCount; i++)
    {
        RogueBiome next = (RogueBiome)def->nextBiomes[i];
        if (next < BIOME_COUNT && RogueBiome_IsUnlocked(next)
            && gBiomeDefs[next].minWave <= gRogueRun.currentWave)
        {
            candidates[candidateCount]  = def->nextBiomes[i];
            candWeights[candidateCount] = def->nextWeights[i];
            total += def->nextWeights[i];
            candidateCount++;
        }
    }

    if (candidateCount == 0)
        return BIOME_GRASS;  // 폴백

    roll = (u8)((seed >> 8) % total);
    total = 0;
    for (i = 0; i < candidateCount; i++)
    {
        total += candWeights[i];
        if (roll < total)
            return (RogueBiome)candidates[i];
    }
    return (RogueBiome)candidates[candidateCount - 1];
}

// ============================================================
//  스폰 종 결정 (가중치 기반 룰렛)
// ============================================================
u16 RogueBiome_RollSpecies(RogueBiome biome, u32 seed)
{
    const struct RogueBiomeDef *def = &gBiomeDefs[biome];
    u16 total = 0, roll;
    u8  i;

    if (def->spawnCount == 0)
        return SPECIES_NONE;

    for (i = 0; i < def->spawnCount; i++)
        total += def->spawnTable[i].weight;

    if (total == 0)
        return SPECIES_NONE;

    roll = (u16)(seed % total);
    total = 0;

    for (i = 0; i < def->spawnCount; i++)
    {
        total += def->spawnTable[i].weight;
        if (roll < total)
            return def->spawnTable[i].species;
    }
    return def->spawnTable[def->spawnCount - 1].species;
}

// ============================================================
//  레벨 결정 (웨이브 스케일링 포함)
// ============================================================
u8 RogueBiome_RollLevel(RogueBiome biome, u16 species, u8 wave, u32 seed)
{
    const struct RogueBiomeDef *def = &gBiomeDefs[biome];
    u8  i, minLv = 5, maxLv = 10;
    u8  spread;

    // species에 맞는 스폰 엔트리 찾기
    for (i = 0; i < def->spawnCount; i++)
    {
        if (def->spawnTable[i].species == species)
        {
            minLv = def->spawnTable[i].minLevel;
            maxLv = def->spawnTable[i].maxLevel;
            break;
        }
    }

    // 웨이브 스케일링: 웨이브 1당 +0.5레벨
    minLv = RogueBiome_ScaleLevel(minLv, maxLv, wave);
    maxLv = minLv + (maxLv - def->spawnTable[i].minLevel);
    if (maxLv > 100) maxLv = 100;

    spread = maxLv - minLv;
    if (spread == 0)
        return minLv;

    return minLv + (u8)((seed & 0xFF) % (spread + 1));
}

// ============================================================
//  보스 선택
// ============================================================
const struct RogueBossEntry *RogueBiome_GetBoss(RogueBiome biome, u8 wave, u32 seed)
{
    const struct RogueBiomeDef *def = &gBiomeDefs[biome];
    u8 idx;

    if (def->bossCount == 0)
        return NULL;

    // 웨이브 기반 보스 인덱스 결정 (후반일수록 강한 보스)
    idx = (u8)(((u32)wave * 7 + (seed & 0xF)) % def->bossCount);
    return &def->bossTable[idx];
}

// ============================================================
//  바이옴 보상 아이템 결정
// ============================================================
u16 RogueBiome_RollReward(RogueBiome biome, u32 seed)
{
    const struct RogueBiomeDef *def = &gBiomeDefs[biome];
    u16 total = 0, roll;
    u8  i;

    if (def->rewardCount == 0)
        return ITEM_NONE;

    for (i = 0; i < def->rewardCount; i++)
        total += def->rewardTable[i].weight;

    roll = (u16)((seed >> 4) % total);
    total = 0;

    for (i = 0; i < def->rewardCount; i++)
    {
        total += def->rewardTable[i].weight;
        if (roll < total)
            return def->rewardTable[i].itemId;
    }
    return def->rewardTable[def->rewardCount - 1].itemId;
}

// ============================================================
//  해금 여부 체크
// ============================================================
bool8 RogueBiome_IsUnlocked(RogueBiome biome)
{
    if (biome >= BIOME_COUNT)
        return FALSE;
    // BIOME_TOWN, BIOME_GRASS는 항상 해금
    if (biome == BIOME_TOWN || biome == BIOME_GRASS)
        return TRUE;
    return (bool8)((gRogueMeta.unlockedBiomes >> biome) & 1);
}

// ============================================================
//  내부 유틸: 가중치 기반 랜덤
// ============================================================
u32 RogueBiome_WeightedRandom(const u8 *weights, u8 count, u32 seed)
{
    u16 total = 0, roll;
    u8  i;

    for (i = 0; i < count; i++)
        total += weights[i];
    if (total == 0) return 0;

    roll = (u16)(seed % total);
    total = 0;

    for (i = 0; i < count; i++)
    {
        total += weights[i];
        if (roll < total)
            return i;
    }
    return count - 1;
}

// ============================================================
//  레벨 스케일링: wave 기반 baseMin 보정
// ============================================================
u8 RogueBiome_ScaleLevel(u8 baseMin, u8 baseMax, u8 wave)
{
    u8 bonus;
    (void)baseMax;
    // 5웨이브마다 +3 레벨
    bonus = (wave / 5) * 3;
    if (baseMin + bonus > 100)
        return 100;
    return baseMin + bonus;
}
