#ifndef ROGUE_PASSIVE_H
#define ROGUE_PASSIVE_H

// ============================================================
//  rogue_passive.h
//  PokéRogue GBA - 패시브 어빌리티 시스템
// ============================================================

#include "global.h"
#include "rogue/rogue_defines.h"
#include "rogue/rogue_main.h"

// ── 패시브 트리거 시점 ────────────────────────────────────────
typedef enum RoguePassiveTrigger
{
    TRIGGER_BATTLE_START = 0,  // 배틀 시작 시
    TRIGGER_TURN_START,        // 매 턴 시작
    TRIGGER_ON_ATTACK,         // 공격 시
    TRIGGER_ON_HIT,            // 피격 시
    TRIGGER_ON_KO,             // KO 시
    TRIGGER_ON_SWITCH,         // 교체 시
    TRIGGER_WAVE_CLEAR,        // 웨이브 클리어 후
    TRIGGER_COUNT
} RoguePassiveTrigger;

// ── 패시브 정의 구조체 ────────────────────────────────────────
struct RoguePassiveDef
{
    u16  id;                // RoguePassiveId
    u8   maxStacks;         // 최대 중첩 수
    u8   weight;            // 보상 풀 등장 가중치

    // 전투 보정 (1024 = 1.0배)
    u16  atkMod;            // 공격 배율
    u16  defMod;            // 방어 배율
    u16  spaModifier;       // 특공 배율
    u16  spdModifier;       // 특방 배율
    u16  speedModifier;     // 스피드 배율

    // 트리거
    u8   trigger;           // RoguePassiveTrigger
    u8   triggerParam;      // 트리거별 추가 파라미터

    const u8 *name;         // ROM 문자열
    const u8 *desc;
};

// ── 전투 보정 컨텍스트 ────────────────────────────────────────
// pokeemerald 배틀 엔진에서 호출하는 구조체
struct RoguePassiveContext
{
    u16 atkModTotal;    // 최종 공격 배율 (1024 = 1.0)
    u16 defModTotal;
    u16 spaModTotal;
    u16 spdModTotal;
    u16 speedModTotal;
    u8  weatherOverride;    // 0 = 없음, 날씨 ID
    u8  regenOnSwitch;      // 1 = 교체 시 1/3 회복
    u8  dmgHalfOnFull;      // 1 = 만피 시 데미지 반감
    u8  superEffOnNeutral;  // 1 = 반감 기술 2배
    u8  boostOnKO;          // 공격 +1 per KO
    u8  _pad[3];
};

// ── 전역 패시브 정의 테이블 (ROM) ─────────────────────────────
extern const struct RoguePassiveDef gPassiveDefs[PASSIVE_ID_COUNT];

// ── API ───────────────────────────────────────────────────────
void  RoguePassive_Init(void);
bool8 RoguePassive_Add(RoguePassiveId id);
bool8 RoguePassive_Remove(RoguePassiveId id);
u8    RoguePassive_GetStacks(RoguePassiveId id);
bool8 RoguePassive_Has(RoguePassiveId id);
void  RoguePassive_StackAll(RoguePassiveTrigger trigger);

// 배틀 엔진 훅 (rogue_battle.c → pokeemerald battle_main.c 연결)
void  RoguePassive_BuildContext(struct RoguePassiveContext *ctx);
void  RoguePassive_OnBattleStart(void);
void  RoguePassive_OnTurnStart(void);
void  RoguePassive_OnKO(void);
void  RoguePassive_OnSwitch(void);

// 보상 풀 빌드 (rogue_reward.c에서 사용)
u8    RoguePassive_BuildRewardPool(RoguePassiveId *outPool, u8 maxCount);

#endif // ROGUE_PASSIVE_H
