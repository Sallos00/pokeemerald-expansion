// ============================================================
//  rogue_save.c
//  PokéRogue GBA - SRAM 저장 시스템
//
//  GBA SRAM은 반드시 8비트 단위로 접근해야 함.
//  pokeemerald의 기본 Save는 Flash 기반이므로
//  Rogue 메타데이터는 별도 SRAM 영역에 저장.
//  SRAM 오프셋 0x6000 이후 사용 (pokeemerald 세이브와 충돌 방지)
// ============================================================

#include "global.h"
#include "rogue/rogue_main.h"
#include "rogue/rogue_save.h"

// GBA SRAM 기저 주소
// GBA SRAM 0x0E000000
#define ROGUE_SRAM_PTR ((vu8 *)0x0E000000u)

// pokeemerald의 Flash 세이브는 보통 0~0x5FFF 영역 사용
// Rogue 메타는 0x6000 이후에 저장
#define ROGUE_META_SRAM_OFFSET  0x6000u

// ============================================================
//  바이트 단위 SRAM 읽기 / 쓰기
// ============================================================
static void Sram_Write8(u32 offset, const void *src, u32 size)
{
    const u8 *p = (const u8 *)src;
    u32 i;
    for (i = 0; i < size; i++)
        ROGUE_SRAM_PTR[offset + i] = p[i];
}

static void Sram_Read8(u32 offset, void *dst, u32 size)
{
    u8 *p = (u8 *)dst;
    u32 i;
    for (i = 0; i < size; i++)
        p[i] = ROGUE_SRAM_PTR[offset + i];
}

// ============================================================
//  메타 데이터 저장
// ============================================================
void RogueSave_SaveMeta(const struct RogueMetaData *meta)
{
    Sram_Write8(ROGUE_META_SRAM_OFFSET,
                meta, sizeof(struct RogueMetaData));
}

// ============================================================
//  메타 데이터 로드
// ============================================================
void RogueSave_LoadMeta(struct RogueMetaData *meta)
{
    Sram_Read8(ROGUE_META_SRAM_OFFSET,
               meta, sizeof(struct RogueMetaData));
}

// ============================================================
//  메타 데이터 초기화 (최초 실행)
// ============================================================
void RogueSave_InitMeta(struct RogueMetaData *meta)
{
    memset(meta, 0, sizeof(struct RogueMetaData));
    meta->magic   = ROGUE_META_SAVE_MAGIC;
    meta->version = ROGUE_META_VERSION;
    // 초기 바이옴 해금 (TOWN + GRASS)
    meta->unlockedBiomes = (u16)((1 << BIOME_TOWN) | (1 << BIOME_GRASS));
}
