#!/usr/bin/env python3
# ============================================================
#  import_passives.py
#  PokéRogue GBA - 패시브(어빌리티) 데이터 자동 변환 도구
#
#  사용법:
#    python3 tools/pokerogue_import/import_passives.py \
#        --pokerogue-src /path/to/pokerogue/src \
#        --output        src/data/rogue/passive_data.generated.h
#
#  입력:
#    pokerogue/src/data/ability.ts  (어빌리티 효과 정의)
#    pokerogue/src/enums/abilities.ts (어빌리티 ID 열거형)
#
#  출력:
#    passive_data.generated.h  (패시브 참고/확장용 주석)
#    passive_mapping.txt        (TS 어빌리티 → GBA 패시브 매핑 보고서)
# ============================================================

import re
import argparse
import sys
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Dict, Optional, Tuple

# ── 데이터 구조 ───────────────────────────────────────────────

@dataclass
class AbilityEntry:
    ts_name: str        # TypeScript enum 이름 (e.g. "SPEED_BOOST")
    gba_passive: str    # 대응 GBA 패시브 ID (e.g. "PASSIVE_SPEED_BOOST")
    max_stacks: int = 1
    weight: int = 20
    atk_mod: int = 1024    # Q10 고정소수점
    def_mod: int = 1024
    spa_mod: int = 1024
    spd_mod: int = 1024
    speed_mod: int = 1024
    trigger: str = "TRIGGER_BATTLE_START"
    trigger_param: int = 0
    name_ko: str = ""
    desc_ko: str = ""
    notes: str = ""     # 구현 노트

# ── TS → GBA 어빌리티 매핑 테이블 ────────────────────────────
# PokéRogue 어빌리티 ID → GBA 패시브 파라미터
# weight=0 → 보상 풀 미등장 (구현 불완전 또는 제외)

ABILITY_TO_PASSIVE: Dict[str, AbilityEntry] = {
    "SPEED_BOOST": AbilityEntry(
        ts_name="SPEED_BOOST",
        gba_passive="PASSIVE_SPEED_BOOST",
        max_stacks=3, weight=30,
        trigger="TRIGGER_TURN_START",
        name_ko="가속", desc_ko="매 턴 스피드가 1단계 상승한다.",
        notes="gBattleMons[].statStages[STAT_SPEED]++",
    ),
    "DROUGHT": AbilityEntry(
        ts_name="DROUGHT",
        gba_passive="PASSIVE_DROUGHT",
        max_stacks=1, weight=20,
        trigger="TRIGGER_BATTLE_START", trigger_param=2,  # WEATHER_SUNNY
        name_ko="가뭄", desc_ko="전투 시작 시 쾌청을 일으킨다.",
        notes="TryChangeBattleWeather(WEATHER_SUNNY)",
    ),
    "DRIZZLE": AbilityEntry(
        ts_name="DRIZZLE",
        gba_passive="PASSIVE_DRIZZLE",
        max_stacks=1, weight=20,
        trigger="TRIGGER_BATTLE_START", trigger_param=3,  # WEATHER_RAIN
        name_ko="잔비", desc_ko="전투 시작 시 비를 내리게 한다.",
    ),
    "GUTS": AbilityEntry(
        ts_name="GUTS",
        gba_passive="PASSIVE_GUTS",
        max_stacks=1, weight=25,
        atk_mod=1536,  # 1.5배
        trigger="TRIGGER_ON_HIT",
        name_ko="근성", desc_ko="상태이상 시 공격이 1.5배 오른다.",
    ),
    "HUSTLE": AbilityEntry(
        ts_name="HUSTLE",
        gba_passive="PASSIVE_HUSTLE",
        max_stacks=1, weight=20,
        atk_mod=1536,  # 물리 1.5배 (명중 0.8 패널티는 전투 엔진에서 처리)
        trigger="TRIGGER_ON_ATTACK",
        name_ko="우격다짐", desc_ko="물리 공격 1.5배, 명중 0.8배.",
    ),
    "THICK_FAT": AbilityEntry(
        ts_name="THICK_FAT",
        gba_passive="PASSIVE_THICK_FAT",
        max_stacks=1, weight=25,
        spa_mod=512, spd_mod=512,  # 불꽃/얼음 0.5배 (특수 타입 체크 필요)
        trigger="TRIGGER_ON_HIT",
        name_ko="두꺼운지방", desc_ko="불꽃/얼음 데미지를 절반으로 줄인다.",
    ),
    "REGENERATOR": AbilityEntry(
        ts_name="REGENERATOR",
        gba_passive="PASSIVE_REGENERATOR",
        max_stacks=1, weight=30,
        trigger="TRIGGER_ON_SWITCH",
        name_ko="재생력", desc_ko="교체 시 HP 1/3 회복.",
    ),
    "MULTISCALE": AbilityEntry(
        ts_name="MULTISCALE",
        gba_passive="PASSIVE_MULTISCALE",
        max_stacks=1, weight=15,
        trigger="TRIGGER_ON_HIT",
        name_ko="멀티스케일", desc_ko="만피 시 받는 데미지가 절반이 된다.",
    ),
    "ADAPTABILITY": AbilityEntry(
        ts_name="ADAPTABILITY",
        gba_passive="PASSIVE_ADAPTABILITY",
        max_stacks=1, weight=20,
        trigger="TRIGGER_ON_ATTACK",
        name_ko="적응력", desc_ko="자속보정이 2.0배가 된다.",
        notes="gBattleMoveDamage *= 2 / 1.5 → battleScript 수정 필요",
    ),
    "SHEER_FORCE": AbilityEntry(
        ts_name="SHEER_FORCE",
        gba_passive="PASSIVE_SHEER_FORCE",
        max_stacks=1, weight=20,
        atk_mod=1331, spa_mod=1331,  # 1.3배
        trigger="TRIGGER_ON_ATTACK",
        name_ko="저돌맞기", desc_ko="추가효과를 제거하고 데미지 1.3배.",
    ),
    "ANALYTIC": AbilityEntry(
        ts_name="ANALYTIC",
        gba_passive="PASSIVE_ANALYTIC",
        max_stacks=1, weight=15,
        atk_mod=1331, spa_mod=1331,
        trigger="TRIGGER_ON_ATTACK",
        name_ko="분석", desc_ko="나중에 행동하면 데미지 1.3배.",
    ),
    "TINTED_LENS": AbilityEntry(
        ts_name="TINTED_LENS",
        gba_passive="PASSIVE_TINTED_LENS",
        max_stacks=1, weight=18,
        trigger="TRIGGER_ON_ATTACK",
        name_ko="색안경", desc_ko="효과 반감 기술이 2배 데미지.",
    ),
    "MOXIE": AbilityEntry(
        ts_name="MOXIE",
        gba_passive="PASSIVE_MOXIE",
        max_stacks=6, weight=22,
        trigger="TRIGGER_ON_KO",
        name_ko="자기과신", desc_ko="KO 시마다 공격 +1.",
    ),
    "INTIMIDATE": AbilityEntry(
        ts_name="INTIMIDATE",
        gba_passive="PASSIVE_INTIMIDATE",
        max_stacks=1, weight=25,
        trigger="TRIGGER_BATTLE_START",
        name_ko="위협", desc_ko="전투 시작 시 상대 공격 -1.",
    ),

    # ── 구현 예정 (weight=0으로 비활성) ──────────────────────
    "WONDER_GUARD": AbilityEntry(
        ts_name="WONDER_GUARD",
        gba_passive="PASSIVE_WONDER_GUARD",
        max_stacks=1, weight=0,
        trigger="TRIGGER_ON_HIT",
        name_ko="불가사의수비", desc_ko="효과 없음/보통 효과 기술 무효.",
        notes="TODO: battle_script에서 타입 체크 후 무효화",
    ),
    "MAGIC_GUARD": AbilityEntry(
        ts_name="MAGIC_GUARD",
        gba_passive="PASSIVE_NONE",
        max_stacks=0, weight=0,
        name_ko="매직가드", notes="TODO: 간접 데미지 무효화",
    ),
    "SAND_STREAM": AbilityEntry(
        ts_name="SAND_STREAM",
        gba_passive="PASSIVE_NONE",
        max_stacks=0, weight=0,
        name_ko="모래올리기", notes="TODO: PASSIVE_DROUGHT 구조 복제",
    ),
    "SNOW_WARNING": AbilityEntry(
        ts_name="SNOW_WARNING",
        gba_passive="PASSIVE_NONE",
        max_stacks=0, weight=0,
        name_ko="눈퍼뜨리기", notes="TODO: 싸라기눈 날씨",
    ),
    "LIBERO": AbilityEntry(
        ts_name="LIBERO",
        gba_passive="PASSIVE_NONE",
        max_stacks=0, weight=0,
        name_ko="리베로", notes="TODO: 기술 타입으로 자신 타입 변경",
    ),
    "PROTEAN": AbilityEntry(
        ts_name="PROTEAN",
        gba_passive="PASSIVE_NONE",
        max_stacks=0, weight=0,
        name_ko="변환자재", notes="TODO: LIBERO와 동일",
    ),
}

# ── C 코드 생성 ───────────────────────────────────────────────

def generate_passive_header(entries: Dict[str, AbilityEntry]) -> str:
    """passive_data.generated.h 생성 (참고/확장용 주석 파일)"""
    lines = []
    lines.append("// AUTO-GENERATED by import_passives.py — DO NOT EDIT")
    lines.append("// 이 파일은 rogue_passive.c의 gPassiveDefs[] 배열을")
    lines.append("// 자동으로 채우는 데이터 소스입니다.")
    lines.append("// 실제 구현은 rogue_passive.c를 직접 수정하세요.")
    lines.append("")
    lines.append("// ── 매핑 현황 ────────────────────────────────────────")

    implemented   = [e for e in entries.values() if e.weight > 0 and e.gba_passive != "PASSIVE_NONE"]
    todo          = [e for e in entries.values() if e.weight == 0 or e.gba_passive == "PASSIVE_NONE"]

    lines.append(f"// 구현 완료: {len(implemented)}개")
    lines.append(f"// 구현 예정: {len(todo)}개")
    lines.append("")

    lines.append("// ── 구현 완료 ────────────────────────────────────────")
    for e in implemented:
        lines.append(f"// [{e.ts_name}] → {e.gba_passive}")
        lines.append(f"//   이름: {e.name_ko}")
        lines.append(f"//   중첩: {e.max_stacks}  가중치: {e.weight}")
        lines.append(f"//   공배율: {e.atk_mod}/1024  특공배율: {e.spa_mod}/1024")
        if e.notes:
            lines.append(f"//   구현노트: {e.notes}")
        lines.append("")

    lines.append("// ── 구현 예정 ────────────────────────────────────────")
    for e in todo:
        lines.append(f"// [TODO] {e.ts_name} → {e.name_ko}")
        if e.notes:
            lines.append(f"//        {e.notes}")

    return "\n".join(lines)


def generate_mapping_report(entries: Dict[str, AbilityEntry]) -> str:
    """사람이 읽기 좋은 매핑 보고서"""
    lines = []
    lines.append("=" * 60)
    lines.append("  PokéRogue GBA - 어빌리티 → 패시브 매핑 보고서")
    lines.append("=" * 60)
    lines.append("")

    for name, e in sorted(entries.items()):
        status = "✓" if (e.weight > 0 and e.gba_passive != "PASSIVE_NONE") else "○"
        lines.append(f"  {status} {e.ts_name:25s} → {e.gba_passive}")
        if e.name_ko:
            lines.append(f"      [{e.name_ko}] weight={e.weight} stacks={e.max_stacks}")

    lines.append("")
    lines.append(f"  총 {len(entries)}개 어빌리티 분석됨")
    return "\n".join(lines)


# ── TS 파일에서 추가 어빌리티 ID 추출 ────────────────────────

def extract_ability_ids(src_path: Path) -> List[str]:
    """abilities.ts enum에서 어빌리티 ID 목록 추출"""
    ability_file = src_path / "enums" / "abilities.ts"
    if not ability_file.exists():
        ability_file = src_path / "data" / "abilities.ts"
    if not ability_file.exists():
        return []

    text = ability_file.read_text(encoding="utf-8")
    pattern = re.compile(r'\b([A-Z][A-Z0-9_]+)\s*=\s*\d+')
    ids = pattern.findall(text)
    return [i for i in ids if not i.startswith("_")]


# ── 메인 ──────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="PokéRogue 어빌리티 → GBA 패시브 매핑 도구"
    )
    parser.add_argument("--pokerogue-src", required=True,
                        help="PokéRogue src/ 디렉터리")
    parser.add_argument("--output", required=True,
                        help="출력 C 헤더 파일 경로")
    parser.add_argument("--report", default=None,
                        help="매핑 보고서 텍스트 파일 출력 경로")
    parser.add_argument("--list-unmapped", action="store_true",
                        help="매핑 안 된 어빌리티 목록 출력")
    args = parser.parse_args()

    src_path = Path(args.pokerogue_src)
    out_path = Path(args.output)

    print(f"[*] PokéRogue src: {src_path}")

    # TS에서 추가 어빌리티 ID 추출
    ts_ids = extract_ability_ids(src_path)
    if ts_ids:
        print(f"[*] TS에서 {len(ts_ids)}개 어빌리티 ID 발견")

        if args.list_unmapped:
            unmapped = [i for i in ts_ids if i not in ABILITY_TO_PASSIVE]
            print(f"\n  매핑 안 된 어빌리티 ({len(unmapped)}개):")
            for name in unmapped[:30]:
                print(f"    - {name}")
            if len(unmapped) > 30:
                print(f"    ... 외 {len(unmapped) - 30}개")

    # 헤더 생성
    code = generate_passive_header(ABILITY_TO_PASSIVE)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(code, encoding="utf-8")
    print(f"[+] 패시브 데이터 헤더 생성: {out_path}")

    # 보고서 생성 (옵션)
    if args.report:
        report = generate_mapping_report(ABILITY_TO_PASSIVE)
        Path(args.report).write_text(report, encoding="utf-8")
        print(f"[+] 매핑 보고서: {args.report}")

    # 요약
    impl_count = sum(1 for e in ABILITY_TO_PASSIVE.values()
                     if e.weight > 0 and e.gba_passive != "PASSIVE_NONE")
    print(f"\n[완료] {impl_count}/{len(ABILITY_TO_PASSIVE)}개 패시브 구현됨")


if __name__ == "__main__":
    main()
