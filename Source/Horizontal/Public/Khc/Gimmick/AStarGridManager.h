#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Khc/Gimmick/PathLinkZone.h"
#include "AStarGridManager.generated.h"

USTRUCT(BlueprintType)
struct FPathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsObstacle = false;

    // Grid 내에서의 2D 인덱스 (X, Y)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntPoint GridIndex = FIntPoint(0, 0);

    // 월드 상의 실제 위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldLocation = FVector::ZeroVector;

    // A* 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GCost = 0.f; // 시작 노드로부터 현재 노드까지의 실제 이동 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HCost = 0.f; // 현재 노드로부터 목표 노드까지의 예상 이동 비용 (Heuristic)

    // F Cost (총 비용)를 계산하는 헬퍼 함수
    float GetFCost() const { return GCost + HCost; }

    // 이동 가중치 (기본값 1, 숫자가 클수록 이동 비용이 비싸짐)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementWeight = 1.0f;

    // 경로를 역추적하기 위해 부모 노드 저장
    FPathNode* ParentNode = nullptr;

    APathLinkZone* LinkedActor = nullptr;

    // 생성자
    FPathNode() {}
    FPathNode(FIntPoint InGridIndex, FVector InWorldLocation)
        : GridIndex(InGridIndex), WorldLocation(InWorldLocation) {}

    // TSet 등에서 사용하기 위한 비교 연산자 및 해시 함수
    bool operator==(const FPathNode& Other) const
    {
        return GridIndex == Other.GridIndex;
    }

    friend uint32 GetTypeHash(const FPathNode& Node)
    {
        return GetTypeHash(Node.GridIndex);
    }
};

UCLASS()
class HORIZONTAL_API AAStarGridManager : public AActor
{
	GENERATED_BODY()

public:
    AAStarGridManager();

    UPROPERTY(EditAnywhere, Category = "A* Grid")
    FVector2D GridWorldSize = FVector2D(10000.f, 10000.f);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A* Grid")
    class UBoxComponent* GridArea;

    UPROPERTY(EditAnywhere, Category = "A* Grid")
    float NodeRadius = 50.f;

    // 장애물로 인식할 오브젝트 타입
    UPROPERTY(EditAnywhere, Category = "A* Grid")
    TArray<TEnumAsByte<EObjectTypeQuery>> ObstacleObjectTypes;

    // 길찾기 메인 함수
    bool FindPath(FVector StartLocation, FVector TargetLocation, TArray<FVector>& OutPath);

    bool IsGridReady() const { return bGridReady; }

    UPROPERTY(EditAnywhere, Category = "A* Grid|Debug")
    bool bDebugDrawGrid = false;

    void BlurObstaclePenalties(int32 BlurSize);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
    TArray<FPathNode> Grid;
    float NodeDiameter;
    int32 GridSizeX, GridSizeY;

    void CreateGrid();
    FPathNode* GetNodeFromWorldLocation(FVector WorldLocation);
    void GetNeighborNodes(const FPathNode* Node, TArray<FPathNode*>& OutNeighbors);
    float CalculateDistance(const FPathNode* A, const FPathNode* B);
    void RetracePath(const FPathNode* StartNode, const FPathNode* EndNode, TArray<FVector>& OutPath);
    void SimplifyPath(const TArray<FVector>& InPath, TArray<FVector>& OutPath);

    bool bGridReady = false;

};
