// ============================================================
//  rogue_passive.c
//  PokéRogue GBA - 패시브 어빌리티 시스템
//
//  pokeemerald 배틀 엔진 훅 포인트:
//    battle_main.c  → RoguePassive_OnBattleStart()
//    battle_util.c  → RoguePassive_BuildContext() 참조
//    battle_script_commands.c → 개별 효과 적용
// ============================================================

#include "global.h"
#include "battle.h"
#include "pokemon.h"
#include "random.h"

#include "rogue/rogue_passive.h"
#include "constants/weather.h"
#include "rogue/rogue_main.h"

// ── 패시브 정의 테이블 (ROM) ──────────────────────────────────
// 1024 = 1.0배 (고정소수점 Q10 방식)
const struct RoguePassiveDef gPassiveDefs[PASSIVE_ID_COUNT] =
{
    [PASSIVE_NONE] =
    {
        .id = PASSIVE_NONE, .maxStacks = 0, .weight = 0,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_BATTLE_START, .triggerParam = 0,
        .name = NULL, .desc = NULL,
    },

    [PASSIVE_SPEED_BOOST] =
    {
        .id = PASSIVE_SPEED_BOOST, .maxStacks = 3, .weight = 30,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_TURN_START, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_DROUGHT] =
    {
        .id = PASSIVE_DROUGHT, .maxStacks = 1, .weight = 20,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_BATTLE_START, .triggerParam = WEATHER_SUNNY,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_DRIZZLE] =
    {
        .id = PASSIVE_DRIZZLE, .maxStacks = 1, .weight = 20,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_BATTLE_START, .triggerParam = WEATHER_RAIN,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_GUTS] =
    {
        .id = PASSIVE_GUTS, .maxStacks = 1, .weight = 25,
        .atkMod = 1536, .defMod = 1024,  // 1.5배
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_ON_HIT, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_HUSTLE] =
    {
        .id = PASSIVE_HUSTLE, .maxStacks = 1, .weight = 20,
        .atkMod = 1536, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_ON_ATTACK, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_THICK_FAT] =
    {
        .id = PASSIVE_THICK_FAT, .maxStacks = 1, .weight = 25,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 512, .spdModifier = 512,  // 0.5배 받음
        .speedModifier = 1024,
        .trigger = TRIGGER_ON_HIT, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_REGENERATOR] =
    {
        .id = PASSIVE_REGENERATOR, .maxStacks = 1, .weight = 30,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_ON_SWITCH, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_MULTISCALE] =
    {
        .id = PASSIVE_MULTISCALE, .maxStacks = 1, .weight = 15,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_ON_HIT, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_ADAPTABILITY] =
    {
        .id = PASSIVE_ADAPTABILITY, .maxStacks = 1, .weight = 20,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_ON_ATTACK, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_SHEER_FORCE] =
    {
        .id = PASSIVE_SHEER_FORCE, .maxStacks = 1, .weight = 20,
        .atkMod = 1331, .defMod = 1024,  // 1.3배
        .spaModifier = 1331, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_ON_ATTACK, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_TINTED_LENS] =
    {
        .id = PASSIVE_TINTED_LENS, .maxStacks = 1, .weight = 18,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_ON_ATTACK, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_MOXIE] =
    {
        .id = PASSIVE_MOXIE, .maxStacks = 6, .weight = 22,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_ON_KO, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },

    [PASSIVE_INTIMIDATE] =
    {
        .id = PASSIVE_INTIMIDATE, .maxStacks = 1, .weight = 25,
        .atkMod = 1024, .defMod = 1024,
        .spaModifier = 1024, .spdModifier = 1024, .speedModifier = 1024,
        .trigger = TRIGGER_BATTLE_START, .triggerParam = 0,
        .name = NULL,
        .desc = NULL,
    },
};

// ============================================================
//  초기화
// ============================================================
void RoguePassive_Init(void)
{
    memset(gRogueRun.passives, 0, sizeof(gRogueRun.passives));
    gRogueRun.passiveCount = 0;
}

// ============================================================
//  패시브 추가
// ============================================================
bool8 RoguePassive_Add(RoguePassiveId id)
{
    u8 i;
    const struct RoguePassiveDef *def;

    if (id >= PASSIVE_ID_COUNT)
        return FALSE;

    def = &gPassiveDefs[id];

    // 이미 보유 중? → 스택 증가
    for (i = 0; i < gRogueRun.passiveCount; i++)
    {
        if (gRogueRun.passives[i].id == id)
        {
            if (gRogueRun.passives[i].stacks < def->maxStacks)
            {
                gRogueRun.passives[i].stacks++;
                return TRUE;
            }
            return FALSE;  // 최대 스택
        }
    }

    // 슬롯 여유 확인
    if (gRogueRun.passiveCount >= ROGUE_MAX_PASSIVES)
        return FALSE;

    // 새 슬롯 추가
    gRogueRun.passives[gRogueRun.passiveCount].id     = (u16)id;
    gRogueRun.passives[gRogueRun.passiveCount].stacks = 1;
    gRogueRun.passiveCount++;
    return TRUE;
}

// ============================================================
//  패시브 보유 여부
// ============================================================
bool8 RoguePassive_Has(RoguePassiveId id)
{
    u8 i;
    for (i = 0; i < gRogueRun.passiveCount; i++)
    {
        if (gRogueRun.passives[i].id == id)
            return TRUE;
    }
    return FALSE;
}

u8 RoguePassive_GetStacks(RoguePassiveId id)
{
    u8 i;
    for (i = 0; i < gRogueRun.passiveCount; i++)
    {
        if (gRogueRun.passives[i].id == id)
            return gRogueRun.passives[i].stacks;
    }
    return 0;
}

// ============================================================
//  배틀 컨텍스트 빌드
//  pokeemerald battle_util.c의 데미지 계산 직전에 호출됨
// ============================================================
void RoguePassive_BuildContext(struct RoguePassiveContext *ctx)
{
    u8 i;
    const struct RoguePassiveDef *def;

    // 기본값
    ctx->atkModTotal      = 1024;
    ctx->defModTotal      = 1024;
    ctx->spaModTotal      = 1024;
    ctx->spdModTotal      = 1024;
    ctx->speedModTotal    = 1024;
    ctx->weatherOverride  = 0;
    ctx->regenOnSwitch    = 0;
    ctx->dmgHalfOnFull    = 0;
    ctx->superEffOnNeutral= 0;
    ctx->boostOnKO        = 0;

    for (i = 0; i < gRogueRun.passiveCount; i++)
    {
        def = &gPassiveDefs[gRogueRun.passives[i].id];
        u8 stacks = gRogueRun.passives[i].stacks;

        // 배율 누적 (Q10 고정소수점 곱셈 → 오버플로 방지를 위해 순차 적용)
        ctx->atkModTotal   = (u16)((u32)ctx->atkModTotal * def->atkMod / 1024);
        ctx->defModTotal   = (u16)((u32)ctx->defModTotal * def->defMod / 1024);
        ctx->spaModTotal   = (u16)((u32)ctx->spaModTotal * def->spaModifier / 1024);
        ctx->spdModTotal   = (u16)((u32)ctx->spdModTotal * def->spdModifier / 1024);
        ctx->speedModTotal = (u16)((u32)ctx->speedModTotal * def->speedModifier / 1024);

        // 특수 효과
        switch (gRogueRun.passives[i].id)
        {
        case PASSIVE_DROUGHT:
        case PASSIVE_DRIZZLE:
            ctx->weatherOverride = def->triggerParam;
            break;
        case PASSIVE_REGENERATOR:
            ctx->regenOnSwitch = 1;
            break;
        case PASSIVE_MULTISCALE:
            ctx->dmgHalfOnFull = 1;
            break;
        case PASSIVE_TINTED_LENS:
            ctx->superEffOnNeutral = 1;
            break;
        case PASSIVE_MOXIE:
            ctx->boostOnKO = stacks;  // 스택 수만큼 추가 상승
            break;
        default:
            break;
        }
    }
}

// ============================================================
//  배틀 시작 훅
// ============================================================
void RoguePassive_OnBattleStart(void)
{
    u8 i;
    struct RoguePassiveContext ctx;
    RoguePassive_BuildContext(&ctx);

    // 날씨 설정
    if (ctx.weatherOverride != 0)
    {
        // pokeemerald의 날씨 변경 함수 호출
        // TryChangeBattleWeather(B_POSITION_PLAYER_LEFT, ctx.weatherOverride, FALSE);
    }

    // INTIMIDATE 효과
    if (RoguePassive_Has(PASSIVE_INTIMIDATE))
    {
        // gBattleMons[B_POSITION_OPPONENT_LEFT].statStages[STAT_ATK]--;
    }
}

// ============================================================
//  턴 시작 훅 (Speed Boost 등)
// ============================================================
void RoguePassive_OnTurnStart(void)
{
    if (RoguePassive_Has(PASSIVE_SPEED_BOOST))
    {
        // 턴마다 Speed +1 스테이지
        // gBattleMons[B_POSITION_PLAYER_LEFT].statStages[STAT_SPEED]++;
    }
}

// ============================================================
//  KO 훅 (Moxie)
// ============================================================
void RoguePassive_OnKO(void)
{
    u8 boost = RoguePassive_GetStacks(PASSIVE_MOXIE);
    if (boost > 0)
    {
        // gBattleMons[B_POSITION_PLAYER_LEFT].statStages[STAT_ATK] += boost;
    }
}

// ============================================================
//  교체 훅 (Regenerator)
// ============================================================
void RoguePassive_OnSwitch(void)
{
    if (RoguePassive_Has(PASSIVE_REGENERATOR))
    {
        // u16 maxHp = gBattleMons[B_POSITION_PLAYER_LEFT].maxHP;
        // gBattleMons[B_POSITION_PLAYER_LEFT].hp += maxHp / 3;
    }
}

// ============================================================
//  보상 풀 빌드 (가중치 기반, 중복 제거)
// ============================================================
u8 RoguePassive_BuildRewardPool(RoguePassiveId *outPool, u8 maxCount)
{
    u8  count = 0;
    u8  i;
    bool8 alreadyHas;

    for (i = 1; i < PASSIVE_ID_COUNT && count < maxCount; i++)
    {
        if (gPassiveDefs[i].weight == 0)
            continue;

        // 이미 최대 스택이면 제외
        alreadyHas = RoguePassive_Has((RoguePassiveId)i);
        if (alreadyHas)
        {
            u8 stacks = RoguePassive_GetStacks((RoguePassiveId)i);
            if (stacks >= gPassiveDefs[i].maxStacks)
                continue;
        }

        outPool[count++] = (RoguePassiveId)i;
    }
    return count;
}
