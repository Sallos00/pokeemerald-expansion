// ============================================================
//  rogue_wave.c
//  PokéRogue GBA - 웨이브 레벨 스케일링 / 난이도 계산
//
//  레벨 커브 (PokéRogue 참고):
//    웨이브  1 → Lv  5~ 8   (초반)
//    웨이브 10 → Lv 25~30   (보스1)
//    웨이브 20 → Lv 45~50   (보스2)
//    웨이브 30 → Lv 60~65   (보스3)
//    웨이브 40 → Lv 78~82   (보스4)
//    웨이브 50 → Lv 95~100  (최종)
// ============================================================

#include "global.h"
#include "rogue/rogue_wave.h"
#include "rogue/rogue_defines.h"
#include "rogue/rogue_main.h"

// ── 웨이브별 레벨 룩업 테이블 (선형 보간 기준점) ─────────────
// { wave, minLv, maxLv }
static const struct { u8 wave; u8 minLv; u8 maxLv; } sLevelCurve[] =
{
    {  1,  5,  8 },
    {  5, 12, 16 },
    { 10, 25, 30 },
    { 15, 36, 41 },
    { 20, 45, 50 },
    { 25, 55, 60 },
    { 30, 62, 67 },
    { 35, 70, 75 },
    { 40, 78, 83 },
    { 45, 87, 92 },
    { 50, 95,100 },
};

#define LEVEL_CURVE_COUNT  ARRAY_COUNT(sLevelCurve)

// ============================================================
//  선형 보간 (정수 연산)
// ============================================================
static u8 LerpU8(u8 a, u8 b, u8 t, u8 maxT)
{
    // a + (b - a) * t / maxT
    if (b >= a)
        return a + (u8)((u32)(b - a) * t / maxT);
    return a - (u8)((u32)(a - b) * t / maxT);
}

// ============================================================
//  레벨 커브 조회 + 보간
// ============================================================
u8 RogueWave_GetEnemyMinLevel(u8 wave)
{
    u8 i;

    if (wave <= sLevelCurve[0].wave)
        return sLevelCurve[0].minLv;
    if (wave >= sLevelCurve[LEVEL_CURVE_COUNT - 1].wave)
        return sLevelCurve[LEVEL_CURVE_COUNT - 1].minLv;

    for (i = 0; i < LEVEL_CURVE_COUNT - 1; i++)
    {
        if (wave >= sLevelCurve[i].wave && wave <= sLevelCurve[i + 1].wave)
        {
            u8 span = sLevelCurve[i + 1].wave - sLevelCurve[i].wave;
            u8 t    = wave - sLevelCurve[i].wave;
            return LerpU8(sLevelCurve[i].minLv,
                          sLevelCurve[i + 1].minLv, t, span);
        }
    }
    return sLevelCurve[LEVEL_CURVE_COUNT - 1].minLv;
}

u8 RogueWave_GetEnemyMaxLevel(u8 wave)
{
    u8 i;

    if (wave <= sLevelCurve[0].wave)
        return sLevelCurve[0].maxLv;
    if (wave >= sLevelCurve[LEVEL_CURVE_COUNT - 1].wave)
        return sLevelCurve[LEVEL_CURVE_COUNT - 1].maxLv;

    for (i = 0; i < LEVEL_CURVE_COUNT - 1; i++)
    {
        if (wave >= sLevelCurve[i].wave && wave <= sLevelCurve[i + 1].wave)
        {
            u8 span = sLevelCurve[i + 1].wave - sLevelCurve[i].wave;
            u8 t    = wave - sLevelCurve[i].wave;
            return LerpU8(sLevelCurve[i].maxLv,
                          sLevelCurve[i + 1].maxLv, t, span);
        }
    }
    return sLevelCurve[LEVEL_CURVE_COUNT - 1].maxLv;
}

// ============================================================
//  엘리트 레벨 보너스
// ============================================================
u8 RogueWave_GetEliteBonus(u8 wave)
{
    // 초반 +3, 후반 +6
    if (wave < WAVE_PHASE_MID)  return 3;
    if (wave < WAVE_PHASE_LATE) return 5;
    return 7;
}

// ============================================================
//  웨이브 구간 (Phase)
// ============================================================
u8 RogueWave_GetPhase(u8 wave)
{
    if (wave <= WAVE_PHASE_EARLY)  return 0;  // 초반
    if (wave <= WAVE_PHASE_MID)    return 1;  // 중반
    if (wave <= WAVE_PHASE_LATE)   return 2;  // 후반
    return 3;                                 // 최종
}

// ============================================================
//  보스 웨이브 체크
// ============================================================
bool8 RogueWave_IsBossWave(u8 wave)
{
    return (bool8)((wave % ROGUE_BOSS_WAVE_INTERVAL) == 0 && wave > 0);
}

// ============================================================
//  난이도 파라미터 종합
// ============================================================
void RogueWave_GetDifficultyParams(u8 wave, struct WaveDifficultyParams *out)
{
    u8 phase = RogueWave_GetPhase(wave);

    // 적 파티 크기 (초반 1, 후반 최대 6)
    switch (phase)
    {
    case 0: out->enemyPartySize = 1 + (wave / 5);     break;  // 1~3
    case 1: out->enemyPartySize = 3 + (wave / 10);    break;  // 3~5
    case 2: out->enemyPartySize = 5;                  break;
    case 3: out->enemyPartySize = 6;                  break;
    default: out->enemyPartySize = 1;                 break;
    }
    if (out->enemyPartySize > 6) out->enemyPartySize = 6;

    // IV 보너스 (초반 0, 최종 31)
    out->ivBonus = (u8)((u32)wave * 31 / ROGUE_MAX_WAVES);

    // EV 보너스 (0~252)
    out->evBonus = (u8)((u32)wave * 252 / ROGUE_MAX_WAVES);
    if (out->evBonus > 252) out->evBonus = 252;

    // 엘리트 확률 (초반 0%, 후반 20%)
    out->isEliteChance = (u8)(phase * 7);

    // 돈 배율 (후반일수록 많이)
    out->moneyMultiplier = 100 + (u16)wave * 10;

    // 메타 업그레이드 반영
    // (추후: 영구 업그레이드로 enemyPartySize 감소 등 가능)
}
