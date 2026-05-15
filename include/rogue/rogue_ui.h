#ifndef ROGUE_UI_H
#define ROGUE_UI_H

// ============================================================
//  rogue_ui.h
//  PokéRogue GBA - UI 헬퍼 (pokeemerald Window 시스템 래퍼)
// ============================================================

#include "global.h"
#include "window.h"
#include "text.h"
#include "constants/characters.h"

// ── 표준 색상 배열 ────────────────────────────────────────────
// {배경, 전경, 그림자} - pokeemerald AddTextPrinterParameterized 형식
extern const u8 gRogueColorWhite[3];   // 흰 글씨
extern const u8 gRogueColorYellow[3];  // 노란 글씨 (선택)
extern const u8 gRogueColorGray[3];    // 회색 글씨 (비활성)
extern const u8 gRogueColorRed[3];     // 빨간 글씨 (경고)

// ── 윈도우 ID 레지스트리 (상태별로 재사용) ────────────────────
#define ROGUE_WINID_INVALID  0xFF

// ── 윈도우 생성 헬퍼 ──────────────────────────────────────────
// pokeemerald WindowTemplate을 쉽게 만들기 위한 매크로
#define ROGUE_WIN(bg_, x_, y_, w_, h_, pal_, base_) \
    { .bg=(bg_), .tilemapLeft=(x_), .tilemapTop=(y_), \
      .width=(w_), .height=(h_), .paletteNum=(pal_), .baseBlock=(base_) }

// ── API ───────────────────────────────────────────────────────

// 화면 전체 BG 클리어
void  RogueUI_ClearScreen(void);

// 전체 화면 BG 텍스트 레이어 초기화
void  RogueUI_InitBg(void);

// 윈도우에 텍스트 출력
// windowId: AddWindow() 반환값
// color:    gRogueColorWhite 등
void  RogueUI_Print(u8 windowId, u8 fontId,
                    u8 x, u8 y, const u8 *color,
                    const u8 *str);

// 정수 → 문자열 + 단위 출력
void  RogueUI_PrintInt(u8 windowId, u8 fontId,
                       u8 x, u8 y, const u8 *color,
                       s32 value, u8 digits);

// 윈도우 내용 화면에 적용
void  RogueUI_FlushWindow(u8 windowId);

// 윈도우 지우기 (내용 + 타일맵)
void  RogueUI_ClearWindow(u8 windowId);

// ── 표준 프레임 그리기 ────────────────────────────────────────
void  RogueUI_DrawWindowFrame(u8 windowId);

// ── 커서 표시 ─────────────────────────────────────────────────
void  RogueUI_PrintCursor(u8 windowId, u8 x, u8 y, bool8 visible);

// ── 내부 전역 (공유 문자열 버퍼) ─────────────────────────────
extern u8 gRogueStrBuf[64];

#endif // ROGUE_UI_H
