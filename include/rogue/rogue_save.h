#ifndef ROGUE_SAVE_H
#define ROGUE_SAVE_H

#include "global.h"
#include "rogue/rogue_main.h"

void RogueSave_SaveMeta(const struct RogueMetaData *meta);
void RogueSave_LoadMeta(struct RogueMetaData *meta);
void RogueSave_InitMeta(struct RogueMetaData *meta);

#endif
