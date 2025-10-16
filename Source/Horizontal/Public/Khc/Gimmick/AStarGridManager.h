#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Khc/Gimmick/PathLinkZone.h"
#include "AStarGridManager.generated.h"

USTRUCT(BlueprintType)
struct FPathNode
{
    GENERATED_BODY()

    // 해당 노드의 장애물 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsObstacle = false;

    // Grid 내에서의 2D 인덱스 좌표 (X, Y)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntPoint GridIndex = FIntPoint(0, 0);

    // 월드 상의 실제 3D 위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldLocation = FVector::ZeroVector;

    // --- A* 알고리즘 비용 ---
    // 시작 노드부터 현재 노드까지의 실제 이동 비용 (과거 비용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GCost = 0.f; 
    // 현재 노드부터 목표 노드까지의 예상 이동 비용 (미래 비용, Heuristic)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HCost = 0.f;

    // GCost와 HCost를 더한 총 예상 비용(F Cost)을 계산하는 헬퍼 함수
    float GetFCost() const { return GCost + HCost; }

    // 이동 가중치. 기본값은 1이며, 높을수록 AI가 회피함
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementWeight = 1.0f;

    // 경로 역추적을 위해 이전 노드(부모)를 저장하는 포인터
    FPathNode* ParentNode = nullptr;
    
    // 이 노드가 PathLinkZone(계단 등)과 연결된 경우, 해당 액터를 가리키는 포인터
    APathLinkZone* LinkedActor = nullptr;

    // 기본 생성자
    FPathNode() {}
    // 인덱스와 월드 위치를 받는 생성자
    FPathNode(FIntPoint InGridIndex, FVector InWorldLocation)
        : GridIndex(InGridIndex), WorldLocation(InWorldLocation) {}

    // TSet 등에서 노드를 비교하기 위한 연산자
    bool operator==(const FPathNode& Other) const
    {
        return GridIndex == Other.GridIndex;
    }

    // TSet에서 사용하기 위한 해시 함수
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

    // 2D 평면 그리드의 전체 크기 (현재는 GridArea BoxComponent로 대체됨)
    UPROPERTY(EditAnywhere, Category = "A* Grid")
    FVector2D GridWorldSize = FVector2D(10000.f, 10000.f);

    // 길찾기가 적용될 3D 볼륨을 시각적으로 설정하는 박스 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A* Grid")
    class UBoxComponent* GridArea;

    // 각 노드의 반경. NPC 크기와 유사하게 설정 권장
    UPROPERTY(EditAnywhere, Category = "A* Grid")
    float NodeRadius = 50.f;

    // 장애물로 인식할 콜리전 오브젝트 타입 목록 (예: WorldStatic)
    UPROPERTY(EditAnywhere, Category = "A* Grid")
    TArray<TEnumAsByte<EObjectTypeQuery>> ObstacleObjectTypes;

    // A* 알고리즘을 실행하여 최단 경로를 찾는 메인 함수
    bool FindPath(FVector StartLocation, FVector TargetLocation, TArray<FVector>& OutPath);

    // 그리드 생성이 완료되었는지 외부에서 확인하는 함수
    bool IsGridReady() const { return bGridReady; }

    // true 설정 시 에디터에서 그리드 노드들을 시각적으로 표시 (디버깅용)
    UPROPERTY(EditAnywhere, Category = "A* Grid|Debug")
    bool bDebugDrawGrid = false;

    // 장애물 주변 노드에 페널티(추가 가중치)를 부여하여 자연스러운 경로 생성
    void BlurObstaclePenalties(int32 BlurSize);

    const TArray<FPathNode>& GetGrid() const { return Grid; }

    //UFUNCTION()
    //void UpdateNodesInBounds(const FBox& BoundsToUpdate);

    UFUNCTION(BlueprintCallable, Category = "A* Grid")
    void RebuildGrid();


protected:
    // 게임 시작 시 호출, CreateGrid()를 실행하여 길찾기 지도 생성
    virtual void BeginPlay() override;

public: 
    // 매 프레임 호출, bDebugDrawGrid가 true일 때 디버그 시각화 담당
    virtual void Tick(float DeltaTime) override;

private:
    // 생성된 모든 FPathNode를 저장하는 배열. 길찾기 지도의 실체
    TArray<FPathNode> Grid;
    // 노드의 지름 (NodeRadius * 2)
    float NodeDiameter;
    // 그리드의 가로(X), 세로(Y) 노드 개수
    int32 GridSizeX, GridSizeY;

    // 길찾기 지도를 생성하는 핵심 함수
    void CreateGrid();
    // 월드 좌표(FVector)를 해당하는 그리드 노드(FPathNode)로 변환
    FPathNode* GetNodeFromWorldLocation(FVector WorldLocation);
    // 특정 노드를 기준으로 이웃 노드들을 찾는 함수 (8방향 + 링크)
    void GetNeighborNodes(const FPathNode* Node, TArray<FPathNode*>& OutNeighbors);
    // 두 노드 사이의 휴리스틱 거리(비용) 계산
    float CalculateDistance(const FPathNode* A, const FPathNode* B);
    // 길찾기 성공 시, ParentNode를 역추적하여 최종 경로 생성
    void RetracePath(const FPathNode* StartNode, const FPathNode* EndNode, TArray<FVector>& OutPath);
    // 생성된 경로에서 불필요한 점들을 제거하여 경로를 부드럽게 만듦
    void SimplifyPath(const TArray<FVector>& InPath, TArray<FVector>& OutPath);

    // 그리드 생성이 완료되었음을 나타내는 내부 플래그
    bool bGridReady = false;
};