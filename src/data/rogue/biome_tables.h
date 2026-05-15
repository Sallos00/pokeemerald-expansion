// ============================================================
//  biome_tables.h
//  PokéRogue GBA - 바이옴 데이터 테이블
//
//  이 파일은 두 가지 방식으로 채워집니다:
//    1. 수동 작성 (기본 데이터)
//    2. tools/pokerogue_import/import_biomes.py 자동 생성
//
//  Python 스크립트 실행:
//    python3 tools/pokerogue_import/import_biomes.py \
//        --input path/to/pokerogue/src/data/biomes/ \
//        --output src/data/rogue/biome_tables.h
// ============================================================

#ifndef BIOME_TABLES_H
#define BIOME_TABLES_H

#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "rogue/rogue_biome.h"

// ============================================================
//  BIOME_TOWN
// ============================================================
static const struct RogueSpawnEntry gBiomeTownSpawn[] =
{
    { SPECIES_PIDGEY,      50, 3, 6,  0, 0, 0 },
    { SPECIES_RATTATA,     50, 3, 6,  0, 0, 0 },
    { SPECIES_MEOWTH,      30, 4, 7,  0, 0, 0 },
    { SPECIES_ABRA,        15, 5, 8,  0, 0, 0 },
    { SPECIES_MAGIKARP,    20, 3, 5,  0, 0, 0 },
};

static const struct RogueBossEntry gBiomeTownBoss[] =
{
    {
        .species  = SPECIES_PIDGEOT,
        .level    = 20,
        .moves    = { MOVE_WING_ATTACK, MOVE_QUICK_ATTACK, MOVE_SAND_ATTACK, MOVE_GUST },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_JOLLY,
    },
    {
        .species  = SPECIES_RATICATE,
        .level    = 22,
        .moves    = { MOVE_HYPER_FANG, MOVE_QUICK_ATTACK, MOVE_SCARY_FACE, MOVE_TACKLE },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_ADAMANT,
    },
};

static const struct RogueBiomeReward gBiomeTownReward[] =
{
    { ITEM_POTION,        40, 0 },
    { ITEM_ANTIDOTE,      20, 0 },
    { ITEM_POKE_BALL,      30, 0 },
    { ITEM_REPEL,         10, 0 },
};

// ============================================================
//  BIOME_GRASS
// ============================================================
static const struct RogueSpawnEntry gBiomeGrassSpawn[] =
{
    { SPECIES_BULBASAUR,   10, 5,  8, 0, 0, 0 },
    { SPECIES_ODDISH,      40, 4,  7, 0, 0, 0 },
    { SPECIES_BELLSPROUT,  35, 4,  7, 0, 0, 0 },
    { SPECIES_CATERPIE,    45, 3,  6, 0, 0, 0 },
    { SPECIES_WEEDLE,      45, 3,  6, 0, 0, 0 },
    { SPECIES_PARAS,       30, 5,  8, 0, 0, 0 },
    { SPECIES_VENONAT,     25, 5,  8, 0, 0, 0 },
    { SPECIES_TANGELA,     20, 6, 10, 0, 0, 0 },
    { SPECIES_SUNKERN,     15, 5,  9, 0, 0, 0 },
    { SPECIES_HOPPIP,      25, 4,  7, 0, 0, 0 },
    { SPECIES_CHIKORITA,   10, 5,  8, 0, 0, 0 },
    { SPECIES_TREECKO,     10, 5,  8, 0, 0, 0 },
    { SPECIES_TURTWIG,     10, 5,  8, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeGrassBoss[] =
{
    {
        .species  = SPECIES_VENUSAUR,
        .level    = 30,
        .moves    = { MOVE_SOLAR_BEAM, MOVE_SLEEP_POWDER, MOVE_LEECH_SEED, MOVE_SYNTHESIS },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_BOLD,
    },
    {
        .species  = SPECIES_VICTREEBEL,
        .level    = 28,
        .moves    = { MOVE_RAZOR_LEAF, MOVE_ACID, MOVE_SLEEP_POWDER, MOVE_WRAP },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_MODEST,
    },
    {
        .species  = SPECIES_EXEGGUTOR,
        .level    = 32,
        .moves    = { MOVE_PSYCHIC, MOVE_SOLAR_BEAM, MOVE_EGG_BOMB, MOVE_SLEEP_POWDER },
        .heldItem = ITEM_TWISTED_SPOON,
        .nature   = NATURE_QUIET,
    },
};

static const struct RogueBiomeReward gBiomeGrassReward[] =
{
    { ITEM_MIRACLE_SEED,   25, 0 },
    { ITEM_SUPER_POTION,   30, 0 },
    { ITEM_GREAT_BALL,     25, 0 },
    { ITEM_REVIVE,         10, 0 },
    { ITEM_AWAKENING,      10, 0 },
};

// ============================================================
//  BIOME_FOREST
// ============================================================
static const struct RogueSpawnEntry gBiomeForestSpawn[] =
{
    { SPECIES_SCYTHER,      15, 8, 12, 0, 0, 0 },
    { SPECIES_PINSIR,       12, 9, 13, 0, 0, 0 },
    { SPECIES_HERACROSS,    10, 9, 13, 0, 0, 0 },
    { SPECIES_YANMA,        20, 7, 11, 0, 0, 0 },
    { SPECIES_BEEDRILL,     30, 6, 10, 0, 0, 0 },
    { SPECIES_BUTTERFREE,   30, 6, 10, 0, 0, 0 },
    { SPECIES_NUZLEAF,      25, 7, 11, 0, 0, 0 },
    { SPECIES_WURMPLE,      40, 5,  8, 0, 0, 0 },
    { SPECIES_SEEDOT,       35, 6, 10, 0, 0, 0 },
    { SPECIES_SHROOMISH,    25, 7, 11, 0, 0, 0 },
    { SPECIES_SURSKIT,      20, 6, 10, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeForestBoss[] =
{
    {
        .species  = SPECIES_SCIZOR,
        .level    = 38,
        .moves    = { MOVE_BULLET_PUNCH, MOVE_X_SCISSOR, MOVE_SWORDS_DANCE, MOVE_ROOST },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_ADAMANT,
    },
    {
        .species  = SPECIES_HERACROSS,
        .level    = 36,
        .moves    = { MOVE_MEGAHORN, MOVE_CLOSE_COMBAT, MOVE_STONE_EDGE, MOVE_FACADE },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_JOLLY,
    },
};

static const struct RogueBiomeReward gBiomeForestReward[] =
{
    { ITEM_SHARP_BEAK,     20, 0 },
    { ITEM_SILVER_POWDER,  20, 0 },
    { ITEM_SUPER_POTION,   25, 0 },
    { ITEM_GREAT_BALL,     25, 0 },
    { ITEM_FULL_HEAL,      10, 0 },
};

// ============================================================
//  BIOME_CAVE
// ============================================================
static const struct RogueSpawnEntry gBiomeCaveSpawn[] =
{
    { SPECIES_GEODUDE,     45, 8, 13, 0, 0, 0 },
    { SPECIES_ONIX,        20, 9, 14, 0, 0, 0 },
    { SPECIES_ZUBAT,       50, 7, 12, 0, 0, 0 },
    { SPECIES_DIGLETT,     35, 8, 13, 0, 0, 0 },
    { SPECIES_LARVITAR,    10,10, 14, 0, 0, 0 },
    { SPECIES_ARON,        20, 9, 13, 0, 0, 0 },
    { SPECIES_SABLEYE,     15,10, 14, 0, 0, 0 },
    { SPECIES_BALTOY,      20, 9, 13, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeCaveBoss[] =
{
    {
        .species  = SPECIES_TYRANITAR,
        .level    = 45,
        .moves    = { MOVE_CRUNCH, MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_DRAGON_DANCE },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_JOLLY,
    },
    {
        .species  = SPECIES_RHYDON,
        .level    = 42,
        .moves    = { MOVE_EARTHQUAKE, MOVE_ROCK_BLAST, MOVE_HORN_DRILL, MOVE_MEGAHORN },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_ADAMANT,
    },
    {
        .species  = SPECIES_CROBAT,
        .level    = 40,
        .moves    = { MOVE_CROSS_POISON, MOVE_AIR_SLASH, MOVE_CONFUSE_RAY, MOVE_HAZE },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_JOLLY,
    },
};

static const struct RogueBiomeReward gBiomeCaveReward[] =
{
    { ITEM_HARD_STONE,     25, 0 },
    { ITEM_NEVER_MELT_ICE, 10, 0 },
    { ITEM_HYPER_POTION,   25, 0 },
    { ITEM_ESCAPE_ROPE,    15, 0 },
    { ITEM_ULTRA_BALL,     15, 0 },
    { ITEM_REVIVE,         10, 0 },
};

// ============================================================
//  BIOME_MOUNTAIN
// ============================================================
static const struct RogueSpawnEntry gBiomeMountainSpawn[] =
{
    { SPECIES_MACHOP,      35, 10, 15, 0, 0, 0 },
    { SPECIES_MANKEY,      35, 10, 15, 0, 0, 0 },
    { SPECIES_GEODUDE,     30, 12, 16, 0, 0, 0 },
    { SPECIES_NOSEPASS,    20, 12, 16, 0, 0, 0 },
    { SPECIES_MEDITITE,    20, 11, 15, 0, 0, 0 },
    { SPECIES_SKARMORY,    15, 13, 17, 0, 0, 0 },
    { SPECIES_BAGON,       10, 12, 16, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeMountainBoss[] =
{
    {
        .species  = SPECIES_MACHAMP,
        .level    = 48,
        .moves    = { MOVE_DYNAMIC_PUNCH, MOVE_CROSS_CHOP, MOVE_BULLET_PUNCH, MOVE_EARTHQUAKE },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_ADAMANT,
    },
    {
        .species  = SPECIES_SKARMORY,
        .level    = 46,
        .moves    = { MOVE_BRAVE_BIRD, MOVE_STEEL_WING, MOVE_SPIKES, MOVE_ROOST },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_IMPISH,
    },
};

static const struct RogueBiomeReward gBiomeMountainReward[] =
{
    { ITEM_BLACK_BELT,     20, 0 },
    { ITEM_HYPER_POTION,   25, 0 },
    { ITEM_ULTRA_BALL,     20, 0 },
    { ITEM_X_ATTACK,       15, 0 },
    { ITEM_X_DEFEND,       10, 0 },
    { ITEM_MAX_REPEL,      10, 0 },
};

// ============================================================
//  BIOME_OCEAN
// ============================================================
static const struct RogueSpawnEntry gBiomeOceanSpawn[] =
{
    { SPECIES_TENTACOOL,   50, 12, 18, 0, 0, 0 },
    { SPECIES_HORSEA,      30, 12, 17, 0, 0, 0 },
    { SPECIES_SHELLDER,    30, 12, 17, 0, 0, 0 },
    { SPECIES_STARYU,      25, 13, 18, 0, 0, 0 },
    { SPECIES_GYARADOS,    10, 15, 20, 0, 0, 0 },
    { SPECIES_LAPRAS,       8, 16, 21, 0, 0, 0 },
    { SPECIES_DEWGONG,     20, 14, 19, 0, 0, 0 },
    { SPECIES_MANTINE,     15, 14, 19, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeOceanBoss[] =
{
    {
        .species  = SPECIES_GYARADOS,
        .level    = 52,
        .moves    = { MOVE_WATERFALL, MOVE_DRAGON_DANCE, MOVE_CRUNCH, MOVE_ICE_FANG },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_ADAMANT,
    },
    {
        .species  = SPECIES_STARMIE,
        .level    = 50,
        .moves    = { MOVE_SURF, MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_ICE_BEAM },
        .heldItem = ITEM_TWISTED_SPOON,
        .nature   = NATURE_TIMID,
    },
};

static const struct RogueBiomeReward gBiomeOceanReward[] =
{
    { ITEM_MYSTIC_WATER,   25, 0 },
    { ITEM_MAX_POTION,     20, 0 },
    { ITEM_ULTRA_BALL,     20, 0 },
    { ITEM_FULL_RESTORE,   10, 0 },
    { ITEM_KINGS_ROCK,     10, 0 },
    { ITEM_LURE,           15, 0 },
};

// ============================================================
//  BIOME_BEACH (단축)
// ============================================================
static const struct RogueSpawnEntry gBiomeBeachSpawn[] =
{
    { SPECIES_KRABBY,      45, 9, 14, 0, 0, 0 },
    { SPECIES_CORSOLA,     25, 10, 15, 0, 0, 0 },
    { SPECIES_WINGULL,     40, 8, 13, 0, 0, 0 },
    { SPECIES_CLAMPERL,    20, 10, 15, 0, 0, 0 },
    { SPECIES_CHINCHOU,    25, 10, 15, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeBeachBoss[] =
{
    {
        .species  = SPECIES_KINGLER,
        .level    = 44,
        .moves    = { MOVE_CRABHAMMER, MOVE_GUILLOTINE, MOVE_METAL_CLAW, MOVE_PROTECT },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_ADAMANT,
    },
};

static const struct RogueBiomeReward gBiomeBeachReward[] =
{
    { ITEM_MYSTIC_WATER, 30, 0 },
    { ITEM_SUPER_POTION, 30, 0 },
    { ITEM_GREAT_BALL,   25, 0 },
    { ITEM_ANTIDOTE,     15, 0 },
};

// ============================================================
//  BIOME_DESERT
// ============================================================
static const struct RogueSpawnEntry gBiomeDesertSpawn[] =
{
    { SPECIES_SANDSHREW,   40, 12, 17, 0, 0, 0 },
    { SPECIES_EKANS,       35, 12, 17, 0, 0, 0 },
    { SPECIES_TRAPINCH,    25, 13, 18, 0, 0, 0 },
    { SPECIES_CACNEA,      25, 12, 17, 0, 0, 0 },
    { SPECIES_NUMEL,       20, 13, 18, 0, 0, 0 },
    { SPECIES_HIPPOPOTAS,  20, 13, 18, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeDesertBoss[] =
{
    {
        .species  = SPECIES_FLYGON,
        .level    = 52,
        .moves    = { MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW, MOVE_CRUNCH, MOVE_STONE_EDGE },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_JOLLY,
    },
    {
        .species  = SPECIES_GARCHOMP,
        .level    = 55,
        .moves    = { MOVE_EARTHQUAKE, MOVE_DRAGON_RUSH, MOVE_CRUNCH, MOVE_FIRE_FANG },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_JOLLY,
    },
};

static const struct RogueBiomeReward gBiomeDesertReward[] =
{
    { ITEM_SOFT_SAND,      25, 0 },
    { ITEM_HYPER_POTION,   20, 0 },
    { ITEM_ULTRA_BALL,     20, 0 },
    { ITEM_MAX_REPEL,      15, 0 },
    { ITEM_REVIVE,         10, 0 },
    { ITEM_STARDUST,       10, 0 },
};

// ============================================================
//  BIOME_VOLCANO
// ============================================================
static const struct RogueSpawnEntry gBiomeVolcanoSpawn[] =
{
    { SPECIES_CHARMANDER,   8, 15, 20, 0, 0, 0 },
    { SPECIES_VULPIX,      25, 14, 19, 0, 0, 0 },
    { SPECIES_GROWLITHE,   25, 14, 19, 0, 0, 0 },
    { SPECIES_MAGMAR,      20, 15, 20, 0, 0, 0 },
    { SPECIES_SLUGMA,      30, 14, 19, 0, 0, 0 },
    { SPECIES_NUMEL,       25, 14, 19, 0, 0, 0 },
    { SPECIES_HOUNDOUR,    20, 15, 20, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeVolcanoBoss[] =
{
    {
        .species  = SPECIES_CHARIZARD,
        .level    = 55,
        .moves    = { MOVE_FLAMETHROWER, MOVE_AIR_SLASH, MOVE_DRAGON_PULSE, MOVE_SOLAR_BEAM },
        .heldItem = ITEM_CHARCOAL,
        .nature   = NATURE_MODEST,
    },
    {
        .species  = SPECIES_MAGMORTAR,
        .level    = 54,
        .moves    = { MOVE_FIRE_BLAST, MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST, MOVE_HYPER_BEAM },
        .heldItem = ITEM_CHARCOAL,
        .nature   = NATURE_MODEST,
    },
    {
        .species  = SPECIES_HOUNDOOM,
        .level    = 52,
        .moves    = { MOVE_DARK_PULSE, MOVE_FLAMETHROWER, MOVE_NASTY_PLOT, MOVE_WILL_O_WISP },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_TIMID,
    },
};

static const struct RogueBiomeReward gBiomeVolcanoReward[] =
{
    { ITEM_CHARCOAL,       20, 0 },
    { ITEM_FIRE_STONE,     15, 0 },
    { ITEM_MAX_POTION,     25, 0 },
    { ITEM_FULL_RESTORE,   15, 0 },
    { ITEM_ULTRA_BALL,     15, 0 },
    { ITEM_BURN_HEAL,     10, 0 },
};

// ============================================================
//  BIOME_ICE
// ============================================================
static const struct RogueSpawnEntry gBiomeIceSpawn[] =
{
    { SPECIES_JYNX,        20, 16, 22, 0, 0, 0 },
    { SPECIES_SNEASEL,     20, 16, 22, 0, 0, 0 },
    { SPECIES_SWINUB,      35, 14, 20, 0, 0, 0 },
    { SPECIES_DELIBIRD,    25, 14, 20, 0, 0, 0 },
    { SPECIES_SNORUNT,     30, 14, 20, 0, 0, 0 },
    { SPECIES_SPHEAL,      25, 14, 20, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeIceBoss[] =
{
    {
        .species  = SPECIES_MAMOSWINE,
        .level    = 55,
        .moves    = { MOVE_EARTHQUAKE, MOVE_BLIZZARD, MOVE_ICE_SHARD, MOVE_STONE_EDGE },
        .heldItem = ITEM_NEVER_MELT_ICE,
        .nature   = NATURE_JOLLY,
    },
    {
        .species  = SPECIES_WALREIN,
        .level    = 54,
        .moves    = { MOVE_SURF, MOVE_BLIZZARD, MOVE_BODY_SLAM, MOVE_SHEER_COLD },
        .heldItem = ITEM_NEVER_MELT_ICE,
        .nature   = NATURE_BOLD,
    },
};

static const struct RogueBiomeReward gBiomeIceReward[] =
{
    { ITEM_NEVER_MELT_ICE, 20, 0 },
    { ITEM_ICE_HEAL,       15, 0 },
    { ITEM_MAX_POTION,     25, 0 },
    { ITEM_FULL_RESTORE,   15, 0 },
    { ITEM_ULTRA_BALL,     15, 0 },
    { ITEM_NEVER_MELT_ICE, 10, 0 },
};

// ============================================================
//  BIOME_SWAMP
// ============================================================
static const struct RogueSpawnEntry gBiomeSwampSpawn[] =
{
    { SPECIES_POLIWAG,     35, 12, 17, 0, 0, 0 },
    { SPECIES_WOOPER,      35, 12, 17, 0, 0, 0 },
    { SPECIES_LOTAD,       30, 11, 16, 0, 0, 0 },
    { SPECIES_CROAGUNK,    20, 13, 18, 0, 0, 0 },
    { SPECIES_GASTLY,      20, 13, 18, 0, 0, 0 },
    { SPECIES_POLITOED,    10, 15, 20, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeSwampBoss[] =
{
    {
        .species  = SPECIES_QUAGSIRE,
        .level    = 48,
        .moves    = { MOVE_EARTHQUAKE, MOVE_SURF, MOVE_AMNESIA, MOVE_RECOVER },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_RELAXED,
    },
};

static const struct RogueBiomeReward gBiomeSwampReward[] =
{
    { ITEM_MYSTIC_WATER, 25, 0 },
    { ITEM_ANTIDOTE,     20, 0 },
    { ITEM_HYPER_POTION, 25, 0 },
    { ITEM_ULTRA_BALL,   15, 0 },
    { ITEM_FULL_HEAL,    15, 0 },
};

// ============================================================
//  BIOME_FACTORY
// ============================================================
static const struct RogueSpawnEntry gBiomeFactorySpawn[] =
{
    { SPECIES_MAGNEMITE,   35, 18, 24, 0, 0, 0 },
    { SPECIES_VOLTORB,     30, 18, 24, 0, 0, 0 },
    { SPECIES_KOFFING,     25, 18, 24, 0, 0, 0 },
    { SPECIES_PORYGON,     15, 20, 26, 0, 0, 0 },
    { SPECIES_BRONZOR,     25, 18, 24, 0, 0, 0 },
    { SPECIES_ROTOM,       10, 22, 28, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeFactoryBoss[] =
{
    {
        .species  = SPECIES_MAGNEZONE,
        .level    = 60,
        .moves    = { MOVE_THUNDERBOLT, MOVE_FLASH_CANNON, MOVE_TRI_ATTACK, MOVE_MAGNET_RISE },
        .heldItem = ITEM_MAGNET,
        .nature   = NATURE_MODEST,
    },
    {
        .species  = SPECIES_PORYGON_Z,
        .level    = 62,
        .moves    = { MOVE_TRI_ATTACK, MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_NASTY_PLOT },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_TIMID,
    },
};

static const struct RogueBiomeReward gBiomeFactoryReward[] =
{
    { ITEM_MAGNET,         20, 0 },
    { ITEM_METAL_COAT,     15, 0 },
    { ITEM_MAX_POTION,     20, 0 },
    { ITEM_FULL_RESTORE,   20, 0 },
    { ITEM_ULTRA_BALL,     15, 0 },
    { ITEM_RARE_CANDY,     10, 0 },
};

// ============================================================
//  BIOME_RUINS
// ============================================================
static const struct RogueSpawnEntry gBiomeRuinsSpawn[] =
{
    { SPECIES_UNOWN,       30, 20, 28, 0, 0, 0 },
    { SPECIES_NATU,        25, 20, 27, 0, 0, 0 },
    { SPECIES_SIGILYPH,    15, 22, 29, 0, 0, 0 },
    { SPECIES_CLAYDOL,     20, 22, 28, 0, 0, 0 },
    { SPECIES_SOLROCK,     20, 20, 27, 0, 0, 0 },
    { SPECIES_LUNATONE,    20, 20, 27, 0, 0, 0 },
    { SPECIES_YAMASK,      15, 22, 28, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeRuinsBoss[] =
{
    {
        .species  = SPECIES_ALAKAZAM,
        .level    = 62,
        .moves    = { MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL, MOVE_CALM_MIND },
        .heldItem = ITEM_TWISTED_SPOON,
        .nature   = NATURE_TIMID,
    },
    {
        .species  = SPECIES_COFAGRIGUS,
        .level    = 60,
        .moves    = { MOVE_SHADOW_BALL, MOVE_WILL_O_WISP, MOVE_TOXIC, MOVE_PROTECT },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_BOLD,
    },
};

static const struct RogueBiomeReward gBiomeRuinsReward[] =
{
    { ITEM_TWISTED_SPOON,  20, 0 },
    { ITEM_ODD_INCENSE,    15, 0 },
    { ITEM_MAX_POTION,     20, 0 },
    { ITEM_FULL_RESTORE,   15, 0 },
    { ITEM_RARE_CANDY,     15, 0 },
    { ITEM_PP_UP,          15, 0 },
};

// ============================================================
//  BIOME_SPACE
// ============================================================
static const struct RogueSpawnEntry gBiomeSpaceSpawn[] =
{
    { SPECIES_CLEFAIRY,    20, 28, 35, 0, 0, 0 },
    { SPECIES_STARMIE,     15, 30, 36, 0, 0, 0 },
    { SPECIES_JOLTEON,     12, 30, 36, 0, 0, 0 },
    { SPECIES_GARDEVOIR,   10, 30, 36, 0, 0, 0 },
    { SPECIES_DEOXYS,       3, 35, 40, 0, 1, 0 }, // 샤이니 잠금
    { SPECIES_JIRACHI,      3, 35, 40, 0, 1, 0 },
    { SPECIES_LUNATONE,    15, 28, 34, 0, 0, 0 },
    { SPECIES_SOLROCK,     15, 28, 34, 0, 0, 0 },
};

static const struct RogueBossEntry gBiomeSpaceBoss[] =
{
    {
        .species  = SPECIES_MEWTWO,
        .level    = 70,
        .moves    = { MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_ICE_BEAM, MOVE_CALM_MIND },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_MODEST,
    },
    {
        .species  = SPECIES_DEOXYS,
        .level    = 68,
        .form     = 1,  // Attack Forme
        .moves    = { MOVE_PSYCHO_BOOST, MOVE_SUPERPOWER, MOVE_ICE_BEAM, MOVE_THUNDERBOLT },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_HASTY,
    },
};

static const struct RogueBiomeReward gBiomeSpaceReward[] =
{
    { ITEM_MOON_STONE,     15, 0 },
    { ITEM_STARDUST,       15, 0 },
    { ITEM_MAX_REVIVE,     20, 0 },
    { ITEM_FULL_RESTORE,   20, 0 },
    { ITEM_RARE_CANDY,     15, 0 },
    { ITEM_PP_MAX,         10, 0 },
    { ITEM_MASTER_BALL,     5, 0 },
};

// ============================================================
//  BIOME_FINAL (최종 보스 전용)
// ============================================================
static const struct RogueSpawnEntry gBiomeFinalSpawn[] =
{
    { SPECIES_ARCEUS,       1, 80, 80, 0, 1, 0 },
    { SPECIES_GIRATINA,     1, 75, 80, 0, 1, 0 },
    { SPECIES_DIALGA,       1, 75, 80, 0, 1, 0 },
    { SPECIES_PALKIA,       1, 75, 80, 0, 1, 0 },
};

static const struct RogueBossEntry gBiomeFinalBoss[] =
{
    {
        .species  = SPECIES_ARCEUS,
        .level    = 100,
        .moves    = { MOVE_JUDGMENT, MOVE_EXTREMESPEED, MOVE_RECOVER, MOVE_CALM_MIND },
        .heldItem = ITEM_NONE,
        .nature   = NATURE_MODEST,
    },
};

static const struct RogueBiomeReward gBiomeFinalReward[] =
{
    { ITEM_MASTER_BALL,    50, 0 },
    { ITEM_MAX_REVIVE,     50, 0 },
};

#endif // BIOME_TABLES_H
