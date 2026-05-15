#ifndef ROGUE_WAVE_H
#define ROGUE_WAVE_H

// ============================================================
//  rogue_wave.h
//  PokéRogue GBA - 웨이브 레벨 스케일링 / 난이도 계산
// ============================================================

#include "global.h"

// ── 웨이브 구간 상수 ──────────────────────────────────────────
#define WAVE_PHASE_EARLY   10   // 웨이브  1~10: 초반
#define WAVE_PHASE_MID     25   // 웨이브 11~25: 중반
#define WAVE_PHASE_LATE    40   // 웨이브 26~40: 후반
#define WAVE_PHASE_END     50   // 웨이브 41~50: 최종

// ── 난이도 파라미터 ───────────────────────────────────────────
struct WaveDifficultyParams
{
    u8  enemyPartySize;    // 적 파티 최대 크기
    u8  ivBonus;           // 적 IV 보너스 (0~31)
    u8  evBonus;           // 적 EV 보너스 (0~252)
    u8  isEliteChance;     // 엘리트 등장 확률 (0~100%)
    u16 moneyMultiplier;   // 획득 돈 배율 (100 = 1.0배)
};

// ── API ───────────────────────────────────────────────────────
u8   RogueWave_GetEnemyMinLevel(u8 wave);
u8   RogueWave_GetEnemyMaxLevel(u8 wave);
u8   RogueWave_GetEliteBonus(u8 wave);
void RogueWave_GetDifficultyParams(u8 wave, struct WaveDifficultyParams *out);
bool8 RogueWave_IsBossWave(u8 wave);
u8   RogueWave_GetPhase(u8 wave);

#endif // ROGUE_WAVE_H
