// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/Gimmick/AStarNavigationManager.h"

#include "Khc/Gimmick/AStarGridManager.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AAStarNavigationManager::AAStarNavigationManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

AAStarGridManager* AAStarNavigationManager::GetManagerForLocation(const FVector& Location)
{
	AAStarGridManager* BestManager = nullptr;
	float MinZDistance = BIG_NUMBER;

	for (AAStarGridManager* Manager : AllGridManagers)
	{
		if (Manager)
		{
			float ZDistance = FMath::Abs(Manager->GetActorLocation().Z - Location.Z);
			if (ZDistance < MinZDistance)
			{
				MinZDistance = ZDistance;
				BestManager = Manager;
			}
		}
	}
	return BestManager;
}

float AAStarNavigationManager::FindPath(FVector StartLocation, FVector TargetLocation, TArray<FVector>& OutPath)
{
	TSet<AAStarGridManager*> VisitedManagers; // 방문 기록 초기화
	return FindPathRecursive(StartLocation, TargetLocation, OutPath, VisitedManagers);
}

// Called when the game starts or when spawned
void AAStarNavigationManager::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAStarGridManager::StaticClass(), FoundManagers);
	for (AActor* Manager : FoundManagers)
	{
		AllGridManagers.Add(Cast<AAStarGridManager>(Manager));
	}

	UE_LOG(LogTemp, Log, TEXT("Found %d AStarGridManagers."), AllGridManagers.Num());
	
}

float AAStarNavigationManager::FindPathRecursive(FVector StartLocation, FVector TargetLocation,
	TArray<FVector>& OutPath, TSet<AAStarGridManager*>& VisitedManagers)
{
	AAStarGridManager* StartManager = GetManagerForLocation(StartLocation);
    AAStarGridManager* TargetManager = GetManagerForLocation(TargetLocation);

    if (!StartManager || !TargetManager || !StartManager->IsGridReady() || !TargetManager->IsGridReady())
    {
        return -1.0f;
    }

    // [재귀 탈출 조건 1] 이 층(StartManager)을 이미 방문했다면, 무한 루프이므로 탐색 중단
    if (VisitedManagers.Contains(StartManager))
    {
        return -1.0f;
    }
    VisitedManagers.Add(StartManager); // 현재 층을 방문 목록에 추가

    // [재귀 탈출 조건 2] 만약 같은 층이라면, '지역 가이드'에게 길찾기를 위임하고 비용 반환
    if (StartManager == TargetManager)
    {
        // 방문 기록을 남길 필요가 없으므로 다시 제거 (선택적)
        VisitedManagers.Remove(StartManager); 
        return StartManager->FindPath(StartLocation, TargetLocation, OutPath);
    }

    // [재귀 탐색] 다른 층이라면, 현재 층의 모든 '환승역'을 탐색
    TArray<FVector> BestFullPath;
    float MinTotalCost = BIG_NUMBER;

    for (const FPathNode& Node : StartManager->GetGrid())
    {
        if (Node.LinkedActor && Node.LinkedActor->TargetPoint)
        {
            TArray<FVector> PathToLink; // A경로 (시작점 -> 환승역 1)
            
            // A. 시작점에서 이 환승역(계단 입구)까지의 비용과 경로
            float CostToLink = StartManager->FindPath(StartLocation, Node.WorldLocation, PathToLink);

            if (CostToLink >= 0) // A경로 찾기에 성공했다면
            {
                FVector LinkTargetLocation = Node.LinkedActor->TargetPoint->GetActorLocation();
                
                TArray<FVector> PathFromLink; // B경로 (환승역 2 -> 최종 목적지)
                
                // B. 이 환승역의 반대편(다음 층)에서 최종 목적지까지의 비용과 경로를 '재귀 호출'
                float CostFromLink = FindPathRecursive(LinkTargetLocation, TargetLocation, PathFromLink, VisitedManagers);

                if (CostFromLink >= 0) // B경로 찾기에 성공했다면
                {
                    float TotalCost = CostToLink + CostFromLink;
                    if (TotalCost < MinTotalCost)
                    {
							MinTotalCost = TotalCost;
                        BestFullPath = PathToLink;
                        BestFullPath.Append(PathFromLink);
                    }
                }
            }
        }
    }

    // 이 층의 모든 환승역 탐색이 끝났으므로, 방문 기록에서 제거 (다른 경로가 이 층을 다시 방문할 수 있도록)
    VisitedManagers.Remove(StartManager);

    if (BestFullPath.Num() > 0)
    {
        OutPath = BestFullPath;
        return MinTotalCost;
    }

    return -1.0f;
}
