// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXActor.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UParticleSystemComponent;
class UMaterialParameterCollection; 

UCLASS()
class HORIZONTAL_API AVFXActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVFXActor();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FName VFXName;

	UPROPERTY()
	USceneComponent* SceneComponent;
	
	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ParticleSystemComponent;

	UPROPERTY(EditAnywhere,Category = "VFX")
	UNiagaraSystem* NiagaraSystemAsset = nullptr;

	//메테리얼 파라미터 컬렉션
	UPROPERTY(EditAnywhere,Category = "Fire|Assets")
	UMaterialParameterCollection* MPC_Fire = nullptr;

	// === 런타임 파라미터 (cm, World Space) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|Params")
	FVector BurnCenter = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|Params", meta=(ClampMin="0"))
	float BurnRadius = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|Params", meta=(ClampMin="0.001"))
	float EdgeWidth = 80.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire|Params", meta=(ClampMin="0"))
	float BurnIntensity_G = 6.f;
	// === 간단한 반경 옵션 ===      
	UPROPERTY(EditAnywhere, Category="Fire|Anim")
	float TargetMaxRadius = 600.f;

	UPROPERTY(EditAnywhere, Category="Fire|Anim")
	float RadiusGrowSeconds = 1.0f;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	

	FName GetVFXName() const { return VFXName; };
	void SetVFXName(FName Name) { VFXName = Name; };
	void StartVFX();
	void StopVFX();

	//번짐제어하는 api(외부호출)
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void StartBurnAt(const FVector& WorldLocation, float StartRadius =0.0f, float MaxRadius = 600.f);

	UFUNCTION(BlueprintCallable, Category = "Fire")
	void SetBurnParams(const FVector& InCenter, float InRadius, float InEdgeWidth, float InIntensityG);

private:
	// === 내부 상태 ===                       
	float Elapsed = 0.f;
	bool  bAnimating = false;

	// === 동기화 헬퍼 ===                           
	void PushToNiagara() const;
	void PushToMPC() const;
};
