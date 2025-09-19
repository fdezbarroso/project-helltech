#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlasmaRifle.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USoundBase;
class UParticleSystem;
class APlasmaProjectile;
class UNiagaraSystem;

UCLASS()
class HELLTECH_API APlasmaRifle : public AActor
{
	GENERATED_BODY()
	
public:	
	APlasmaRifle();

	void StartFire();
	void StopFire();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireRate = 0.25;

	UPROPERTY(EditAnywhere, Category="Weapon")
	TSubclassOf<APlasmaProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category="Weapon")
	int32 BulletsPerShot = 1;

	UPROPERTY(EditAnywhere, Category="Weapon")
	UParticleSystem* ShotParticle;

	UPROPERTY(EditAnywhere, Category="Weapon")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, Category="Weapon")
	USceneComponent* RootLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, Category="Weapon")
	float WeaponDamage = 20.0f;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	float MultiplyDamage(float DamageMultiplier);

private:
	FTimerHandle FireTimer;

	float InitialDamage = 20.0f; 
	
	void Fire();

	FActorSpawnParameters MakeSpawnParameters() const;

	UPROPERTY(EditAnywhere, Category="Weapon")
	FVector WeaponLocation = FVector(100.0f, 0.0f, 0.0f);
};
