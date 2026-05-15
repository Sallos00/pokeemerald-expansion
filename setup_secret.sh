#!/usr/bin/env bash
# ============================================================
#  setup_secret.sh
#  baserom.gba → GitHub Secret (BASEROM_BASE64) 등록 도우미
#  
#  사용법:
#    chmod +x setup_secret.sh
#    ./setup_secret.sh                          # 대화형
#    ./setup_secret.sh YOUR_REPO baserom.gba    # 직접 지정
# ============================================================

set -e

REPO="${1:-}"
BASEROM="${2:-baserom.gba}"

echo "============================================"
echo "  PokéRogue GBA - GitHub Secret 설정 도우미"
echo "============================================"

# ── baserom 확인 ──────────────────────────────────────────────
if [ ! -f "$BASEROM" ]; then
    echo "오류: $BASEROM 파일이 없습니다."
    echo "포켓몬 에메랄드 영문판 ROM을 baserom.gba 로 이름 변경 후 재실행하세요."
    exit 1
fi

# SHA1 검증
ACTUAL=$(sha1sum "$BASEROM" | awk '{print $1}')
EXPECTED="f3ae088181bf583e55daf962a92bb46f4f1d07b7"
if [ "$ACTUAL" = "$EXPECTED" ]; then
    echo "✓ SHA1 확인: 올바른 에메랄드 영문판 ROM"
else
    echo "⚠ SHA1 불일치 (expected=$EXPECTED)"
    echo "  actual=$ACTUAL"
    echo "  영문판 에메랄드가 아닐 수 있습니다. 계속하시겠습니까? (y/N)"
    read -r answer
    [ "$answer" = "y" ] || exit 1
fi

# ── base64 인코딩 ─────────────────────────────────────────────
echo ""
echo "base64 인코딩 중... (ROM 크기: $(wc -c < "$BASEROM") bytes)"
B64=$(base64 -w0 "$BASEROM")
echo "✓ 인코딩 완료 (${#B64} 문자)"

# ── GitHub CLI로 자동 등록 (gh 설치된 경우) ──────────────────
if command -v gh &>/dev/null; then
    echo ""
    if [ -z "$REPO" ]; then
        echo "GitHub 저장소 (예: username/pokerogue-gba): "
        read -r REPO
    fi
    echo "Secret 등록 중: $REPO"
    echo "$B64" | gh secret set BASEROM_BASE64 --repo "$REPO"
    echo "✓ BASEROM_BASE64 secret 등록 완료"
    echo ""
    echo "이제 GitHub Actions가 자동으로 빌드합니다."
    echo "https://github.com/$REPO/actions"
else
    # ── 수동 등록 안내 ────────────────────────────────────────
    echo ""
    echo "GitHub CLI(gh)가 없습니다. 수동으로 등록하세요:"
    echo ""
    echo "1. 아래 값을 복사하세요 (매우 긴 문자열):"
    echo "   ──────────────────────────────────────"
    echo "$B64" | head -c 200
    echo "...(이하 생략)"
    echo "   ──────────────────────────────────────"
    echo ""
    echo "   전체 값을 파일로 저장: base64 -w0 $BASEROM > /tmp/baserom_b64.txt"
    echo ""
    echo "2. GitHub 저장소 → Settings → Secrets and variables → Actions"
    echo "3. 'New repository secret' 클릭"
    echo "4. Name: BASEROM_BASE64"
    echo "5. Value: 위에서 복사한 전체 문자열 붙여넣기"
    echo "6. 'Add secret' 클릭"
    echo ""
    echo "파일로 저장하려면:"
    echo "  base64 -w0 $BASEROM > /tmp/baserom_b64.txt"
fi

# ── 로컬 테스트 방법 안내 ─────────────────────────────────────
echo ""
echo "============================================"
echo "  로컬 빌드 테스트"
echo "============================================"
echo ""
echo "DevKitPro 설치 후:"
echo "  export DEVKITPRO=/opt/devkitpro"
echo "  export DEVKITARM=\$DEVKITPRO/devkitARM"
echo "  export PATH=\$DEVKITARM/bin:\$PATH"
echo "  make -j\$(nproc)"
echo "  xdelta3 -e -s baserom.gba pokeemerald.gba pokerogue-gba.xdelta"
echo ""
echo "패치 적용 테스트:"
echo "  xdelta3 -d -s baserom.gba pokerogue-gba.xdelta test_output.gba"
echo "  mgba-qt test_output.gba"
