#!/usr/bin/env python3
# ============================================================
#  extract_korean_chars.py
#  PokéRogue GBA - 한글 문자 추출 및 폰트 생성 도구
#
#  사용법:
#    python3 tools/pokerogue_import/extract_korean_chars.py \
#        --locales    /path/to/pokerogue-locales/ko \
#        --font-ttf   tools/fonts/NotoSansKR-Regular.ttf \
#        --output-dir src/data/rogue
#
#  출력:
#    src/data/rogue/hangul_table.generated.h   (유니코드 → 타일 인덱스)
#    src/data/rogue/hangul_glyphs.generated.h  (8x8 폰트 비트맵)
#    tools/preview/hangul_preview.png          (디버그용 미리보기)
#
#  의존성:
#    pip install pillow freetype-py
# ============================================================

import re
import os
import sys
import json
import argparse
import struct
from pathlib import Path
from typing import Set, List, Tuple, Dict

try:
    from PIL import Image, ImageDraw, ImageFont
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False
    print("[WARN] Pillow 없음. 폰트 렌더링에 더미 데이터를 사용합니다.")
    print("       pip install pillow freetype-py")

# ── 상수 ──────────────────────────────────────────────────────
HANGUL_START   = 0xAC00   # 가
HANGUL_END     = 0xD7A3   # 힣
GBA_GLYPH_SIZE = 32       # 8x8 @ 4bpp = 32 bytes
MAX_CHARS      = 208      # GBA 타일 여유 = 0x50~0xFF
TILE_OFFSET    = 0x50     # pokeemerald 기존 문자 이후

# ── 한글 완성형 유니코드 분해 ─────────────────────────────────
# 초성 / 중성 / 종성 리스트
CHOSEONG  = ['ㄱ','ㄲ','ㄴ','ㄷ','ㄸ','ㄹ','ㅁ','ㅂ','ㅃ',
             'ㅅ','ㅆ','ㅇ','ㅈ','ㅉ','ㅊ','ㅋ','ㅌ','ㅍ','ㅎ']
JUNGSEONG = ['ㅏ','ㅐ','ㅑ','ㅒ','ㅓ','ㅔ','ㅕ','ㅖ','ㅗ',
             'ㅘ','ㅙ','ㅚ','ㅛ','ㅜ','ㅝ','ㅞ','ㅟ','ㅠ','ㅡ','ㅢ','ㅣ']
JONGSEONG = ['','ㄱ','ㄲ','ㄳ','ㄴ','ㄵ','ㄶ','ㄷ','ㄹ',
             'ㄺ','ㄻ','ㄼ','ㄽ','ㄾ','ㄿ','ㅀ','ㅁ','ㅂ',
             'ㅄ','ㅅ','ㅆ','ㅇ','ㅈ','ㅊ','ㅋ','ㅌ','ㅍ','ㅎ']

def decompose_hangul(char: str) -> Tuple[int, int, int]:
    """완성형 한글 → (초성, 중성, 종성) 인덱스"""
    code = ord(char) - HANGUL_START
    if code < 0:
        return (-1, -1, -1)
    jong = code % 28
    code //= 28
    jung = code % 21
    cho  = code // 21
    return cho, jung, jong

# ── 로케일 JSON에서 한글 문자 추출 ───────────────────────────

def extract_chars_from_json(json_dir: Path) -> Set[str]:
    """모든 JSON 파일에서 한글 문자 집합 추출"""
    chars: Set[str] = set()
    json_files = list(json_dir.rglob("*.json"))
    print(f"[*] JSON 파일 {len(json_files)}개 스캔 중...")

    for jf in json_files:
        try:
            text = jf.read_text(encoding="utf-8")
            for ch in text:
                if HANGUL_START <= ord(ch) <= HANGUL_END:
                    chars.add(ch)
        except Exception as e:
            print(f"[WARN] {jf}: {e}")

    return chars

# ── GBA 4bpp 타일 생성 ────────────────────────────────────────

class GlyphRenderer:
    """
    TTF → 8x8 GBA 4bpp 타일 렌더러
    4bpp: 각 픽셀 = 4비트, 팔레트 인덱스 0~15
         0 = 투명, 1~15 = 팔레트 색상
    """
    def __init__(self, font_path: Optional[Path] = None, size: int = 7):
        self.size = size
        self.font = None

        if font_path and PIL_AVAILABLE:
            try:
                from PIL import ImageFont as PILFont
                self.font = PILFont.truetype(str(font_path), size)
                print(f"[+] 폰트 로드: {font_path} ({size}px)")
            except Exception as e:
                print(f"[WARN] 폰트 로드 실패: {e}")

    def render_char(self, char: str) -> bytes:
        """
        문자 → 32바이트 GBA 4bpp 타일
        각 바이트 = 2픽셀 (하위 4비트 먼저)
        """
        if not PIL_AVAILABLE or not self.font:
            return self._dummy_glyph(char)

        img = Image.new("L", (8, 8), 0)
        draw = ImageDraw.Draw(img)

        # 문자 렌더링
        try:
            bbox = draw.textbbox((0, 0), char, font=self.font)
            w = bbox[2] - bbox[0]
            h = bbox[3] - bbox[1]
            x = (8 - w) // 2
            y = (8 - h) // 2
            draw.text((x, y), char, fill=255, font=self.font)
        except Exception:
            draw.text((0, 0), char, fill=255)

        # 픽셀 → 4bpp 변환
        pixels = list(img.getdata())
        tile_data = bytearray(32)

        for i, px in enumerate(pixels[:64]):
            pal_idx = 1 if px > 128 else 0  # 임계값 이진화
            byte_pos = i // 2
            if (i % 2) == 0:
                tile_data[byte_pos] = pal_idx & 0x0F
            else:
                tile_data[byte_pos] |= (pal_idx & 0x0F) << 4

        return bytes(tile_data)

    def _dummy_glyph(self, char: str) -> bytes:
        """폰트 없을 때 더미 글리프 (단순 박스)"""
        tile = bytearray(32)
        # 첫 행, 마지막 행 = 가로선
        tile[0]  = 0xFF  # 첫 행 (2픽셀)
        tile[1]  = 0xFF
        tile[2]  = 0xFF
        tile[3]  = 0xFF
        tile[28] = 0xFF
        tile[29] = 0xFF
        tile[30] = 0xFF
        tile[31] = 0xFF
        # 양쪽 세로선
        for row in range(1, 7):
            base = row * 4
            tile[base]     |= 0x01  # 왼쪽 픽셀
            tile[base + 3] |= 0x10  # 오른쪽 픽셀
        return bytes(tile)


# ── C 헤더 생성 ───────────────────────────────────────────────

def generate_table_header(char_map: Dict[str, int]) -> str:
    """hangul_table.generated.h 생성"""
    lines = []
    lines.append("// AUTO-GENERATED by extract_korean_chars.py — DO NOT EDIT")
    lines.append("")
    lines.append('#include "rogue/rogue_hangul.h"')
    lines.append("")

    lines.append(f"const u16 gHangulTableSize = {len(char_map)};")
    lines.append("")
    lines.append("// 유니코드(오름차순) → GBA 타일 인덱스 매핑")
    lines.append("const struct HangulEntry gHangulTable[] =")
    lines.append("{")

    for char in sorted(char_map.keys(), key=lambda c: ord(c)):
        tile_idx = char_map[char]
        unicode_val = ord(char)
        cho, jung, jong = decompose_hangul(char)
        comment = f"// {char} ({CHOSEONG[cho]}{JUNGSEONG[jung]}{JONGSEONG[jong]})"
        lines.append(f"    {{ 0x{unicode_val:04X}, 0x{tile_idx:02X}, 0 }},  {comment}")

    lines.append("};")
    return "\n".join(lines)


def generate_glyphs_header(char_list: List[str], renderer: GlyphRenderer) -> str:
    """hangul_glyphs.generated.h 생성"""
    lines = []
    lines.append("// AUTO-GENERATED by extract_korean_chars.py — DO NOT EDIT")
    lines.append("")
    lines.append("// 각 엔트리: 32바이트 (8x8 @ 4bpp GBA 타일)")
    lines.append(f"const u8 gHangulGlyphs[{len(char_list)} * {GBA_GLYPH_SIZE}] =")
    lines.append("{")

    for char in char_list:
        glyph = renderer.render_char(char)
        hex_bytes = ", ".join(f"0x{b:02X}" for b in glyph)
        lines.append(f"    /* '{char}' U+{ord(char):04X} */")
        lines.append(f"    {hex_bytes},")

    lines.append("};")
    return "\n".join(lines)


def generate_preview(char_map: Dict[str, int], renderer: GlyphRenderer,
                     out_path: Path):
    """디버그용 PNG 미리보기 생성"""
    if not PIL_AVAILABLE:
        return

    cols = 16
    rows = (len(char_map) + cols - 1) // cols
    img  = Image.new("RGB", (cols * 9, rows * 9), (30, 30, 30))

    char_list = sorted(char_map.keys(), key=lambda c: ord(c))
    for i, char in enumerate(char_list):
        glyph = renderer.render_char(char)
        glyph_img = Image.new("L", (8, 8))
        pixels = []
        for b_idx in range(32):
            lo = glyph[b_idx] & 0x0F
            hi = (glyph[b_idx] >> 4) & 0x0F
            pixels.append(lo * 17)
            pixels.append(hi * 17)
        glyph_img.putdata(pixels)

        x = (i % cols) * 9
        y = (i // cols) * 9
        img.paste(glyph_img.convert("RGB"), (x, y))

    out_path.parent.mkdir(parents=True, exist_ok=True)
    img.save(str(out_path))
    print(f"[+] 미리보기 저장: {out_path}")


# ── 메인 ──────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="PokéRogue 한글 문자 추출 및 GBA 폰트 생성 도구"
    )
    parser.add_argument("--locales",    required=True,
                        help="pokerogue-locales/ko 디렉터리 경로")
    parser.add_argument("--font-ttf",   default=None,
                        help="한글 TTF 폰트 파일 경로 (없으면 더미 사용)")
    parser.add_argument("--output-dir", required=True,
                        help="출력 디렉터리 (src/data/rogue/)")
    parser.add_argument("--font-size",  type=int, default=7,
                        help="렌더링 폰트 크기 (기본: 7)")
    parser.add_argument("--preview",    action="store_true",
                        help="PNG 미리보기 생성")
    parser.add_argument("--max-chars",  type=int, default=MAX_CHARS,
                        help=f"최대 한글 자수 (기본: {MAX_CHARS})")
    args = parser.parse_args()

    locales_path = Path(args.locales)
    out_dir      = Path(args.output_dir)
    font_path    = Path(args.font_ttf) if args.font_ttf else None

    print("=" * 50)
    print("  PokéRogue GBA - 한글 문자 추출기")
    print("=" * 50)

    # 1. 한글 문자 추출
    chars = extract_chars_from_json(locales_path)
    print(f"[*] 추출된 한글 문자: {len(chars)}자")

    if len(chars) > args.max_chars:
        print(f"[!] 문자 수 초과 ({len(chars)} > {args.max_chars}). 빈도 상위 {args.max_chars}자만 사용.")
        # 빈도 계산을 위해 다시 스캔
        freq: Dict[str, int] = {}
        for jf in locales_path.rglob("*.json"):
            try:
                text = jf.read_text(encoding="utf-8")
                for ch in text:
                    if ch in chars:
                        freq[ch] = freq.get(ch, 0) + 1
            except Exception:
                pass
        sorted_chars = sorted(chars, key=lambda c: freq.get(c, 0), reverse=True)
        chars = set(sorted_chars[:args.max_chars])

    # 타일 인덱스 할당 (유니코드 오름차순)
    char_list = sorted(chars, key=lambda c: ord(c))
    char_map  = {ch: TILE_OFFSET + i for i, ch in enumerate(char_list)}

    print(f"[*] 최종 사용 문자: {len(char_list)}자")
    print(f"    타일 인덱스 범위: 0x{TILE_OFFSET:02X} ~ 0x{TILE_OFFSET + len(char_list) - 1:02X}")

    # 2. 폰트 렌더러 초기화
    renderer = GlyphRenderer(font_path, args.font_size)

    # 3. C 헤더 생성
    out_dir.mkdir(parents=True, exist_ok=True)

    table_path = out_dir / "hangul_table.generated.h"
    glyph_path = out_dir / "hangul_glyphs.generated.h"

    table_path.write_text(generate_table_header(char_map), encoding="utf-8")
    print(f"[+] 테이블 헤더 생성: {table_path}")

    glyph_path.write_text(generate_glyphs_header(char_list, renderer), encoding="utf-8")
    print(f"[+] 글리프 헤더 생성: {glyph_path}")

    # 4. 미리보기 (옵션)
    if args.preview:
        preview_path = Path("tools/preview/hangul_preview.png")
        generate_preview(char_map, renderer, preview_path)

    print("\n[완료] 한글 폰트 생성이 완료되었습니다.")
    print(f"       총 {len(char_list)}자 → GBA VRAM 타일 {len(char_list) * 32}바이트 사용")


if __name__ == "__main__":
    main()
