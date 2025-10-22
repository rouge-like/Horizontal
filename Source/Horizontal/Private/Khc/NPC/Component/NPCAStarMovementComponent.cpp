#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/Gimmick/AStarNavigationManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"


UNPCAStarMovementComponent::UNPCAStarMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNPCAStarMovementComponent::BeginPlay()
{
    NavigationManager = Cast<AAStarNavigationManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarNavigationManager::StaticClass()));
	//GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));
}

void UNPCAStarMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (bIsMoving)
    {
        // 1. 실제 목표 지점(SafetyZone)을 빨간색 구로 표시S
        DrawDebugSphere(GetWorld(), Destination, 100.f, 12, FColor::Red, false, -1.f, 0, 10.f);

        if (CurrentPath.Num() > 0)
        {
            // 2. A*가 계산한 경로의 최종 도착 지점을 파란색 구로 표시
            DrawDebugSphere(GetWorld(), CurrentPath.Last(), 120.f, 12, FColor::Blue, false, -1.f, 0, 10.f);

            // 3. 현재 따라가야 할 경로를 초록색 선으로 표시
            for (int32 i = CurrentPathIndex; i < CurrentPath.Num() - 1; ++i)
            {
                DrawDebugLine(GetWorld(), CurrentPath[i], CurrentPath[i + 1], FColor::Green, false, -1.f, 0, 5.f);
            }
        }
    }

    // --- 경로 따라가기 로직 ---
    if (bIsMoving && CurrentPath.IsValidIndex(CurrentPathIndex))
    {
        APawn* OwnerPawn = Cast<APawn>(GetOwner());
        if (!OwnerPawn) return;

        FVector CurrentLocation = OwnerPawn->GetActorLocation();
        FVector Waypoint = CurrentPath[CurrentPathIndex];

        // 1. 도착 판정: 다음 경유지에 충분히 가까워졌는지 확인
        if (FVector::Dist2D(CurrentLocation, Waypoint) < 60.f)
        {
            CurrentPathIndex++;
            // 경로의 마지막에 도달했는지 다시 한번 체크
            if (!CurrentPath.IsValidIndex(CurrentPathIndex))
            {
                // 최종 목적지에 도착했는지 최종 확인
                if (FVector::Dist2D(CurrentLocation, Destination) < 150.f)
                {
                    bIsMoving = false;
                    OnMovementFinished.Broadcast();
                }
                return; // 이번 틱은 여기서 종료
            }
            // 다음 웨이포인트로 목표 갱신
            Waypoint = CurrentPath[CurrentPathIndex];
        }

        // 2. 회전: 다음 경유지를 향해 부드럽게 몸을 돌림
        // FVector DirectionToWaypoint = (Waypoint - CurrentLocation).GetSafeNormal();
        // FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, Waypoint);
        //
        // // Z축 회전은 필요 없으므로 Yaw 값만 사용
        // FRotator CurrentRotation = OwnerPawn->GetActorRotation();
        // FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f); // 5.0f는 회전 속도 (조절 가능)
        //
        // OwnerPawn->SetActorRotation(FRotator(0.f, SmoothedRotation.Yaw, 0.f));
        //
        // // 3. 전진: 현재 캐릭터가 바라보는 '앞 방향'으로 이동 입력을 줌
        // OwnerPawn->AddMovementInput(OwnerPawn->GetActorForwardVector());

        FVector Direction = (Waypoint - CurrentLocation).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction);
    }
    else if(bIsMoving) // 경로가 끝났지만 아직 이동 중 플래그가 켜져 있다면
    {
        // 도착 판정 로직을 한 번 더 수행하여 이동을 확실히 끝냄
        if (FVector::Dist2D(GetOwner()->GetActorLocation(), Destination) < 150.f)
        {
            bIsMoving = false;
            OnMovementFinished.Broadcast();
        }
    }
}

void UNPCAStarMovementComponent::StartMovingTo(const FVector& NewDestination)
{
    if (!NavigationManager)
    {
        UE_LOG(LogTemp, Error, TEXT("AStarMovementComponent: AStarNavigationManager not found!"));
        return;
    }

    Destination = NewDestination;
    CurrentPathIndex = 0;

    // 이제 GridManager가 아닌, NavigationManager의 FindPath를 직접 호출합니다.
    float bPathFound = NavigationManager->FindPath(GetOwner()->GetActorLocation(), Destination, CurrentPath);

    if (bPathFound > 0)
    {
        bIsMoving = true;
        UE_LOG(LogTemp, Log, TEXT("New path found with %d waypoints."), CurrentPath.Num());
    }
    else
    {
        bIsMoving = false;
        UE_LOG(LogTemp, Warning, TEXT("Path to destination could not be found!"));
    }
}
