// ============================================================
//  rogue_node_map.c  –  노드 맵 생성 / 렌더링 / 입력
//  pokeemerald-expansion Window API 기반
// ============================================================

#include "global.h"
#include "window.h"
#include "text.h"
#include "bg.h"
#include "sound.h"
#include "string_util.h"
#include "constants/songs.h"
#include "constants/characters.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_biome.h"
#include "rogue/rogue_node_map.h"
#include "rogue/rogue_ui.h"

// ── 윈도우 ────────────────────────────────────────────────────
enum { WIN_MAP, WIN_INFO, WIN_HINT, NODEMAP_WIN_COUNT };

static const struct WindowTemplate sWinTemplates[NODEMAP_WIN_COUNT] =
{
    [WIN_MAP]  = ROGUE_WIN(0,  0,  0, 20, 18, 15, 0x001),
    [WIN_INFO] = ROGUE_WIN(0, 20,  0, 10, 16, 15, 0x0E1),
    [WIN_HINT] = ROGUE_WIN(0,  0, 18, 30,  2, 15, 0x181),
};
static u8 sWinIds[NODEMAP_WIN_COUNT];

// ── 바이옴 이름 (ASCII, GBA 인코딩 호환) ─────────────────────
static const u8 * const sBiomeName[BIOME_COUNT] =
{
    [BIOME_TOWN]     = (const u8 *)"TOWN",
    [BIOME_GRASS]    = (const u8 *)"GRASS",
    [BIOME_FOREST]   = (const u8 *)"FOREST",
    [BIOME_CAVE]     = (const u8 *)"CAVE",
    [BIOME_MOUNTAIN] = (const u8 *)"MOUNTAIN",
    [BIOME_OCEAN]    = (const u8 *)"OCEAN",
    [BIOME_BEACH]    = (const u8 *)"BEACH",
    [BIOME_DESERT]   = (const u8 *)"DESERT",
    [BIOME_VOLCANO]  = (const u8 *)"VOLCANO",
    [BIOME_ICE]      = (const u8 *)"ICE",
    [BIOME_SWAMP]    = (const u8 *)"SWAMP",
    [BIOME_FACTORY]  = (const u8 *)"FACTORY",
    [BIOME_RUINS]    = (const u8 *)"RUINS",
    [BIOME_SPACE]    = (const u8 *)"SPACE",
    [BIOME_FINAL]    = (const u8 *)"FINAL",
};

static const u8 * const sNodeTypeName[NODE_COUNT] =
{
    [NODE_BATTLE]   = (const u8 *)"BATTLE",
    [NODE_ELITE]    = (const u8 *)"ELITE",
    [NODE_BOSS]     = (const u8 *)"BOSS",
    [NODE_SHOP]     = (const u8 *)"SHOP",
    [NODE_HEAL]     = (const u8 *)"HEAL",
    [NODE_EVENT]    = (const u8 *)"EVENT",
    [NODE_TREASURE] = (const u8 *)"CHEST",
};

// ── 노드 타입 심볼 (single char) ─────────────────────────────
static const u8 sNodeTypeChar[NODE_COUNT] =
{
    [NODE_BATTLE]   = '!',
    [NODE_ELITE]    = '*',
    [NODE_BOSS]     = 'B',
    [NODE_SHOP]     = '$',
    [NODE_HEAL]     = '+',
    [NODE_EVENT]    = '?',
    [NODE_TREASURE] = 'T',
};

// ── 레이어별 노드 타입 가중치 ─────────────────────────────────
// [layer][NODE_COUNT] : BATTLE ELITE BOSS SHOP HEAL EVENT TREASURE
static const u8 sLayerWeights[ROGUE_NODE_LAYERS][NODE_COUNT] =
{
    { 70, 0,   0, 10, 10, 5, 5  },  // Layer 0: 초반
    { 60, 5,   0, 15, 10, 7, 3  },  // Layer 1
    { 50, 10,  0, 15, 12, 8, 5  },  // Layer 2
    { 45, 15,  0, 15, 10, 10, 5 },  // Layer 3
    { 35, 20,  0, 20, 10, 10, 5 },  // Layer 4
    {  0,  0, 100, 0,  0,  0, 0 },  // Layer 5: 보스 전용
};

// ── 레이어별 노드 수 ──────────────────────────────────────────
static const u8 sNodesPerLayer[ROGUE_NODE_LAYERS] = { 1, 3, 3, 2, 3, 1 };

// ── 내부 유틸 ─────────────────────────────────────────────────
static u8 WeightedPick(const u8 *w, u8 count, u32 seed)
{
    u16 total = 0, roll;
    u8  i;
    for (i = 0; i < count; i++) total += w[i];
    if (total == 0) return 0;
    roll = (u16)(seed % total);
    total = 0;
    for (i = 0; i < count; i++)
    {
        total += w[i];
        if (roll < total) return i;
    }
    return count - 1;
}

// ============================================================
//  노드 맵 생성
// ============================================================
void RogueNodeMap_Generate(u32 seed)
{
    u8 layerStart[ROGUE_NODE_LAYERS];
    u8 layer, j, i;
    u8 totalNodes = 0;
    u32 s = seed;

    memset(gRogueRun.nodeMap, 0, sizeof(gRogueRun.nodeMap));

    // 레이어 시작 인덱스
    layerStart[0] = 0;
    for (layer = 1; layer < ROGUE_NODE_LAYERS; layer++)
        layerStart[layer] = layerStart[layer-1] + sNodesPerLayer[layer-1];

    totalNodes = layerStart[ROGUE_NODE_LAYERS-1] + sNodesPerLayer[ROGUE_NODE_LAYERS-1];

    // 노드 생성
    for (layer = 0; layer < ROGUE_NODE_LAYERS; layer++)
    {
        for (j = 0; j < sNodesPerLayer[layer]; j++)
        {
            u8 idx = layerStart[layer] + j;
            s = s * 0x41C64E6Du + 0x6073u;

            gRogueRun.nodeMap[idx].layer   = layer;
            gRogueRun.nodeMap[idx].type    = (u8)WeightedPick(
                sLayerWeights[layer], NODE_COUNT, s);
            gRogueRun.nodeMap[idx].biome   = (u8)RogueBiome_SelectNext(
                (RogueBiome)gRogueRun.currentBiome, s ^ (u32)layer);
            gRogueRun.nodeMap[idx].visited = (layer == 0) ? 1 : 0;

            for (i = 0; i < ROGUE_MAX_NODE_CHILDREN; i++)
                gRogueRun.nodeMap[idx].children[i] = 0xFF;
            gRogueRun.nodeMap[idx].childCount = 0;
        }
    }

    // 연결 생성
    for (layer = 0; layer < ROGUE_NODE_LAYERS - 1; layer++)
    {
        u8 curCount  = sNodesPerLayer[layer];
        u8 nextCount = sNodesPerLayer[layer+1];

        for (j = 0; j < curCount; j++)
        {
            u8 cur = layerStart[layer] + j;
            u8 connCount, target;

            s = s * 0x41C64E6Du + 0x6073u;
            connCount = 1 + (u8)((s >> 8) % 2);
            if (connCount > nextCount) connCount = nextCount;
            if (connCount > ROGUE_MAX_NODE_CHILDREN) connCount = ROGUE_MAX_NODE_CHILDREN;

            for (i = 0; i < connCount; i++)
            {
                s = s * 0x41C64E6Du + 0x6073u;
                target = layerStart[layer+1] + (u8)(s % nextCount);
                gRogueRun.nodeMap[cur].children[gRogueRun.nodeMap[cur].childCount++] = target;
            }
        }
    }

    gRogueRun.nodeCount      = totalNodes;
    gRogueRun.currentNode    = 0;
    gRogueRun.selectedChild  = 0;
}

// ============================================================
//  맵 렌더링
// ============================================================
static void DrawMap(void)
{
    u8 i, j;
    u8 buf[4];
    FillWindowPixelBuffer(sWinIds[WIN_MAP], PIXEL_FILL(0));

    for (i = 0; i < gRogueRun.nodeCount; i++)
    {
        struct RogueNode *nd = &gRogueRun.nodeMap[i];
        // 레이어 내 순서
        u8 posInLayer = 0;
        for (j = 0; j < i; j++)
            if (gRogueRun.nodeMap[j].layer == nd->layer) posInLayer++;

        u8 px = nd->layer * 26 + 2;
        u8 py = posInLayer * 46 + 10;

        buf[0] = sNodeTypeChar[nd->type];
        buf[1] = EOS;

        bool8 isCur = (i == gRogueRun.currentNode);
        bool8 isNext = FALSE;
        // 현재 노드의 자식인지 확인
        if (!isCur)
        {
            for (j = 0; j < gRogueRun.nodeMap[gRogueRun.currentNode].childCount; j++)
                if (gRogueRun.nodeMap[gRogueRun.currentNode].children[j] == i)
                    isNext = TRUE;
        }

        // 커서 (선택 중인 다음 노드)
        if (isNext && gRogueRun.nodeMap[gRogueRun.currentNode].children[gRogueRun.selectedChild] == i)
        {
            static const u8 sCur[] = { CHAR_RIGHT_ARROW, EOS };
            AddTextPrinterParameterized(sWinIds[WIN_MAP], FONT_SMALL,
                sCur, px > 6 ? px - 6 : 0, py, TEXT_SKIP_DRAW, NULL);
        }

        AddTextPrinterParameterized(sWinIds[WIN_MAP], FONT_SMALL,
            buf, px, py, TEXT_SKIP_DRAW, NULL);
    }

    CopyWindowToVram(sWinIds[WIN_MAP], COPYWIN_FULL);
}

static void DrawInfo(void)
{
    u8 nodeIdx = gRogueRun.currentNode;
    struct RogueNode *nd = &gRogueRun.nodeMap[nodeIdx];
    u8 buf[8];

    FillWindowPixelBuffer(sWinIds[WIN_INFO], PIXEL_FILL(0));

    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        (const u8 *)"NODE INFO", 2, 2, TEXT_SKIP_DRAW, NULL);

    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        sNodeTypeName[nd->type], 2, 16, TEXT_SKIP_DRAW, NULL);

    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        nd->biome < BIOME_COUNT ? sBiomeName[nd->biome] : (const u8 *)"???",
        2, 30, TEXT_SKIP_DRAW, NULL);

    // 웨이브
    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        (const u8 *)"WAVE:", 2, 50, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gRogueRun.currentWave, STR_CONV_MODE_LEFT_ALIGN, 2);
    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        buf, 44, 50, TEXT_SKIP_DRAW, NULL);

    // 돈
    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        (const u8 *)"GOLD:", 2, 64, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gRogueRun.money, STR_CONV_MODE_LEFT_ALIGN, 4);
    AddTextPrinterParameterized(sWinIds[WIN_INFO], FONT_SMALL,
        buf, 44, 64, TEXT_SKIP_DRAW, NULL);

    CopyWindowToVram(sWinIds[WIN_INFO], COPYWIN_FULL);
}

// ============================================================
//  초기화
// ============================================================
void RogueNodeMap_Init(void)
{
    u8 i;
    RogueUI_InitBg();
    gRogueRun.selectedChild = 0;

    for (i = 0; i < NODEMAP_WIN_COUNT; i++)
    {
        sWinIds[i] = AddWindow(&sWinTemplates[i]);
        FillWindowPixelBuffer(sWinIds[i], PIXEL_FILL(0));
    }

    static const u8 sHint[] = {
        'A',':',' ','G','O',' ',' ',
        'L','R',':',' ','S','E','L',EOS };
    AddTextPrinterParameterized(sWinIds[WIN_HINT], FONT_SMALL,
        sHint, 2, 2, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sWinIds[WIN_HINT], COPYWIN_FULL);

    DrawMap();
    DrawInfo();
}

// ============================================================
//  업데이트
// ============================================================
void RogueNodeMap_Update(void)
{
    struct RogueNode *cur = &gRogueRun.nodeMap[gRogueRun.currentNode];

    if (cur->childCount == 0) return;

    if (JOY_NEW(L_BUTTON) && gRogueRun.selectedChild > 0)
    {
        gRogueRun.selectedChild--;
        PlaySE(SE_SELECT);
        DrawMap();
    }
    else if (JOY_NEW(R_BUTTON) && gRogueRun.selectedChild < cur->childCount - 1)
    {
        gRogueRun.selectedChild++;
        PlaySE(SE_SELECT);
        DrawMap();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u8 nextNode = cur->children[gRogueRun.selectedChild];
        if (nextNode == 0xFF || nextNode >= gRogueRun.nodeCount) return;

        u8 i;
        for (i = 0; i < NODEMAP_WIN_COUNT; i++) RemoveWindow(sWinIds[i]);

        gRogueRun.currentNode = nextNode;
        gRogueRun.nodeMap[nextNode].visited = 1;
        gRogueRun.selectedChild = 0;
        gRogueRun.currentBiome  = gRogueRun.nodeMap[nextNode].biome;

        PlaySE(SE_PC_LOGIN);

        switch (gRogueRun.nodeMap[nextNode].type)
        {
        case NODE_BATTLE:                          RogueMain_SetState(ROGUE_STATE_BATTLE_INIT); break;
        case NODE_ELITE:  gRogueRun.isElite = 1;  RogueMain_SetState(ROGUE_STATE_BATTLE_INIT); break;
        case NODE_BOSS:   gRogueRun.isBoss  = 1;  RogueMain_SetState(ROGUE_STATE_BOSS_INTRO);  break;
        case NODE_SHOP:                            RogueMain_SetState(ROGUE_STATE_SHOP);        break;
        case NODE_HEAL:                            RogueMain_SetState(ROGUE_STATE_HEAL);        break;
        case NODE_EVENT:                           RogueMain_SetState(ROGUE_STATE_EVENT);       break;
        case NODE_TREASURE:                        RogueMain_SetState(ROGUE_STATE_REWARD);      break;
        default:                                   RogueMain_SetState(ROGUE_STATE_BATTLE_INIT); break;
        }
    }
}
