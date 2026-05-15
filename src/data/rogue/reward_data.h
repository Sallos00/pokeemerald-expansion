// ============================================================
//  reward_data.h
//  PokéRogue GBA - 일반 아이템 보상 풀
//  (바이옴 특화 외의 공통 보상 아이템 목록)
// ============================================================

#ifndef REWARD_DATA_H
#define REWARD_DATA_H

#include "constants/items.h"

// ── 웨이브 구간별 일반 아이템 풀 ────────────────────────────
struct GeneralRewardEntry
{
    u16 itemId;
    u8  weight;
    u8  minWave;   // 등장 최소 웨이브
};

static const struct GeneralRewardEntry gGeneralRewardPool[] =
{
    // ── 소모품 ──────────────────────────────────────────────
    { ITEM_POTION,          30,  1 },
    { ITEM_SUPER_POTION,    25,  3 },
    { ITEM_HYPER_POTION,    20,  8 },
    { ITEM_MAX_POTION,      10, 15 },
    { ITEM_FULL_RESTORE,     8, 20 },
    { ITEM_REVIVE,          20,  5 },
    { ITEM_MAX_REVIVE,       8, 20 },
    { ITEM_FULL_HEAL,       15,  3 },
    { ITEM_ANTIDOTE,        15,  1 },
    { ITEM_BURN_HEAL,       12,  1 },
    { ITEM_ICE_HEAL,        12,  1 },
    { ITEM_AWAKENING,       12,  1 },
    { ITEM_PARALYZE_HEAL,   12,  1 },

    // ── 강화 아이템 (전투 중) ───────────────────────────────
    { ITEM_X_ATTACK,        12,  5 },
    { ITEM_X_DEFEND,        12,  5 },
    { ITEM_X_SPEED,         12,  5 },
    { ITEM_X_SP_ATK,        12,  8 },
    { ITEM_X_ACCURACY,       8,  5 },
    { ITEM_DIRE_HIT,        10,  5 },
    { ITEM_GUARD_SPEC,       8, 10 },

    // ── 볼 ──────────────────────────────────────────────────
    { ITEM_GREAT_BALL,      20,  3 },
    { ITEM_ULTRA_BALL,      15, 10 },
    { ITEM_MASTER_BALL,      2, 35 },

    // ── 강화 도구 (지속) ────────────────────────────────────
    { ITEM_CHOICE_BAND,     10, 12 },
    { ITEM_CHOICE_SPECS,    10, 12 },
    { ITEM_CHOICE_SCARF,    10, 12 },
    { ITEM_LIFE_ORB,         8, 15 },
    { ITEM_FOCUS_SASH,       8, 15 },
    { ITEM_LEFTOVERS,       10, 10 },
    { ITEM_BLACK_SLUDGE,     6, 15 },
    { ITEM_ROCKY_HELMET,     8, 12 },
    { ITEM_ASSAULT_VEST,     8, 15 },
    { ITEM_EVIOLITE,         6, 10 },
    { ITEM_EXPERT_BELT,      8, 12 },
    { ITEM_WIDE_LENS,        8, 10 },
    { ITEM_ZOOM_LENS,        6, 12 },
    { ITEM_KINGS_ROCK,       6, 10 },
    { ITEM_RAZOR_FANG,       6, 10 },
    { ITEM_SHELL_BELL,       8, 12 },
    { ITEM_QUICK_CLAW,       8,  8 },
    { ITEM_BRIGHT_POWDER,    6, 10 },
    { ITEM_LAX_INCENSE,      5, 10 },
    { ITEM_MENTAL_HERB,      6, 10 },
    { ITEM_WHITE_HERB,       8,  8 },
    { ITEM_POWER_HERB,       6, 10 },
    { ITEM_ABSORB_BULB,      5, 12 },
    { ITEM_CELL_BATTERY,     5, 12 },
    { ITEM_RED_CARD,         5, 12 },
    { ITEM_EJECT_BUTTON,     5, 12 },

    // ── 타입 강화 아이템 ────────────────────────────────────
    { ITEM_CHARCOAL,         8,  5 },
    { ITEM_MYSTIC_WATER,     8,  5 },
    { ITEM_MIRACLE_SEED,     8,  5 },
    { ITEM_MAGNET,           8,  5 },
    { ITEM_TWISTED_SPOON,    8,  5 },
    { ITEM_NEVER_MELT_ICE,   8,  5 },
    { ITEM_SOFT_SAND,        8,  5 },
    { ITEM_HARD_STONE,       8,  5 },
    { ITEM_SILVER_POWDER,    8,  5 },
    { ITEM_SHARP_BEAK,       8,  5 },
    { ITEM_POISON_BARB,      8,  5 },
    { ITEM_SPELL_TAG,        8,  5 },
    { ITEM_BLACK_BELT,       8,  5 },
    { ITEM_SILK_SCARF,       8,  5 },
    { ITEM_METAL_COAT,       8,  5 },
    { ITEM_DRAGON_FANG,      8,  5 },
    { ITEM_FAIRY_FEATHER,    8,  5 },

    // ── 희귀 아이템 ─────────────────────────────────────────
    { ITEM_RARE_CANDY,       8, 10 },
    { ITEM_PP_UP,            6, 15 },
    { ITEM_PP_MAX,           3, 25 },
    { ITEM_PROTEIN,          6, 15 },
    { ITEM_IRON,             6, 15 },
    { ITEM_CALCIUM,          6, 15 },
    { ITEM_CARBOS,           6, 15 },
    { ITEM_HP_UP,            6, 15 },
    { ITEM_ZINC,             6, 15 },
};

#define GENERAL_REWARD_COUNT  ARRAY_COUNT(gGeneralRewardPool)

// ── API ───────────────────────────────────────────────────────
static inline u16 RogueReward_RollGeneralItem(u8 wave, u32 seed)
{
    u16 total = 0, roll;
    u8  i;

    // 웨이브 필터링 후 가중치 합산
    for (i = 0; i < GENERAL_REWARD_COUNT; i++)
    {
        if (gGeneralRewardPool[i].minWave <= wave)
            total += gGeneralRewardPool[i].weight;
    }
    if (total == 0)
        return ITEM_POTION;

    roll = (u16)((seed >> 4) % total);
    total = 0;

    for (i = 0; i < GENERAL_REWARD_COUNT; i++)
    {
        if (gGeneralRewardPool[i].minWave > wave)
            continue;
        total += gGeneralRewardPool[i].weight;
        if (roll < total)
            return gGeneralRewardPool[i].itemId;
    }
    return ITEM_POTION;
}

#endif // REWARD_DATA_H
