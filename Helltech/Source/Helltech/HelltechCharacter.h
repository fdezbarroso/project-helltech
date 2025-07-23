// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HelltechCharacter.generated.h"

UCLASS()
class HELLTECH_API AHelltechCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region Functions
public:
	// Sets default values for this character's properties
	AHelltechCharacter();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Dash functions
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Dash();
    
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsDashing() const { return bIsDashing; }
    
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool CanDash() const { return bCanDash; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void DashUpdate(float DeltaTime);
	void StopDash();
	void ResetDashCooldown();
#pragma endregion
#pragma region Variables
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	float DashDistance = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true", Units = "Seconds"))
	float DashDuration = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true", Units = "Seconds"))
	float DashCooldown = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* DashSpeedCurve;

	// Dash state
	bool bIsDashing = false;
	bool bCanDash = true;
	FTimerHandle DashTimerHandle;
	FTimerHandle DashCooldownTimerHandle;
	FVector DashStartLocation;
	FVector DashTargetLocation;
	float CurrentDashTime = 0.0f;

#pragma endregion

};
