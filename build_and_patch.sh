#!/usr/bin/env bash
# ============================================================
#  build_and_patch.sh
#  로컬에서 빌드 + xdelta 패치 생성 자동화 스크립트
#
#  사용법:
#    chmod +x build_and_patch.sh
#    ./build_and_patch.sh
#    ./build_and_patch.sh --skip-build   # 빌드 건너뛰고 패치만
#    ./build_and_patch.sh --jobs 8       # 병렬 빌드 수 지정
# ============================================================

set -e

# ── 파라미터 ──────────────────────────────────────────────────
SKIP_BUILD=0
JOBS=$(nproc 2>/dev/null || echo 4)
BASEROM="baserom.gba"
OUTPUT_ROM="pokeemerald.gba"
PATCH_OUT="pokerogue-gba.xdelta"
EXPECTED_SHA1="f3ae088181bf583e55daf962a92bb46f4f1d07b7"

while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-build)  SKIP_BUILD=1 ;;
        --jobs|-j)     JOBS="$2"; shift ;;
        --baserom)     BASEROM="$2"; shift ;;
        --output)      PATCH_OUT="$2"; shift ;;
        *) echo "알 수 없는 옵션: $1"; exit 1 ;;
    esac
    shift
done

echo "============================================"
echo "  PokéRogue GBA 빌드 스크립트"
echo "============================================"

# ── 환경 체크 ─────────────────────────────────────────────────
check_tool() {
    if ! command -v "$1" &>/dev/null; then
        echo "오류: $1 이 없습니다."
        echo "설치: $2"
        exit 1
    fi
}

check_tool "arm-none-eabi-gcc" \
    "DevKitPro: https://devkitpro.org/wiki/Getting_Started"
check_tool "xdelta3" \
    "sudo apt install xdelta3  (또는 brew install xdelta)"
check_tool "make" \
    "sudo apt install build-essential"

echo "✓ 도구 확인 완료"
echo "  arm-none-eabi-gcc: $(arm-none-eabi-gcc --version | head -1)"
echo "  xdelta3: $(xdelta3 -V 2>/dev/null | head -1)"

# ── baserom 확인 ──────────────────────────────────────────────
if [ ! -f "$BASEROM" ]; then
    echo ""
    echo "오류: $BASEROM 파일이 없습니다."
    echo "포켓몬 에메랄드 영문판 ROM을 '$BASEROM' 로 복사하세요."
    exit 1
fi

ACTUAL_SHA1=$(sha1sum "$BASEROM" | awk '{print $1}')
if [ "$ACTUAL_SHA1" = "$EXPECTED_SHA1" ]; then
    echo "✓ baserom SHA1 확인"
else
    echo "⚠ baserom SHA1 불일치"
    echo "  예상: $EXPECTED_SHA1"
    echo "  실제: $ACTUAL_SHA1"
    echo "  계속하시겠습니까? (y/N)"
    read -r answer
    [ "$answer" = "y" ] || exit 1
fi

# ── 빌드 ──────────────────────────────────────────────────────
if [ "$SKIP_BUILD" = "0" ]; then
    echo ""
    echo "빌드 시작 (jobs=$JOBS)..."
    START_TIME=$(date +%s)

    make -j"$JOBS"

    END_TIME=$(date +%s)
    ELAPSED=$((END_TIME - START_TIME))
    echo "✓ 빌드 완료 (${ELAPSED}초)"
    echo "  ROM 크기: $(wc -c < "$OUTPUT_ROM") bytes"
else
    echo "빌드 건너뜀 (--skip-build)"
fi

# ── ROM 확인 ──────────────────────────────────────────────────
if [ ! -f "$OUTPUT_ROM" ]; then
    echo "오류: $OUTPUT_ROM 가 없습니다. 빌드를 먼저 실행하세요."
    exit 1
fi

# ── xdelta3 패치 생성 ─────────────────────────────────────────
echo ""
echo "xdelta3 패치 생성 중..."
xdelta3 -e -s "$BASEROM" "$OUTPUT_ROM" "$PATCH_OUT"

PATCH_SIZE=$(wc -c < "$PATCH_OUT")
ROM_SIZE=$(wc -c < "$OUTPUT_ROM")
echo "✓ 패치 생성 완료"
echo "  ROM 크기:  $ROM_SIZE bytes"
echo "  패치 크기: $PATCH_SIZE bytes ($(( PATCH_SIZE * 100 / ROM_SIZE ))%)"

# ── 패치 검증 ─────────────────────────────────────────────────
echo ""
echo "패치 검증 중..."
VERIFY_ROM="$(mktemp).gba"
xdelta3 -d -s "$BASEROM" "$PATCH_OUT" "$VERIFY_ROM"

ORIGINAL_SHA=$(sha1sum "$OUTPUT_ROM"  | awk '{print $1}')
PATCHED_SHA=$(sha1sum  "$VERIFY_ROM"  | awk '{print $1}')
rm "$VERIFY_ROM"

if [ "$ORIGINAL_SHA" = "$PATCHED_SHA" ]; then
    echo "✓ 패치 검증 성공"
    echo "  SHA1: $ORIGINAL_SHA"
else
    echo "✗ 패치 검증 실패!"
    echo "  원본 SHA1: $ORIGINAL_SHA"
    echo "  패치 SHA1: $PATCHED_SHA"
    exit 1
fi

# ── 체크섬 파일 생성 ──────────────────────────────────────────
sha1sum "$PATCH_OUT"  > "${PATCH_OUT}.sha1"
md5sum  "$OUTPUT_ROM" > "${OUTPUT_ROM}.md5"

# ── 결과 요약 ─────────────────────────────────────────────────
echo ""
echo "============================================"
echo "  완료!"
echo "============================================"
echo ""
echo "생성된 파일:"
echo "  $PATCH_OUT         ← 배포용 패치"
echo "  ${PATCH_OUT}.sha1  ← 체크섬"
echo ""
echo "사용자 배포 방법:"
echo "  xdelta3 -d -s 에메랄드.gba $PATCH_OUT 결과.gba"
echo ""
echo "mGBA로 바로 테스트:"
echo "  mgba-qt $OUTPUT_ROM"
