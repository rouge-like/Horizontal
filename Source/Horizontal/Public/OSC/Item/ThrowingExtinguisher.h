#pragma once

#include "CoreMinimal.h"
#include "OSC/Item/AimableItemBase.h"
#include "ThrowingExtinguisher.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;
class USphereComponent;
class APlayerBase;
class AActor;
class USplineComponent;
class USplineMeshComponent;

UCLASS()
class HORIZONTAL_API AThrowingExtinguisher : public AAimableItemBase
{
    GENERATED_BODY()

public:
    AThrowingExtinguisher();

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void HandleStartUse() override;
    virtual bool GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const override;
    virtual void HandlePickupAvailabilityChanged() override;

    /** 로컬 플레이어가 궤적 미리보기를 보여줄지 판정 */
    bool ShouldShowTrajectory() const;
    /** 에임 상태에서 예측 궤적을 디버그 라인으로 표시 */
    void UpdateTrajectoryVisualization();
    /** 궤적 시각화용 스플라인 메시 풀을 보장 */
    void EnsureSplineMeshPool();
    /** 플레이어 카메라 기준 투척 시작 위치 계산 */
    FVector GetThrowStartLocation(const APlayerBase& ThrowingPlayer) const;
    /** 1인칭 카메라를 우선 사용하여 투척 방향 산출 */
    FVector GetThrowDirection(const APlayerBase& ThrowingPlayer) const;
    /** 투사체 정지 시 물리/충돌 상태 초기화 */
    UFUNCTION()
    void HandleProjectileStop(const FHitResult& ImpactResult);
    
    /** 투척 시 에임 FOV를 즉시 복구하도록 클라이언트 처리 */
    UFUNCTION(Client, Reliable)
    void ClientHandleThrow();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Visual", meta=(AllowPrivateAccess="true"))
    USplineComponent* TrajectorySpline;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Visual", meta=(AllowPrivateAccess="true"))
    UStaticMesh* LineMesh;
    
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Visual", meta=(AllowPrivateAccess="true"))
    UMaterialInterface* LineMaterial;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Visual")
    int32 MaxSegments = 100;

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Visual")
    FVector2D LineMeshScale = FVector2D(1,1);
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient,Category="Visual")
    TArray<USplineMeshComponent*> ActiveSplineMeshes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Visual")
    UStaticMeshComponent* HitSphereComponent;
    
    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float ThrowSpeed = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float ProjectileGravityScale = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float TrajectorySimTime = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float TrajectorySimFrequency = 15.0f;

    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float HitRadius = 100.0f;
    
    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float CollisionRadius = 12.0f;

    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float TrajectoryRadius = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category="Throw")
    FLinearColor TrajectoryColor = FLinearColor::FromSRGBColor(FColor::Green);

    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float TrajectoryLineLifetime = 0.05f;

    UPROPERTY(EditDefaultsOnly, Category="Throw")
    TEnumAsByte<ECollisionChannel> TrajectoryTraceChannel = ECC_Visibility;

    UPROPERTY(EditDefaultsOnly, Category="Throw")
    FVector LocalThrowOffset = FVector(15.0f, 0.0f, 0.0f);

    UPROPERTY(EditDefaultsOnly, Category="Throw")
    FName VFXName;
    
    bool bInFlight = false;
    TWeakObjectPtr<APlayerBase> LastThrowingActor;
};