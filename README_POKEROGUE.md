# PokéRogue GBA

> pokeemerald-expansion 기반 로그라이크 포켓몬 게임  
> GBA 실기 및 에뮬레이터 실행 가능 · xdelta3 패치 배포

---

## 사용자 (패치 적용)

### 필요한 것
- 포켓몬 에메랄드 영문판 ROM
  - `SHA1: f3ae088181bf583e55daf962a92bb46f4f1d07b7`
- xdelta3 패치 도구

### xdelta3 설치

| OS | 방법 |
|----|------|
| Windows | [Delta Patcher GUI](https://github.com/marco-calautti/DeltaPatcher/releases) 추천 |
| macOS | `brew install xdelta` |
| Linux | `sudo apt install xdelta3` |

### 패치 적용

**터미널**
```bash
xdelta3 -d -s 에메랄드원본.gba pokerogue-gba.xdelta 결과.gba
```

**Delta Patcher GUI**
1. Original File → 에메랄드 원본 ROM 선택
2. XDelta Patch  → `pokerogue-gba.xdelta` 선택
3. Apply Patch 클릭

### 실행
- mGBA (추천)
- VisualBoyAdvance-M
- 실물 GBA + EZ-FLASH / Everdrive 플래시카트

---

## 개발자 (빌드 환경 구성)

### 구조
```
pokeemerald-expansion/       ← 이 저장소
├── src/rogue/               ← PokéRogue GBA 소스 (14개 파일)
├── include/rogue/           ← 헤더 (11개 파일)
├── src/data/rogue/          ← 바이옴/보상/패시브 데이터
├── tools/pokerogue_import/  ← Python 데이터 변환 도구
├── .github/workflows/
│   └── pokerogue-build.yml  ← 자동 빌드 + xdelta 생성
└── setup_secret.sh          ← GitHub Secret 등록 도우미
```

---

## GitHub Actions 자동 빌드 설정

### 1단계 - 저장소 fork / clone

```bash
# pokeemerald-expansion을 fork한 후
git clone https://github.com/YOUR_NAME/pokeemerald-expansion.git
cd pokeemerald-expansion

# 이 프로젝트 파일 적용 (이미 적용된 경우 생략)
# rogue 파일들이 이미 포함되어 있음
```

### 2단계 - baserom Secret 등록

GitHub Actions가 빌드할 때 에메랄드 원본 ROM이 필요합니다.  
ROM 자체는 저장소에 올리면 안 되므로 **Encrypted Secret**으로 등록합니다.

**자동 등록 (gh CLI 사용)**
```bash
# GitHub CLI 설치: https://cli.github.com
gh auth login
./setup_secret.sh YOUR_NAME/pokeemerald-expansion baserom.gba
```

**수동 등록**
```bash
# 1. base64 인코딩
base64 -w0 baserom.gba > /tmp/baserom_b64.txt

# 2. GitHub 저장소 → Settings
#    → Secrets and variables → Actions
#    → New repository secret
#      Name:  BASEROM_BASE64
#      Value: /tmp/baserom_b64.txt 내용 전체 붙여넣기
```

### 3단계 - 빌드 트리거

```bash
# 일반 push → Actions에서 빌드 아티팩트 다운로드 가능
git push origin main

# 릴리즈 배포 → GitHub Release에 xdelta 자동 업로드
git tag v0.1.0
git push origin v0.1.0
```

### 4단계 - 결과 확인

```
GitHub 저장소 → Actions 탭 → 최신 워크플로 → Artifacts
  └── pokerogue-gba-{sha}.zip
        ├── pokerogue-gba.xdelta      ← 패치 파일
        └── pokerogue-gba.xdelta.sha1 ← 체크섬
```

태그 push 시 → **Releases 탭**에 자동 등록

---

## 로컬 빌드 (선택)

### DevKitPro 설치

```bash
# Linux
wget https://apt.devkitpro.org/install-devkitpro-pacman -O install-dkp.sh
chmod +x install-dkp.sh
echo "y" | sudo ./install-dkp.sh
sudo dkp-pacman -Sy gba-dev

# macOS (Homebrew)
brew install devkitpro/devkitpro/devkitARM

# Windows
# https://github.com/devkitPro/installer/releases 에서 설치
```

### 환경변수

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export PATH=$DEVKITARM/bin:$PATH
```

### 빌드 및 패치 생성

```bash
# 빌드
cp /path/to/pokemon_emerald.gba baserom.gba
make -j$(nproc)

# xdelta3 패치 생성
xdelta3 -e -s baserom.gba pokeemerald.gba pokerogue-gba.xdelta

# 패치 검증
xdelta3 -d -s baserom.gba pokerogue-gba.xdelta verify.gba
sha1sum pokeemerald.gba verify.gba  # 동일해야 함

# 실행
mgba-qt pokeemerald.gba
```

### Python 데이터 임포트 (선택)

```bash
pip install pillow

# PokéRogue 저장소에서 바이옴 데이터 가져오기
python3 tools/pokerogue_import/import_biomes.py \
    --pokerogue-src /path/to/pokerogue/src \
    --output src/data/rogue/biome_tables.h

# 한글 폰트 생성 (NotoSansKR-Regular.ttf 필요)
python3 tools/pokerogue_import/extract_korean_chars.py \
    --locales /path/to/pokerogue-locales/ko \
    --font-ttf tools/fonts/NotoSansKR-Regular.ttf \
    --output-dir src/data/rogue

# 전체 파이프라인
python3 tools/pokerogue_import/run_all.py \
    --pokerogue /path/to/pokerogue \
    --locales /path/to/pokerogue-locales/ko \
    --output-root .
```

---

## 게임 흐름

```
타이틀
  └─ A: 스타터 선택 (24종+, 3×3 그리드, LR 페이지)
       └─ A: 런 시작
            └─ 노드 맵 (6레이어, 분기 경로)
                 ├─ 전투 노드  → 배틀 → 승리 → 보상 선택 (3종)
                 ├─ 엘리트 노드 → 강화된 배틀
                 ├─ 상점 노드  → 아이템/패시브 구매
                 ├─ 회복 노드  → HP 회복
                 ├─ 이벤트 노드 → 랜덤 이벤트
                 └─ 보물 노드  → 즉시 보상
                      └─ 10웨이브마다 보스
                           └─ 50웨이브 클리어 → 런 완료
  └─ B: 메타 화면
        ├─ 영구 업그레이드 (포인트로 구매)
        └─ 기록 확인
```

---

## 시스템 개요

| 시스템 | 내용 |
|--------|------|
| **바이옴** | 15종 (초원/동굴/화산/우주 등), 가중치 기반 분기 |
| **스폰** | 바이옴별 스폰 테이블, 웨이브 레벨 스케일링 |
| **패시브** | 13종, 중첩 가능, 배틀 시작/KO/교체 시 발동 |
| **보상** | 아이템/패시브/포켓몬/돈 중 3종 선택 |
| **저장** | GBA SRAM (메타 데이터 영구 보존) |
| **한글** | 커스텀 8×8 폰트, Python으로 자동 생성 |

---

## 라이선스

- **pokeemerald-expansion**: MIT License
- **이 프로젝트 (rogue 레이어)**: MIT License
- **PokéRogue 데이터 참고**: GPL-3.0 (pagefaultgames/pokerogue)
- **포켓몬 관련 IP**: Nintendo / Game Freak 소유
