#!/usr/bin/env python3
# ============================================================
#  run_all.py
#  PokéRogue GBA - 전체 임포트 파이프라인 실행 스크립트
#
#  사용법:
#    python3 tools/pokerogue_import/run_all.py \
#        --pokerogue    /path/to/pokerogue \
#        --locales      /path/to/pokerogue-locales/ko \
#        --font         tools/fonts/NotoSansKR-Regular.ttf \
#        --output-root  .
#
#  실행 순서:
#    1. import_biomes.py     → src/data/rogue/biome_tables.h
#    2. extract_korean_chars.py → src/data/rogue/hangul_*.h
#    3. import_locales.py    → src/data/rogue/locale_ko.h
#    4. import_passives.py   → src/data/rogue/passive_data.generated.h
# ============================================================

import argparse
import subprocess
import sys
import time
from pathlib import Path

TOOLS_DIR = Path(__file__).parent

def run_step(label: str, cmd: list[str]) -> bool:
    """단계 실행 + 결과 출력"""
    print(f"\n{'='*50}")
    print(f"  [{label}]")
    print(f"  명령: {' '.join(cmd)}")
    print('='*50)

    start = time.time()
    result = subprocess.run(cmd, capture_output=False)
    elapsed = time.time() - start

    if result.returncode == 0:
        print(f"  ✓ 완료 ({elapsed:.1f}초)")
        return True
    else:
        print(f"  ✗ 실패 (코드 {result.returncode})")
        return False


def check_deps():
    """의존성 패키지 체크"""
    missing = []
    try:
        import PIL
    except ImportError:
        missing.append("pillow")

    if missing:
        print(f"[WARN] 누락된 패키지: {', '.join(missing)}")
        print(f"       pip install {' '.join(missing)}")
        print("       (폰트 렌더링은 더미 데이터로 진행됩니다)")
        return False
    return True


def main():
    parser = argparse.ArgumentParser(
        description="PokéRogue GBA 전체 데이터 임포트 파이프라인"
    )
    parser.add_argument("--pokerogue",   required=True,
                        help="PokéRogue 저장소 루트 경로")
    parser.add_argument("--locales",     default=None,
                        help="pokerogue-locales/ko 디렉터리 (없으면 한글 스킵)")
    parser.add_argument("--font",        default=None,
                        help="한글 TTF 폰트 파일 경로")
    parser.add_argument("--output-root", default=".",
                        help="출력 루트 디렉터리 (기본: 현재 폴더)")
    parser.add_argument("--skip-biomes",   action="store_true")
    parser.add_argument("--skip-hangul",   action="store_true")
    parser.add_argument("--skip-locales",  action="store_true")
    parser.add_argument("--skip-passives", action="store_true")
    parser.add_argument("--preview",       action="store_true",
                        help="한글 폰트 PNG 미리보기 생성")
    args = parser.parse_args()

    root      = Path(args.output_root)
    pokerogue = Path(args.pokerogue)
    data_out  = root / "src" / "data" / "rogue"

    print("=" * 50)
    print("  PokéRogue GBA - 데이터 임포트 파이프라인")
    print("=" * 50)
    print(f"  PokéRogue: {pokerogue}")
    print(f"  출력: {data_out}")

    check_deps()

    results = {}

    # ── 1. 바이옴 임포트 ─────────────────────────────────────
    if not args.skip_biomes:
        ok = run_step("바이옴 데이터 임포트", [
            sys.executable,
            str(TOOLS_DIR / "import_biomes.py"),
            "--pokerogue-src", str(pokerogue / "src"),
            "--output", str(data_out / "biome_tables.h"),
        ])
        results["biomes"] = ok
    else:
        print("\n[SKIP] 바이옴 임포트 건너뜀")

    # ── 2. 한글 문자 추출 ────────────────────────────────────
    if not args.skip_hangul:
        if args.locales:
            cmd = [
                sys.executable,
                str(TOOLS_DIR / "extract_korean_chars.py"),
                "--locales",    args.locales,
                "--output-dir", str(data_out),
            ]
            if args.font:
                cmd += ["--font-ttf", args.font]
            if args.preview:
                cmd += ["--preview"]

            ok = run_step("한글 문자 추출 + 폰트 생성", cmd)
            results["hangul"] = ok
        else:
            print("\n[SKIP] --locales 없음, 한글 추출 건너뜀")
            print("       더미 hangul_table.generated.h를 사용합니다.")
    else:
        print("\n[SKIP] 한글 추출 건너뜀")

    # ── 3. 로케일 변환 ───────────────────────────────────────
    if not args.skip_locales:
        if args.locales:
            ok = run_step("한글 로케일 변환", [
                sys.executable,
                str(TOOLS_DIR / "import_locales.py"),
                "--locales", args.locales,
                "--output",  str(data_out / "locale_ko.h"),
            ])
            results["locales"] = ok
        else:
            print("\n[SKIP] --locales 없음, 로케일 변환 건너뜀")
    else:
        print("\n[SKIP] 로케일 변환 건너뜀")

    # ── 4. 패시브 매핑 ───────────────────────────────────────
    if not args.skip_passives:
        ok = run_step("패시브 데이터 생성", [
            sys.executable,
            str(TOOLS_DIR / "import_passives.py"),
            "--pokerogue-src", str(pokerogue / "src"),
            "--output", str(data_out / "passive_data.generated.h"),
            "--report", str(root / "tools" / "passive_mapping.txt"),
        ])
        results["passives"] = ok
    else:
        print("\n[SKIP] 패시브 데이터 건너뜀")

    # ── 결과 요약 ────────────────────────────────────────────
    print(f"\n{'='*50}")
    print("  임포트 결과 요약")
    print('='*50)
    for key, ok in results.items():
        status = "✓" if ok else "✗"
        print(f"  {status} {key}")

    fail_count = sum(1 for ok in results.values() if not ok)
    if fail_count == 0:
        print(f"\n  모든 임포트 완료! 이제 'make -j4'로 빌드하세요.")
    else:
        print(f"\n  {fail_count}개 단계 실패. 위 오류를 확인하세요.")
        sys.exit(1)


if __name__ == "__main__":
    main()
