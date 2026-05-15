#ifndef ROGUE_DEFINES_H
#define ROGUE_DEFINES_H

// ============================================================
//  rogue_defines.h
//  PokéRogue GBA - 핵심 상수 / 열거형
//  pokeemerald-expansion 실제 API 기준으로 작성
// ============================================================

// ── 런 파라미터 ──────────────────────────────────────────────
#define ROGUE_MAX_TEAM_SIZE       6
#define ROGUE_MAX_PASSIVES       12   // 메모리 절약
#define ROGUE_MAX_ITEMS          10
#define ROGUE_MAX_NODES          14   // 6레이어 × 최대 3노드
#define ROGUE_MAX_NODE_CHILDREN   3
#define ROGUE_NODE_LAYERS         6
#define ROGUE_MAX_REWARD_CHOICES  3
#define ROGUE_MAX_SHOP_ITEMS      6
#define ROGUE_BOSS_WAVE_INTERVAL 10
#define ROGUE_MAX_WAVES          50
#define ROGUE_STARTER_POOL_SIZE  28

// ── 메타 파라미터 ─────────────────────────────────────────────
#define ROGUE_META_SAVE_MAGIC    0x524F4755u  // "ROGU"
#define ROGUE_META_VERSION       1u

// ── 바이옴 ID ─────────────────────────────────────────────────
typedef enum RogueBiome
{
    BIOME_TOWN = 0,
    BIOME_GRASS,
    BIOME_FOREST,
    BIOME_CAVE,
    BIOME_MOUNTAIN,
    BIOME_OCEAN,
    BIOME_BEACH,
    BIOME_DESERT,
    BIOME_VOLCANO,
    BIOME_ICE,
    BIOME_SWAMP,
    BIOME_FACTORY,
    BIOME_RUINS,
    BIOME_SPACE,
    BIOME_FINAL,
    BIOME_COUNT
} RogueBiome;

// ── 게임 상태 ─────────────────────────────────────────────────
typedef enum RogueState
{
    ROGUE_STATE_TITLE = 0,
    ROGUE_STATE_STARTER_SELECT,
    ROGUE_STATE_NODE_MAP,
    ROGUE_STATE_BATTLE_INIT,
    ROGUE_STATE_BATTLE,
    ROGUE_STATE_VICTORY,
    ROGUE_STATE_REWARD,
    ROGUE_STATE_EVENT,
    ROGUE_STATE_SHOP,
    ROGUE_STATE_HEAL,
    ROGUE_STATE_BOSS_INTRO,
    ROGUE_STATE_BOSS,
    ROGUE_STATE_RUN_END,
    ROGUE_STATE_META,
    ROGUE_STATE_COUNT
} RogueState;

// ── 노드 타입 ─────────────────────────────────────────────────
typedef enum RogueNodeType
{
    NODE_BATTLE = 0,
    NODE_ELITE,
    NODE_BOSS,
    NODE_SHOP,
    NODE_HEAL,
    NODE_EVENT,
    NODE_TREASURE,
    NODE_COUNT
} RogueNodeType;

// ── 보상 타입 ─────────────────────────────────────────────────
typedef enum RogueRewardType
{
    REWARD_ITEM = 0,
    REWARD_PASSIVE,
    REWARD_MONEY,
    REWARD_POKEMON,
    REWARD_TM,
    REWARD_COUNT
} RogueRewardType;

// ── 패시브 ID ─────────────────────────────────────────────────
typedef enum RoguePassiveId
{
    PASSIVE_NONE = 0,
    PASSIVE_SPEED_BOOST,
    PASSIVE_DROUGHT,
    PASSIVE_DRIZZLE,
    PASSIVE_GUTS,
    PASSIVE_HUSTLE,
    PASSIVE_THICK_FAT,
    PASSIVE_REGENERATOR,
    PASSIVE_MULTISCALE,
    PASSIVE_ADAPTABILITY,
    PASSIVE_SHEER_FORCE,
    PASSIVE_TINTED_LENS,
    PASSIVE_MOXIE,
    PASSIVE_INTIMIDATE,
    PASSIVE_ID_COUNT
} RoguePassiveId;

// ── 이벤트 타입 ───────────────────────────────────────────────
typedef enum RogueEventType
{
    EVENT_MYSTERY_ITEM = 0,
    EVENT_SHRINE,
    EVENT_MOVE_TUTOR,
    EVENT_COUNT
} RogueEventType;

// ── 영구 업그레이드 ───────────────────────────────────────────
typedef enum RoguePermUpgrade
{
    PERM_UPGRADE_STARTER_LEVEL = 0,
    PERM_UPGRADE_SHOP_DISCOUNT,
    PERM_UPGRADE_HEAL_BONUS,
    PERM_UPGRADE_PASSIVE_SLOT,
    PERM_UPGRADE_COUNT
} RoguePermUpgrade;

// ── UI 윈도우 상수 ────────────────────────────────────────────
#define ROGUE_WIN_MSG      0   // 메시지 윈도우
#define ROGUE_WIN_INFO     1   // 정보 패널
#define ROGUE_WIN_COUNT    2

// 텍스트 색상 (pokeemerald 표준)
// u8 color[3] = {배경, 전경, 그림자}
#define ROGUE_COLOR_WHITE   {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE,     TEXT_COLOR_DARK_GRAY}
#define ROGUE_COLOR_YELLOW  {TEXT_COLOR_TRANSPARENT, TEXT_DYNAMIC_COLOR_1, TEXT_COLOR_DARK_GRAY}
#define ROGUE_COLOR_GRAY    {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_TRANSPARENT}

#endif // ROGUE_DEFINES_H
