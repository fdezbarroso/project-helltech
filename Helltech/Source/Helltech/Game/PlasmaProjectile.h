#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlasmaProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class HELLTECH_API APlasmaProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	APlasmaProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Movement")
	UProjectileMovementComponent* MovementComp;

	UPROPERTY(EditAnywhere, Category = "FX")
	UParticleSystem* ParticleImpact;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* ImpactSound;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float InitialSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;

public:	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, Category = "Damage")
	float Damage = 20.0f;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	TArray<UMaterialInstanceDynamic*> DynMats;
	FTimerHandle HitFlashTimer;
	void ClearHitFlash();
};
