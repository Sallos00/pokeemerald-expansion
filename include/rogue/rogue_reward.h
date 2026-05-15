#ifndef ROGUE_REWARD_H
#define ROGUE_REWARD_H

#include "global.h"
#include "rogue/rogue_defines.h"
#include "rogue/rogue_biome.h"

void  RogueReward_Init(void);
void  RogueReward_Update(void);
void  RogueReward_GiveItem(u16 itemId);
void  RogueReward_GivePokemon(u16 species, u8 level);
u16   RogueReward_RollItem(RogueBiome biome, u8 wave, u32 seed);

#endif
