#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireManager.generated.h"

class UPrimitiveComponent;
class AVFXActor;
class AVFXManager;
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
    void ApplySuppressionInSphere(const FVector& Center, float Radius, float SuppressionAmount);

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

    FFireCellTickResult ProcessCellRecursive(int32 CellIndex, float DeltaSeconds);
    FFireCellTickResult ProcessCollapseRecursive(int32 CellIndex, float DeltaSeconds);

    bool AreBoxesPotentiallyAdjacent(const FBox& SourceBounds, const FBox& TargetBounds) const;
    bool AreBoxesAdjacent(const FBox& SourceBounds, const FBox& TargetBounds) const;

    bool EnsureCombustibleComponent(int32 CellIndex);
    
    void ActivateFireVFX(int32 CellIndex);

    void DeactivateFireVFX(int32 CellIndex);

    int32 FindCellIndexRecursive(int32 CellIndex, const FVector& WorldLocation) const;

    void ApplySuppressionInSphereRecursive(int32 CellIndex, const FVector& Center, double RadiusSquared, float SuppressionAmount);
    bool ApplySuppressionToCell(int32 CellIndex, float SuppressionAmount);

    UPROPERTY(Transient)
    TWeakObjectPtr<AVFXManager> VFXManager;

    UPROPERTY(Transient)
    TMap<int32, TWeakObjectPtr<AVFXActor>> ActiveFireVFXActors;
};
