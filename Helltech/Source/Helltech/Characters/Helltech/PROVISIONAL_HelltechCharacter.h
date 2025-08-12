

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "UI/DashProgressBarWidget.h"
#include "PROVISIONAL_HelltechCharacter.generated.h"

UCLASS()
class HELLTECH_API APROVISIONAL_HelltechCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APROVISIONAL_HelltechCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float CODE_DashDistance = 800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float CODE_DashTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float CODE_DashCooldown = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	bool CODE_DashWithMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float CODE_FinalInertiaMultiplicator = 0.3f;
	
	bool bIsDashing = false;
	bool bCanDash = true;

	bool bTestGroundTouchedAfterDash = false;
	bool bCooldownStarted = false;

	FVector DashDirection;
	FVector DashCurrentVelocity;
	float DashElapsedTime = 0.f;
	float DashCooldownElapsedTime = 0.f;

	float OriginalBrakingFrictionFactor;
	float OriginalAirControl;

	UPROPERTY()
	UDashProgressBarWidget* DashProgressBar;
	
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCamera;

	UFUNCTION(BlueprintCallable, Category="Dash")
	void Dash();

	void ResetDash();
	bool IsWidgetClassInViewport(UWorld* World, TSubclassOf<UUserWidget> WidgetClass);

#pragma region DEBUG_ZONE

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG")
	bool CODE_DashDebug = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG")
	FColor CODE_DashDebugColor = FColor::Red;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG")
	FColor CODE_InertiaDebugColor = FColor::Magenta;
	bool DebugPostDashUntilTouchGround = false;
	float ElapsedTimePostDashUntilTouchGround = 0.f;
	float CooldownDebugUntilTouchGround = 0.2f;
	FVector DebugLastPlayerPositionUntilTouchGround = FVector::ZeroVector;

#pragma endregion
};
