#ifndef ROGUE_BATTLE_H
#define ROGUE_BATTLE_H

#include "global.h"
#include "rogue/rogue_defines.h"

// 배틀 초기화 실행 (BATTLE_INIT 상태에서 호출)
void RogueBattleInit_Run(void);

// 승리 화면
void RogueVictory_Init(void);
void RogueVictory_Update(void);

#endif
