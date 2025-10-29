#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireManager.generated.h"

class UPrimitiveComponent;
class AVFXActor;
class AVFXManager;
class APlayerBaseState;
UENUM(BlueprintType)
enum class EFireCellState : uint8
{
    Dormant,
    Igniting,
    Burning,
    Extinguished
};

USTRUCT(BlueprintType)
struct HORIZONTAL_API FFireCell
{
    GENERATED_BODY()

    FFireCell()
        : State(EFireCellState::Dormant)
        , GridIndex(FIntVector::ZeroValue)
        , WorldCenter(FVector::ZeroVector)
        , CellExtent(FVector::ZeroVector)
        , Fuel(0.0f)
        , MaxFuel(0.0f) 
        , Heat(0.0f)
        , IgnitionTimeRemaining(0.0f)
        , Depth(0)
        , ParentIndex(INDEX_NONE)
        , bIsLeaf(true)
    {
        for (int32 Index = 0; Index < NumChildren; ++Index)
        {
            ChildIndices[Index] = INDEX_NONE;
        }
    }

    static constexpr int32 NumChildren = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    EFireCellState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    FIntVector GridIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    FVector WorldCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    FVector CellExtent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    float Fuel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    float MaxFuel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    float Heat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    float IgnitionTimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    int32 Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    int32 ParentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FireCell")
    bool bIsLeaf;
    
    UPROPERTY(Transient)
    TWeakObjectPtr<UPrimitiveComponent> AttachedComponent;

    UPROPERTY(Transient)
    TWeakObjectPtr<AVFXActor> ActiveEffect;

    UPROPERTY(Transient)
    int32 ChildIndices[NumChildren];

    UPROPERTY(Transient)
    FVector SurfacePoint = FVector::ZeroVector;

    UPROPERTY(Transient)
    FVector SurfaceNormal = FVector::UpVector;
};

UCLASS()
class HORIZONTAL_API AFireManager : public AActor
{
    GENERATED_BODY()

public:
    AFireManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    void CheckPlayerInFire(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category="Fire|Cells")
    int32 FindCellIndexAtLocation(const FVector& WorldLocation) const;

    void ApplySuppressionAtLocation(const FVector& WorldLocation, float SuppressionAmount);
public:
    void ApplySuppressionInSphere(APlayerBaseState* PlayerState, const FVector& Center, float Radius, float SuppressionAmount);

    // 불이 붙었을 때 사용할 파라미터들 (에디터에서 조정 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|OnFireParams", meta=(ClampMin="0.0", ToolTip="불의 중심 반경 (cm)"))
    float OnFireBurnRadius = 25.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|OnFireParams", meta=(ClampMin="0.001", ToolTip="그라데이션 영역 폭 - 클수록 부드러운 경계 (cm)"))
    float OnFireEdgeWidth = 60.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|OnFireParams", meta=(ClampMin="0.0", ClampMax="100.0"))
    float OnFireBurnIntensity_G = 25.f;
    
    // 그라데이션 부드러움 (머티리얼에서 smoothstep의 power 값으로 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|OnFireParams", meta=(ClampMin="0.5", ClampMax="5.0", ToolTip="1=선형, >1=부드러운 곡선, <1=날카로운 경계"))
    float EdgeFalloffPower = 2.0f;

    // 불이 꺼질 때 애니메이션 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|ExtinguishAnimation", meta=(ClampMin="0.1", ToolTip="소화 애니메이션 지속 시간 (초)"))
    float ExtinguishAnimationDuration = 2.0f;  // 2초 동안 점진적으로 재로 변화
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|ExtinguishAnimation", meta=(ToolTip="재 상태의 어두움 정도 (0=원본색, 1=완전히 검음)"))
    float AshDarkenAmount = 0.7f;

protected:
    UPROPERTY(EditAnywhere, Category="Fire|Cells", meta=(ClampMin="1.0"))
    float LeafCellSize = 50.0f;

    UPROPERTY(EditAnywhere, Category="Fire|Cells", meta=(ClampMin="1.0"))
    float FireVFXSize = 1.0f;
    
    UPROPERTY(EditAnywhere, Category="Fire|Cells", meta=(ClampMin="0"))
    int32 MaxOctreeDepth = 4;

    UPROPERTY(EditAnywhere, Category="Fire|Cells")
    float DefaultFuelAmount = 100.0f;

    UPROPERTY(EditAnywhere, Category="Fire|Spread")
    float IgnitionDelay = 2.0f;

    UPROPERTY(EditAnywhere, Category="Fire|Spread")
    float DefaultHeatValue = 2.0f;
    
    UPROPERTY(EditAnywhere, Category="Fire|Spread")
    float SpreadSearchTolerance = 100.0f;

    UPROPERTY(EditAnywhere, Category="Fire|Spread")
    float AdjacentContactTolerance = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category="Fire|VFX")
    TArray<FName> FireVFXs;

    UPROPERTY(VisibleAnywhere, Category="Fire|Cells")
    TArray<FFireCell> Cells;

    UPROPERTY(VisibleAnywhere, Category="Fire|Cells")
    TArray<int32> RootCellIndices;

    TArray<int32> FreeCellIndices;

private:
    void GenerateCellsFromActor(AActor& SourceActor);
    int32 CreateCell(const FVector& Center, const FVector& Extent, int32 Depth, int32 ParentIndex);
    void SubdivideCell(int32 CellIndex);
    void CollapseCell(int32 CellIndex);

    TSet<int32> ActiveCells;     // 현재 불타거나 점화 중인 셀 인덱스
    TSet<int32> NextActiveCells; // 다음 프레임에 활성화될 셀
    
    struct FFireCellTickResult
    {
        bool bIsLeaf = true;
        bool bAnyBurning = false;
    };

    void InitializeFireAreas();
public:
    void IgniteSphere(const FVector& Center, float Radius);
private:
    void IgniteSphereRecursive(int32 CellIndex, const FVector& Center, float Radius);

    void SpreadFireFromCell(int32 CellIndex);
    void SpreadFireRecursive(int32 SourceIndex, int32 TargetIndex, const FBox& SourceBounds);

    //FFireCellTickResult ProcessCellRecursive(int32 CellIndex, float DeltaSeconds);
    FFireCellTickResult ProcessCollapseRecursive(int32 CellIndex, float DeltaSeconds);

    bool AreBoxesPotentiallyAdjacent(const FBox& SourceBounds, const FBox& TargetBounds) const;
    bool AreBoxesAdjacent(const FBox& SourceBounds, const FBox& TargetBounds) const;

    bool EnsureCombustibleComponent(int32 CellIndex);
    
    void ActivateFireVFX(int32 CellIndex);

    void DeactivateFireVFX(int32 CellIndex);

    int32 FindCellIndexRecursive(int32 CellIndex, const FVector& WorldLocation) const;

    void ApplySuppressionInSphereRecursive(APlayerBaseState* PlayerState, int32 CellIndex, const FVector& Center, double RadiusSquared, float SuppressionAmount);
    bool ApplySuppressionToCell(int32 CellIndex, float SuppressionAmount);
  
    void FreezeBurnMaterialAtCell(int32 CellIndex);
    void ClearBurnMIForCell(int32 CellIndex);
    
    // 점진적 소화 애니메이션 함수들
    UFUNCTION(BlueprintCallable, Category="Fire")
    void StartExtinguishAnimation(int32 CellIndex);
    
    UFUNCTION(BlueprintCallable, Category="Fire")
    void UpdateExtinguishAnimation(int32 CellIndex);
    
    UFUNCTION(BlueprintCallable, Category="Fire")
    void CompleteExtinguishAnimation(int32 CellIndex);


    
    UPROPERTY(Transient)
    TWeakObjectPtr<AVFXManager> VFXManager;

    UPROPERTY(Transient)
    TMap<int32, TWeakObjectPtr<AVFXActor>> ActiveFireVFXActors;

    UPROPERTY(EditDefaultsOnly, Category="Fire|Materials")
    UMaterialInterface* BurnMaterial = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="Fire|Materials")
    UMaterialParameterCollection* MPC_Fire = nullptr;
    
    // 머티리얼 변경을 제외할 액터 태그들
    UPROPERTY(EditAnywhere, Category="Fire|Materials", meta=(ToolTip="이 태그를 가진 액터는 BurnMaterial로 변경되지 않음 (예: Floor, Ground)"))
    TArray<FName> ExcludedActorTags = { FName("Floor"), FName("Ground") };

    //런타임 데이터. 빈쯤 탄 상태 고정, 슬롯 캐쉬
    UPROPERTY(Transient)
    TMap<TWeakObjectPtr<UPrimitiveComponent>, TWeakObjectPtr<UMaterialInstanceDynamic>> BurnMICache;
    TMap<TTuple<TWeakObjectPtr<UMeshComponent>, int32>, TWeakObjectPtr<UMaterialInstanceDynamic>> BurnMICacheBySlot;
    
    // 각 메시 슬롯이 몇 개의 셀에서 사용되는지 추적 (참조 카운팅)
    TMap<TTuple<TWeakObjectPtr<UMeshComponent>, int32>, int32> BurnMIRefCount;

    // 점진적 소화 애니메이션을 위한 데이터
    UPROPERTY(Transient)
    TMap<int32, FTimerHandle> ExtinguishAnimationTimers;
    UPROPERTY(Transient)
    TMap<int32, float> ExtinguishAnimationProgress;  // 0.0 ~ 1.0

};
