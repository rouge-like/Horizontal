# Repository Guidelines

## 프로젝트 개요
화재대응훈련멀티플레이시뮬레이션(코드베이스 식별명: Horizontal)은 Unreal Engine 5 기반의 멀티 플레이 훈련 시뮬레이터로, 공통 시스템을 공유하면서도 `Variant_Horror`와 `Variant_Shooter` 두 가지 시나리오를 통해 협동 구조, 긴급 대응, 장비 운용을 빠르게 검증하는 것을 목표로 합니다. 공포 변형은 제한된 시야와 감지 메커니즘으로 위기 인지와 팀 커뮤니케이션을 강조하고, 슈팅 변형은 빠른 전투와 무기 밸런스 실험을 통해 장비 운용 훈련을 보완합니다. 각 변형은 공통 캐릭터, 입력, 카메라 매니저를 확장하여 기능을 구현하며, 반복적인 시뮬레이션 개선을 위해 최소한의 의존성으로 모듈화되어 있습니다.

## 목적
실제 화재 대응 절차를 안전하게 체험할 수 있는 가상 훈련장을 제공하고, 협력 역할 분담을 통한 대응 훈련을 강화하며, 교육 효과와 몰입감 있는 게임성을 동시에 확보해 대원들이 위험 없이 상황 인지·의사 결정·팀 협업 역량을 높일 수 있도록 돕습니다.

## 프로젝트 구조 및 모듈 구성
프로젝트 루트에는 `Horizontal.uproject`와 Visual Studio 솔루션 `Horizontal.sln`이 위치합니다. 실질적인 게임 코드는 `Source/Horizontal`에 있으며, 루트에는 공통 클래스와 매니저가, 하위 폴더 `Variant_Horror`, `Variant_Shooter`에는 UI·AI·Weapons 등 변형별 구현이 정리되어 있습니다. `Content`에는 블루프린트, 레벨, 데이터 테이블 등 에셋이, `Config`에는 환경 설정이 존재합니다. `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`는 빌드 산출물이므로 직접 수정하지 말고, 버전에 포함할 파일인지 확인 후 커밋하세요.

## 빌드·테스트·개발 명령어
- `Engine\Build\BatchFiles\Build.bat HorizontalEditor Win64 Development` — 개발용 에디터 타깃을 빌드합니다.
- `GenerateProjectFiles.bat Horizontal.uproject` — 모듈 변경 후 프로젝트 파일을 재생성합니다.
- `UnrealEditor.exe Horizontal.uproject -game` — 현재 기본 게임 모드로 플레이 모드를 실행합니다.
- `UnrealEditor.exe Horizontal.uproject -ExecCmds="Automation RunTests Horizontal" -ReportExportPath="Saved/Automation"` — 자동화 테스트를 수행하고 결과 리포트를 출력합니다.

## 코딩 스타일 및 네이밍
Epic 스타일 가이드에 따라 4칸 공백 들여쓰기, 다음 줄에 중괄호를 배치합니다. 클래스는 Unreal 접두사(`A`, `U`, `F`, `E`)를 포함한 PascalCase(`AHorizontalGameMode`)를 사용하고, 함수는 UpperCamelCase, 지역 변수는 lowerCamelCase, 불린은 `b` 접두사를 붙입니다. 헤더와 소스 파일은 동일 경로에 짝지어 두고, 필수 헤더만 포함해 컴파일 시간을 최소화하세요. Unreal 타입(`FVector`, `TArray`)을 우선 사용하고, 모듈 간 상호작용은 기존 매니저(`HorizontalCameraManager` 등)를 통해 이루어지도록 유지합니다.

## 테스트 가이드
자동화 테스트는 대응하는 런타임 클래스와 동일한 이름 패턴(예: `ShooterCharacter_Automation`)으로 `Source/Horizontal` 하위에 배치합니다. 신규 게임플레이 로직, 입력 매핑, 멀티플레이 의존 기능은 결정론적 시나리오로 커버리지를 확보합니다. PR 전에는 위 자동화 명령을 헤드리스로 실행하고, 생성된 `Saved/Automation/Logs`를 검토해 실패 케이스를 해결한 후 공유합니다. 실시간 협업 피드백이 필요한 기능은 수동 검증 절차를 README나 PR 본문에 간단히 남겨주세요.

## 커밋 및 PR 가이드라인
커밋 메시지는 명령형 현재형과 모듈 접두사를 결합한 `module: 요약` 형식(예: `shooter: adjust recoil curve`)으로 작성하고, 제목은 72자 이내로 유지합니다. 필요한 경우 본문에 구현 배경과 사이드이펙트를 설명하세요. PR에는 변경된 게임플레이 영향, 수정한 시스템(코어·호러·슈터·UI)을 리스트업하고, UI·아트 변경 시 스크린샷이나 짧은 클립을 첨부합니다. 관련 Jira 또는 GitHub 이슈를 링크하며, 머지 전에는 `main`에 리베이스해 충돌을 제거합니다.

## 에셋 및 설정 팁
재사용 가능한 데이터 테이블, 입력 매핑, 머티리얼 인스턴스는 변형별 `Content/.../Data` 폴더에 정리해 추적성을 높입니다. 입력 관련 코드를 수정할 경우 `Config/DefaultInput.ini`를 함께 갱신하여 동기화를 유지하고, 개인 설정은 `Saved/Config/WindowsEditor`에 저장해 공동 설정 파일을 건드리지 마세요. 대용량 에셋을 추가할 때는 Git LFS 적용 여부를 확인하고, 저장소 용량 영향이 있는 경우 팀과 사전 공유합니다.
