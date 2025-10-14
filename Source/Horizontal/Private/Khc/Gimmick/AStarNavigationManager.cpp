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

bool AAStarNavigationManager::FindPath(FVector StartLocation, FVector TargetLocation, TArray<FVector>& OutPath)
{
	// 1. 시작점과 목표점이 각각 어느 층에 있는지 확인
    AAStarGridManager* StartManager = GetManagerForLocation(StartLocation);
    AAStarGridManager* TargetManager = GetManagerForLocation(TargetLocation);

    if (!StartManager || !TargetManager || !StartManager->IsGridReady() || !TargetManager->IsGridReady())
    {
        return false; // 지도가 준비되지 않았으면 실패
    }

    // 2. 만약 같은 층이라면, 해당 층의 GridManager에게 길찾기를 위임
    if (StartManager == TargetManager)
    {
        return StartManager->FindPath(StartLocation, TargetLocation, OutPath);
    }

    // 3. 다른 층이라면, 층간 이동 경로 탐색 (가장 중요한 부분)
    else
    {
        TArray<FVector> BestFullPath;
        float MinTotalCost = BIG_NUMBER;

        // 시작 층에 있는 모든 '환승역'(PathLink)을 후보지로 탐색
        for (const FPathNode& Node : StartManager->GetGrid())
        {
            if (Node.LinkedActor && Node.LinkedActor->TargetPoint)
            {
                TArray<FVector> PathToLink;
                // A. 시작점에서 환승역까지의 경로 계산
                bool bFoundPathToLink = StartManager->FindPath(StartLocation, Node.WorldLocation, PathToLink);

                if (bFoundPathToLink)
                {
                    // 환승역 반대편(다른 층) 위치
                    FVector TargetLinkLocation = Node.LinkedActor->TargetPoint->GetActorLocation();
                    
                    TArray<FVector> PathFromLink;
                    // B. 환승역 반대편에서 최종 목적지까지의 경로 계산
                    bool bFoundPathFromLink = TargetManager->FindPath(TargetLinkLocation, TargetLocation, PathFromLink);

                    if (bFoundPathFromLink)
                    {
                        // A 경로와 B 경로의 비용을 합산 (경로 길이로 근사치 계산)
                        float PathACost = (PathToLink.Num() > 0) ? FVector::Dist(StartLocation, PathToLink.Last()) : 0;
                        float PathBCost = (PathFromLink.Num() > 0) ? FVector::Dist(TargetLinkLocation, PathFromLink.Last()) : 0;
                        float TotalCost = PathACost + PathBCost;

                        // 이 환승역을 거치는 경로가 지금까지 찾은 경로보다 더 짧다면, 이걸 최고의 경로로 저장
                        if (TotalCost < MinTotalCost)
                        {
                            MinTotalCost = TotalCost;
                            BestFullPath = PathToLink;
                            BestFullPath.Append(PathFromLink); // 두 경로를 합침
                        }
                    }
                }
            }
        }

        if (BestFullPath.Num() > 0)
        {
            OutPath = BestFullPath;
            return true;
        }
    }

    return false; // 층간 이동 경로를 찾지 못함
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