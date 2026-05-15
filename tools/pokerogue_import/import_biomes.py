#!/usr/bin/env python3
# ============================================================
#  import_biomes.py
#  PokéRogue GBA - 바이옴 데이터 자동 변환 도구
#
#  사용법:
#    python3 tools/pokerogue_import/import_biomes.py \
#        --pokerogue-src  /path/to/pokerogue/src \
#        --output         src/data/rogue/biome_tables.h
#
#  입력:
#    pokerogue/src/data/biomes.ts
#    pokerogue/src/enums/biome.ts
#
#  출력:
#    src/data/rogue/biome_tables.h  (biome_tables.h 완전 교체)
# ============================================================

import re
import os
import sys
import json
import argparse
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Dict, Optional, Tuple

# ── 데이터 구조 ───────────────────────────────────────────────

@dataclass
class SpawnEntry:
    species: str
    weight: int
    min_level: int
    max_level: int
    form: int = 0
    shiny_locked: bool = False

@dataclass
class BossEntry:
    species: str
    level: int
    moves: List[str] = field(default_factory=list)
    held_item: str = "ITEM_NONE"
    nature: str = "NATURE_HARDY"
    form: int = 0

@dataclass
class RewardEntry:
    item_id: str
    weight: int

@dataclass
class BiomeData:
    name: str
    spawn_table: List[SpawnEntry] = field(default_factory=list)
    boss_table: List[BossEntry] = field(default_factory=list)
    reward_table: List[RewardEntry] = field(default_factory=list)
    next_biomes: List[str] = field(default_factory=list)
    next_weights: List[int] = field(default_factory=list)
    min_wave: int = 1

# ── PokéRogue → GBA 이름 변환 매핑 ───────────────────────────

SPECIES_MAP = {
    # 변환이 필요한 특수 케이스만 정의
    # 나머지는 "Species.XXX" → "SPECIES_XXX" 자동 변환
    "NidoranF":   "NIDORAN_F",
    "NidoranM":   "NIDORAN_M",
    "Farfetchd":  "FARFETCHD",
    "MrMime":     "MR_MIME",
    "HoOh":       "HO_OH",
    "MimeJr":     "MIME_JR",
    "Porygon2":   "PORYGON2",
    "PorygonZ":   "PORYGON_Z",
    "Jangmoo":    "JANGMO_O",
    "Komoo":      "KOMMO_O",
    "Hakamoo":    "HAKAMO_O",
    "MrRime":     "MR_RIME",
}

MOVE_MAP = {
    "HyperFang":     "HYPER_FANG",
    "WingAttack":    "WING_ATTACK",
    "SolarBeam":     "SOLAR_BEAM",
    "SleepPowder":   "SLEEP_POWDER",
    "LeechSeed":     "LEECH_SEED",
    "BulletPunch":   "BULLET_PUNCH",
    "DragonDance":   "DRAGON_DANCE",
    "StoneEdge":     "STONE_EDGE",
    "CloseCombat":   "CLOSE_COMBAT",
    "NastyPlot":     "NASTY_PLOT",
    "WillOWisp":     "WILL_O_WISP",
    "AirSlash":      "AIR_SLASH",
    "DragonPulse":   "DRAGON_PULSE",
    "FocusBlast":    "FOCUS_BLAST",
    "ShadowBall":    "SHADOW_BALL",
    "EarthPower":    "EARTH_POWER",
    "IceBeam":       "ICE_BEAM",
    "FlashCannon":   "FLASH_CANNON",
    "PsychoBoosted": "PSYCHO_BOOST",
}

ITEM_MAP = {
    "MIRACLE_SEED":  "ITEM_MIRACLE_SEED",
    "CHARCOAL":      "ITEM_CHARCOAL",
    "MYSTIC_WATER":  "ITEM_MYSTIC_WATER",
    "MAGNET":        "ITEM_MAGNET",
    "TWISTED_SPOON": "ITEM_TWISTED_SPOON",
    "NEVER_MELT_ICE":"ITEM_NEVER_MELT_ICE",
    "SOFT_SAND":     "ITEM_SOFT_SAND",
    "HARD_STONE":    "ITEM_HARD_STONE",
    "SILVER_POWDER": "ITEM_SILVER_POWDER",
    "SHARP_BEAK":    "ITEM_SHARP_BEAK",
    "POISON_BARB":   "ITEM_POISON_BARB",
    "SPELL_TAG":     "ITEM_SPELL_TAG",
    "BLACK_BELT":    "ITEM_BLACK_BELT",
    "SILK_SCARF":    "ITEM_SILK_SCARF",
    "METAL_COAT":    "ITEM_METAL_COAT",
    "DRAGON_FANG":   "ITEM_DRAGON_FANG",
    "NONE":          "ITEM_NONE",
}

BIOME_MAP = {
    "Biome.TOWN":     "BIOME_TOWN",
    "Biome.GRASS":    "BIOME_GRASS",
    "Biome.FOREST":   "BIOME_FOREST",
    "Biome.SEA":      "BIOME_OCEAN",
    "Biome.SWAMP":    "BIOME_SWAMP",
    "Biome.BEACH":    "BIOME_BEACH",
    "Biome.LAKE":     "BIOME_SWAMP",  # 폴백
    "Biome.SEABED":   "BIOME_OCEAN",
    "Biome.MOUNTAIN": "BIOME_MOUNTAIN",
    "Biome.BADLANDS": "BIOME_DESERT",
    "Biome.CAVE":     "BIOME_CAVE",
    "Biome.DESERT":   "BIOME_DESERT",
    "Biome.ICE_CAVE": "BIOME_ICE",
    "Biome.MEADOW":   "BIOME_GRASS",
    "Biome.POWER_PLANT": "BIOME_FACTORY",
    "Biome.VOLCANO":  "BIOME_VOLCANO",
    "Biome.GRAVEYARD":"BIOME_RUINS",
    "Biome.DOJO":     "BIOME_MOUNTAIN",
    "Biome.FACTORY":  "BIOME_FACTORY",
    "Biome.RUINS":    "BIOME_RUINS",
    "Biome.WASTELAND":"BIOME_DESERT",
    "Biome.ABYSS":    "BIOME_CAVE",
    "Biome.SPACE":    "BIOME_SPACE",
    "Biome.CONSTRUCTION_SITE": "BIOME_FACTORY",
    "Biome.JUNGLE":   "BIOME_FOREST",
    "Biome.FAIRY_CAVE": "BIOME_CAVE",
    "Biome.TEMPLE":   "BIOME_RUINS",
    "Biome.SLUM":     "BIOME_FACTORY",
    "Biome.SNOWY_FOREST": "BIOME_ICE",
    "Biome.ISLAND":   "BIOME_BEACH",
    "Biome.LABORATORY": "BIOME_FACTORY",
    "Biome.END":      "BIOME_FINAL",
}

NATURE_MAP = {
    "Nature.ADAMANT": "NATURE_ADAMANT",
    "Nature.JOLLY":   "NATURE_JOLLY",
    "Nature.TIMID":   "NATURE_TIMID",
    "Nature.MODEST":  "NATURE_MODEST",
    "Nature.BOLD":    "NATURE_BOLD",
    "Nature.RELAXED": "NATURE_RELAXED",
    "Nature.IMPISH":  "NATURE_IMPISH",
    "Nature.QUIET":   "NATURE_QUIET",
    "Nature.HASTY":   "NATURE_HASTY",
    "Nature.HARDY":   "NATURE_HARDY",
    "Nature.BRAVE":   "NATURE_BRAVE",
    "Nature.RASH":    "NATURE_RASH",
}

# ── 이름 변환 유틸 ────────────────────────────────────────────

def camel_to_upper(name: str) -> str:
    """CamelCase → UPPER_SNAKE_CASE"""
    s = re.sub(r'([A-Z])', r'_\1', name).upper().lstrip('_')
    return s

def species_to_gba(ts_name: str) -> str:
    """Species.Bulbasaur → SPECIES_BULBASAUR"""
    # "Species.Xxx" 형식 처리
    name = ts_name.replace("Species.", "").strip()
    if name in SPECIES_MAP:
        return f"SPECIES_{SPECIES_MAP[name]}"
    return f"SPECIES_{camel_to_upper(name)}"

def move_to_gba(ts_name: str) -> str:
    """Moves.SolarBeam → MOVE_SOLAR_BEAM"""
    name = ts_name.replace("Moves.", "").strip()
    if name in MOVE_MAP:
        return f"MOVE_{MOVE_MAP[name]}"
    return f"MOVE_{camel_to_upper(name)}"

def item_to_gba(ts_name: str) -> str:
    """Items.MIRACLE_SEED → ITEM_MIRACLE_SEED"""
    name = ts_name.replace("Items.", "").strip()
    if name in ITEM_MAP:
        return ITEM_MAP[name]
    return f"ITEM_{name.upper()}"

# ── TypeScript 파서 ───────────────────────────────────────────

class BiomeParser:
    """
    PokéRogue의 biomes.ts를 파싱하여 BiomeData 목록으로 변환.
    완벽한 TS 파서가 아닌 패턴 기반 추출 방식.
    """

    def __init__(self, src_path: Path):
        self.src_path = src_path
        self.biomes: Dict[str, BiomeData] = {}

    def parse(self) -> Dict[str, BiomeData]:
        biomes_file = self.src_path / "data" / "biomes.ts"
        if not biomes_file.exists():
            # 대안 경로 시도
            biomes_file = self.src_path / "data" / "biome.ts"
        if not biomes_file.exists():
            print(f"[WARN] biomes.ts not found at {biomes_file}")
            return self._create_fallback()

        text = biomes_file.read_text(encoding="utf-8")
        return self._parse_text(text)

    def _parse_text(self, text: str) -> Dict[str, BiomeData]:
        """
        biomes.ts 구조 예시:
          [Biome.GRASS]: {
            pokemon: [
              { pokemon: Species.Bulbasaur, weight: 10 },
              ...
            ],
            trainers: [...],
            ...
          }
        """
        biomes = {}

        # Biome 블록 추출 패턴
        block_pattern = re.compile(
            r'\[Biome\.(\w+)\]\s*:\s*\{(.*?)\}(?=\s*,?\s*\[Biome\.|$)',
            re.DOTALL
        )

        for m in block_pattern.finditer(text):
            biome_name = m.group(1)
            block      = m.group(2)
            gba_biome  = BIOME_MAP.get(f"Biome.{biome_name}", f"BIOME_{biome_name}")

            bd = BiomeData(name=gba_biome)
            self._parse_pokemon_block(block, bd)
            self._parse_boss_block(block, bd)
            self._parse_links(block, bd)

            biomes[gba_biome] = bd
            print(f"  [+] Parsed biome: {biome_name} → {gba_biome} "
                  f"({len(bd.spawn_table)} spawns, {len(bd.boss_table)} bosses)")

        return biomes

    def _parse_pokemon_block(self, block: str, bd: BiomeData):
        """pokemon: [ { pokemon: Species.Xxx, weight: N }, ... ] 파싱"""
        pkmn_section = re.search(r'pokemon\s*:\s*\[(.*?)\]', block, re.DOTALL)
        if not pkmn_section:
            return

        entry_pattern = re.compile(
            r'\{\s*pokemon\s*:\s*([\w.]+)\s*(?:,\s*weight\s*:\s*(\d+))?\s*\}',
            re.DOTALL
        )
        for m in entry_pattern.finditer(pkmn_section.group(1)):
            species = species_to_gba(m.group(1))
            weight  = int(m.group(2)) if m.group(2) else 20
            # 레벨은 기본값 (biome_tables.h 수동 조정 또는 별도 레벨 파일 파싱)
            bd.spawn_table.append(SpawnEntry(
                species=species, weight=min(weight, 255),
                min_level=5, max_level=10
            ))

    def _parse_boss_block(self, block: str, bd: BiomeData):
        """bosses 또는 trainer 블록에서 보스 포켓몬 추출"""
        boss_section = re.search(r'bosses\s*:\s*\[(.*?)\]', block, re.DOTALL)
        if not boss_section:
            return

        boss_pattern = re.compile(
            r'new\s+PokemonSpecies\s*\(\s*([\w.]+)',
            re.DOTALL
        )
        for m in boss_pattern.finditer(boss_section.group(1)):
            species = species_to_gba(m.group(1))
            bd.boss_table.append(BossEntry(
                species=species,
                level=50,   # 기본값
                moves=["MOVE_NONE", "MOVE_NONE", "MOVE_NONE", "MOVE_NONE"],
            ))

    def _parse_links(self, block: str, bd: BiomeData):
        """links: [ [Biome.XXX, N], ... ] 파싱"""
        link_section = re.search(r'links\s*:\s*\[(.*?)\]', block, re.DOTALL)
        if not link_section:
            return

        link_pattern = re.compile(r'\[\s*(Biome\.\w+)\s*,\s*(\d+)\s*\]')
        for m in link_pattern.finditer(link_section.group(1)):
            gba_biome = BIOME_MAP.get(m.group(1), "BIOME_GRASS")
            weight    = int(m.group(2))
            bd.next_biomes.append(gba_biome)
            bd.next_weights.append(weight)

    def _create_fallback(self) -> Dict[str, BiomeData]:
        """파일을 찾지 못한 경우 기본 데이터 반환"""
        print("[WARN] Using fallback biome data (biomes.ts not found)")
        return {}


# ── C 코드 생성기 ─────────────────────────────────────────────

class CGenerator:
    def __init__(self, biomes: Dict[str, BiomeData]):
        self.biomes = biomes

    def generate(self) -> str:
        lines = []
        lines.append("// AUTO-GENERATED by import_biomes.py — DO NOT EDIT")
        lines.append("// Run: python3 tools/pokerogue_import/import_biomes.py")
        lines.append("")
        lines.append("#ifndef BIOME_TABLES_H")
        lines.append("#define BIOME_TABLES_H")
        lines.append("")
        lines.append('#include "constants/species.h"')
        lines.append('#include "constants/moves.h"')
        lines.append('#include "constants/items.h"')
        lines.append('#include "rogue/rogue_biome.h"')
        lines.append("")

        for biome_name, bd in self.biomes.items():
            lines.extend(self._gen_biome(biome_name, bd))
            lines.append("")

        lines.append("#endif // BIOME_TABLES_H")
        return "\n".join(lines)

    def _gen_biome(self, biome_name: str, bd: BiomeData) -> List[str]:
        lines = []
        macro = biome_name.replace("BIOME_", "")
        prefix = f"gBiome{macro.title().replace('_', '')}"

        # 스폰 테이블
        lines.append(f"// ── {biome_name} ────────────────────")
        lines.append(f"static const struct RogueSpawnEntry {prefix}Spawn[] =")
        lines.append("{")
        if bd.spawn_table:
            for e in bd.spawn_table:
                lock = "1" if e.shiny_locked else "0"
                lines.append(f"    {{ {e.species}, {e.weight},"
                              f" {e.min_level}, {e.max_level},"
                              f" {e.form}, {lock}, 0 }},")
        else:
            # 최소 1개 엔트리 필요
            lines.append("    { SPECIES_RATTATA, 50, 5, 10, 0, 0, 0 },")
        lines.append("};")
        lines.append("")

        # 보스 테이블
        lines.append(f"static const struct RogueBossEntry {prefix}Boss[] =")
        lines.append("{")
        if bd.boss_table:
            for b in bd.boss_table:
                moves = b.moves[:4] + ["MOVE_NONE"] * (4 - len(b.moves))
                lines.append("    {")
                lines.append(f"        .species  = {b.species},")
                lines.append(f"        .level    = {b.level},")
                lines.append(f"        .moves    = {{ {', '.join(moves[:4])} }},")
                lines.append(f"        .heldItem = {b.held_item},")
                lines.append(f"        .nature   = {b.nature},")
                lines.append("    },")
        else:
            lines.append("    { .species = SPECIES_RATTATA, .level = 30,"
                         " .moves = { MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE },"
                         " .heldItem = ITEM_NONE, .nature = NATURE_HARDY },")
        lines.append("};")
        lines.append("")

        # 보상 테이블 (기본값)
        lines.append(f"static const struct RogueBiomeReward {prefix}Reward[] =")
        lines.append("{")
        if bd.reward_table:
            for r in bd.reward_table:
                lines.append(f"    {{ {r.item_id}, {r.weight}, 0 }},")
        else:
            lines.append("    { ITEM_SUPER_POTION, 50, 0 },")
            lines.append("    { ITEM_GREAT_BALL,   50, 0 },")
        lines.append("};")

        return lines


# ── 메인 ──────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="PokéRogue biome data → GBA C header 변환 도구"
    )
    parser.add_argument("--pokerogue-src", required=True,
                        help="PokéRogue 소스 루트 (src/ 폴더 포함)")
    parser.add_argument("--output", required=True,
                        help="출력 C 헤더 파일 경로")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    src_path = Path(args.pokerogue_src)
    out_path = Path(args.output)

    print(f"[*] PokéRogue 소스: {src_path}")
    print(f"[*] 출력: {out_path}")

    # 파싱
    bp = BiomeParser(src_path)
    biomes = bp.parse()

    if not biomes:
        print("[!] 바이옴 데이터를 파싱하지 못했습니다. 기존 biome_tables.h를 유지합니다.")
        sys.exit(1)

    # C 코드 생성
    gen = CGenerator(biomes)
    code = gen.generate()

    # 출력 디렉터리 생성
    out_path.parent.mkdir(parents=True, exist_ok=True)

    # 기존 파일 백업
    if out_path.exists():
        backup = out_path.with_suffix(".h.bak")
        out_path.rename(backup)
        print(f"[*] 기존 파일 백업: {backup}")

    out_path.write_text(code, encoding="utf-8")
    print(f"[+] 생성 완료: {out_path}")
    print(f"    총 {len(biomes)}개 바이옴 변환됨")


if __name__ == "__main__":
    main()
