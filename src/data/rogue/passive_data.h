// ============================================================
//  passive_data.h
//  PokéRogue GBA - 패시브 어빌리티 ROM 데이터
//  (import_passives.py 실행 전 초기 수동 데이터)
// ============================================================

#ifndef PASSIVE_DATA_H
#define PASSIVE_DATA_H

#include "rogue/rogue_passive.h"

// ── 보상 풀 전용 가중치 테이블 ───────────────────────────────
// 웨이브 구간별 패시브 등장 가중치 보정
// [phase][PASSIVE_ID_COUNT] 구조
// (실제 weight는 gPassiveDefs[].weight 기반)

// 초반(phase 0): 단순한 패시브 선호
static const u8 sPassivePhaseWeights_Early[PASSIVE_ID_COUNT] =
{
    [PASSIVE_NONE]          = 0,
    [PASSIVE_SPEED_BOOST]   = 40,
    [PASSIVE_DROUGHT]       = 15,
    [PASSIVE_DRIZZLE]       = 15,
    [PASSIVE_GUTS]          = 30,
    [PASSIVE_HUSTLE]        = 25,
    [PASSIVE_THICK_FAT]     = 30,
    [PASSIVE_REGENERATOR]   = 35,
    [PASSIVE_MULTISCALE]    = 10,
    [PASSIVE_WONDER_GUARD]  = 0,
    [PASSIVE_ADAPTABILITY]  = 20,
    [PASSIVE_SHEER_FORCE]   = 20,
    [PASSIVE_ANALYTIC]      = 10,
    [PASSIVE_TINTED_LENS]   = 15,
    [PASSIVE_MOXIE]         = 25,
    [PASSIVE_INTIMIDATE]    = 30,
};

// 후반(phase 2+): 강력한 패시브 비중 증가
static const u8 sPassivePhaseWeights_Late[PASSIVE_ID_COUNT] =
{
    [PASSIVE_NONE]          = 0,
    [PASSIVE_SPEED_BOOST]   = 25,
    [PASSIVE_DROUGHT]       = 20,
    [PASSIVE_DRIZZLE]       = 20,
    [PASSIVE_GUTS]          = 20,
    [PASSIVE_HUSTLE]        = 20,
    [PASSIVE_THICK_FAT]     = 20,
    [PASSIVE_REGENERATOR]   = 30,
    [PASSIVE_MULTISCALE]    = 25,
    [PASSIVE_WONDER_GUARD]  = 5,
    [PASSIVE_ADAPTABILITY]  = 25,
    [PASSIVE_SHEER_FORCE]   = 25,
    [PASSIVE_ANALYTIC]      = 20,
    [PASSIVE_TINTED_LENS]   = 25,
    [PASSIVE_MOXIE]         = 25,
    [PASSIVE_INTIMIDATE]    = 20,
};

#endif // PASSIVE_DATA_H
