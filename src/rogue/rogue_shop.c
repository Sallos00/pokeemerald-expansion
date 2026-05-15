// ============================================================
//  rogue_shop.c  –  상점 화면
// ============================================================

#include "global.h"
#include "window.h"
#include "text.h"
#include "bg.h"
#include "sound.h"
#include "item.h"
#include "string_util.h"
#include "constants/songs.h"
#include "constants/items.h"
#include "constants/characters.h"

#include "rogue/rogue_main.h"
#include "rogue/rogue_reward.h"
#include "rogue/rogue_passive.h"
#include "rogue/rogue_biome.h"
#include "rogue/rogue_ui.h"
#include "data/rogue/reward_data.h"

// ── 윈도우 ────────────────────────────────────────────────────
enum { SHOP_WIN_TITLE, SHOP_WIN_LIST, SHOP_WIN_GOLD, SHOP_WIN_HINT, SHOP_WIN_COUNT };

static const struct WindowTemplate sShopWinTemplates[SHOP_WIN_COUNT] =
{
    [SHOP_WIN_TITLE] = ROGUE_WIN(0,  0,  0, 30,  2, 15, 0x001),
    [SHOP_WIN_LIST]  = ROGUE_WIN(0,  0,  2, 22, 16, 15, 0x03D),
    [SHOP_WIN_GOLD]  = ROGUE_WIN(0, 22,  2,  8,  4, 15, 0x18D),
    [SHOP_WIN_HINT]  = ROGUE_WIN(0,  0, 18, 30,  2, 15, 0x1A9),
};
static u8 sShopWinIds[SHOP_WIN_COUNT];

struct ShopEntry { u8 type; u16 value; u16 price; u8 sold; };
static struct ShopEntry sShop[ROGUE_MAX_SHOP_ITEMS];
static u8 sShopCursor = 0;

static void Shop_Build(void)
{
    u8  i;
    u32 seed = gRogueRun.waveSeed ^ 0x5A3C0000u;
    u8  wave = gRogueRun.currentWave;

    for (i = 0; i < ROGUE_MAX_SHOP_ITEMS; i++)
    {
        seed = seed * 0x41C64E6Du + 0x6073u;

        if (i < 4)
        {
            sShop[i].type  = REWARD_ITEM;
            sShop[i].value = RogueReward_RollItem(
                (RogueBiome)gRogueRun.currentBiome, wave, seed);
        }
        else if (i == 4)
        {
            RoguePassiveId pool[PASSIVE_ID_COUNT];
            u8 n = RoguePassive_BuildRewardPool(pool, PASSIVE_ID_COUNT);
            sShop[i].type  = (n > 0) ? REWARD_PASSIVE : REWARD_ITEM;
            sShop[i].value = (n > 0) ? (u16)pool[seed % n] : ITEM_RARE_CANDY;
        }
        else
        {
            sShop[i].type  = REWARD_TM;
            // 간단 TM 풀 (items.h의 실제 TM 아이템 ID)
            sShop[i].value = ITEM_TM01 + (u16)(seed % 50);
        }

        // 가격
        u16 base = (sShop[i].type == REWARD_PASSIVE) ? 300u : 100u;
        u16 price = (u16)((u32)base * (100 + wave * 3) / 100);
        price = (u16)((u32)price * (100 - gRogueRun.shopDiscount) / 100);
        sShop[i].price = price ? price : 10;
        sShop[i].sold  = 0;
    }
}

static void Shop_DrawList(void)
{
    u8 i;
    u8 buf[12];
    FillWindowPixelBuffer(sShopWinIds[SHOP_WIN_LIST], PIXEL_FILL(0));

    for (i = 0; i < ROGUE_MAX_SHOP_ITEMS; i++)
    {
        u8 py = i * 22 + 2;
        bool8 sel = (i == sShopCursor);

        if (sel)
        {
            static const u8 sCur[] = { CHAR_RIGHT_ARROW, EOS };
            AddTextPrinterParameterized(sShopWinIds[SHOP_WIN_LIST], FONT_SMALL,
                sCur, 2, py, TEXT_SKIP_DRAW, NULL);
        }

        if (sShop[i].sold)
        {
            static const u8 sSold[] = { 'S','O','L','D',' ','O','U','T',EOS };
            AddTextPrinterParameterized(sShopWinIds[SHOP_WIN_LIST], FONT_SMALL,
                sSold, 14, py, TEXT_SKIP_DRAW, NULL);
        }
        else
        {
            // 이름
            u8 nameBuf[16];
            if (sShop[i].type == REWARD_PASSIVE && sShop[i].value < PASSIVE_ID_COUNT)
                
        StringCopyN(nameBuf, gPassiveDefs[sShop[i].value].name ? gPassiveDefs[sShop[i].value].name : (const u8 *)"PASSIVE", 15);
            else
                StringCopyN(nameBuf, GetItemName(sShop[i].value), 15);
            AddTextPrinterParameterized(sShopWinIds[SHOP_WIN_LIST], FONT_SMALL,
                nameBuf, 14, py, TEXT_SKIP_DRAW, NULL);

            // 가격
            ConvertIntToDecimalStringN(buf, sShop[i].price, STR_CONV_MODE_LEFT_ALIGN, 4);
            buf[4] = 'G'; buf[5] = EOS;
            AddTextPrinterParameterized(sShopWinIds[SHOP_WIN_LIST], FONT_SMALL,
                buf, 120, py, TEXT_SKIP_DRAW, NULL);
        }
    }
    CopyWindowToVram(sShopWinIds[SHOP_WIN_LIST], COPYWIN_FULL);
}

static void Shop_DrawGold(void)
{
    u8 buf[8];
    FillWindowPixelBuffer(sShopWinIds[SHOP_WIN_GOLD], PIXEL_FILL(0));
    AddTextPrinterParameterized(sShopWinIds[SHOP_WIN_GOLD], FONT_SMALL,
        (const u8 *)"GOLD:", 2, 2, TEXT_SKIP_DRAW, NULL);
    ConvertIntToDecimalStringN(buf, gRogueRun.money, STR_CONV_MODE_LEFT_ALIGN, 4);
    AddTextPrinterParameterized(sShopWinIds[SHOP_WIN_GOLD], FONT_SMALL,
        buf, 2, 14, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sShopWinIds[SHOP_WIN_GOLD], COPYWIN_FULL);
}

void RogueShop_Init(void)
{
    u8 i;
    static const u8 sTitle[] = {'-',' ','S','H','O','P',' ','-',EOS};
    static const u8 sHint[]  = {'A',':',' ','B','U','Y',' ',' ','B',':',' ','L','E','A','V','E',EOS};

    RogueUI_InitBg();
    sShopCursor = 0;
    Shop_Build();

    for (i = 0; i < SHOP_WIN_COUNT; i++)
    {
        sShopWinIds[i] = AddWindow(&sShopWinTemplates[i]);
        FillWindowPixelBuffer(sShopWinIds[i], PIXEL_FILL(0));
    }
    AddTextPrinterParameterized(sShopWinIds[SHOP_WIN_TITLE], FONT_NORMAL,
        sTitle, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sShopWinIds[SHOP_WIN_TITLE], COPYWIN_FULL);

    AddTextPrinterParameterized(sShopWinIds[SHOP_WIN_HINT], FONT_SMALL,
        sHint, 4, 2, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sShopWinIds[SHOP_WIN_HINT], COPYWIN_FULL);

    Shop_DrawList();
    Shop_DrawGold();
}

void RogueShop_Update(void)
{
    if (JOY_NEW(DPAD_UP) && sShopCursor > 0)
    {
        sShopCursor--;
        PlaySE(SE_SELECT);
        Shop_DrawList();
    }
    else if (JOY_NEW(DPAD_DOWN) && sShopCursor < ROGUE_MAX_SHOP_ITEMS - 1)
    {
        sShopCursor++;
        PlaySE(SE_SELECT);
        Shop_DrawList();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        struct ShopEntry *e = &sShop[sShopCursor];
        if (e->sold || gRogueRun.money < e->price)
        {
            PlaySE(SE_FAILURE);
            return;
        }
        gRogueRun.money = (u16)(gRogueRun.money - e->price);
        e->sold = 1;
        switch (e->type)
        {
        case REWARD_ITEM: case REWARD_TM: RogueReward_GiveItem(e->value);                   break;
        case REWARD_PASSIVE:              RoguePassive_Add((RoguePassiveId)e->value);         break;
        default: break;
        }
        PlaySE(SE_SHOP);
        Shop_DrawList();
        Shop_DrawGold();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        u8 i;
        for (i = 0; i < SHOP_WIN_COUNT; i++) RemoveWindow(sShopWinIds[i]);
        RogueMain_SetState(ROGUE_STATE_NODE_MAP);
    }
}
