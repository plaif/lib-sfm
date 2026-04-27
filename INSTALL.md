# libSFM 설치 및 검증 가이드

> **man_libSFM** `.deb` 패키지를 Linux 시스템에 설치하고,  
> 의존성, 설치 파일 구조, 런타임 링킹을 검증하는 단계별 가이드입니다.

---

## 목차

1. [패키지 설치](#1-패키지-설치)
2. [의존성 확인](#2-의존성-확인)
3. [설치된 파일 목록 확인](#3-설치된-파일-목록-확인)
4. [디스크 경로 확인](#4-디스크-경로-확인)
5. [런타임 링크 확인](#5-런타임-링크-확인)
6. [설치 체크리스트](#6-설치-체크리스트)

---

## 1. 패키지 설치

### ✅ 권장 방법 — `apt`를 통한 설치

`dpkg -i`와 달리, `apt`는 `Depends` 항목에 명시된 모든 패키지를 자동으로 해결하고 설치합니다.

```bash
sudo apt install ./man_libSFM_<version>_amd64.deb
```

### 대안 — `dpkg`로 설치 후 의존성 수동 해결

이미 `dpkg -i`로 설치한 경우, 아래 명령어로 누락된 의존성을 해결하세요.

```bash
sudo dpkg -i ./man_libSFM_<version>_amd64.deb
sudo apt -f install
```

> ⚠️ `-f install` 단계를 생략하면 라이브러리가 불완전한 상태로 남을 수 있습니다.

---

## 2. 의존성 확인

### 설치 전 — 패키지 파일 직접 검사

```bash
dpkg-deb -I ./man_libSFM_<version>_amd64.deb
```

출력 결과에서 `Depends:` 항목을 확인하세요.

### 설치 후 — 시스템에 등록된 메타데이터 확인

```bash
dpkg -s man_libSFM
```

### 현재 브랜치의 예상 의존성

`Depends:` 필드에 다음 패키지들이 포함되어 있어야 합니다.

| 패키지 | 역할 |
|--------|------|
| `libc6` | C 표준 런타임 라이브러리 |
| `libstdc++6` | C++ 표준 라이브러리 |
| `libgcc-s1` | GCC 런타임 지원 |
| `zlib1g` | 데이터 압축 라이브러리 |
| `libcurl4` | HTTP/HTTPS 전송 라이브러리 |
| `libssl3` | OpenSSL TLS/SSL 암호화 |
| `libfmt8` | 빠르고 안전한 C++ 포맷팅 라이브러리(버전 8) — printf의 현대적 대안 |

---

## 3. 설치된 파일 목록 확인

`dpkg -L`을 사용하여 패키지가 배포한 모든 파일 경로를 확인합니다.

```bash
dpkg -L man_libSFM
```

### 확인해야 할 주요 경로

| 경로 | 내용 |
|------|------|
| `/usr/lib/` | 공유 라이브러리 (`.so`) |
| `/usr/include/libsfm/` | 헤더 파일 (`.h`) |
| `/usr/lib/cmake/` | CMake 설정 파일 |
| `/usr/share/doc/man_libSFM/examples/` | 예제 소스 코드 |

---

## 4. 디스크 경로 확인

예제 소스 파일이 실제로 설치되었는지 확인합니다.

```bash
ls -al /usr/share/doc/man_libSFM/examples
```

파일만 나열하려면 (하위 디렉토리 제외):

```bash
find /usr/share/doc/man_libSFM/examples -maxdepth 1 -type f
```

---

## 5. 런타임 링크 확인

설치가 성공했더라도 공유 라이브러리 의존성이 런타임에 누락될 수 있습니다.  
실행 전에 반드시 `ldd`로 확인하세요.

```bash
# 일반 경로
ldd /usr/lib/libsfm.so

# 아키텍처별 경로 (amd64)
ldd /usr/lib/x86_64-linux-gnu/libsfm.so
```

### 정상 상태의 예상 출력

```
    linux-vdso.so.1 => (0x00007ffd...)
    libssl.so.3 => /lib/x86_64-linux-gnu/libssl.so.3
    libcurl.so.4 => /usr/lib/x86_64-linux-gnu/libcurl.so.4
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6
```

### `not found`가 표시되는 경우

`not found`로 표시된 항목은 런타임에 누락된 라이브러리이므로 별도로 설치해야 합니다.

```bash
# 누락된 라이브러리 검색
apt-cache search <라이브러리명>

# 해당 패키지 설치
sudo apt install <패키지명>
```

---

## 6. 설치 체크리스트

libSFM이 올바르게 설치되려면 아래 항목을 모두 통과해야 합니다.

- [ ] `sudo apt install ./man_libSFM_<version>_amd64.deb`로 설치 성공
- [ ] `dpkg -s man_libSFM`의 `Depends` 필드에 6개의 예상 패키지가 모두 표시됨
- [ ] `dpkg -L man_libSFM`에서 라이브러리, 헤더, cmake, 예제 경로가 모두 나열됨
- [ ] `ls /usr/share/doc/man_libSFM/examples`에서 예제 파일 존재 확인
- [ ] `ldd /usr/lib/.../libsfm.so` 출력에 `not found` 항목 없음

---

## 7. 환경 설정

제공된 활성화 키와 최신 모델 가중치 파일명으로 `.env` 파일을 업데이트하세요

경로 : `/etc/libsfm/.env`
```env
LIBSFM_API_KEY=your-activation-key-here
LIBSFM_WEIGHT_FILE=your-weight-filename-here
```

---

## 8. TensorRT 경로 설정

```bash
export PATH="/usr/local/bin:/usr/local/cuda/bin:${PATH}"
export LD_LIBRARY_PATH="/usr/local/cuda/lib64:/usr/local/tensorrt/lib:${LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH="/usr/local/tensorrt/targets/x86_64-linux-gnu/lib:${LD_LIBRARY_PATH}"
```
