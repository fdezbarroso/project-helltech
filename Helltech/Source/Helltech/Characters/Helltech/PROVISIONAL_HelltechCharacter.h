

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UI/DashProgressBarWidget.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "PROVISIONAL_HelltechCharacter.generated.h"

UENUM()
enum class EWallRunSide : uint8
{
	None,
	Left,
	Right
};

USTRUCT()
struct FMovementKeys2D_PROVISIONAL
{
	GENERATED_BODY()
	UPROPERTY()
	bool bRight = false;
	UPROPERTY()
	bool bLeft = false;
	UPROPERTY()
	bool bUp = false;
	UPROPERTY()
	bool bDown = false;

	bool IsAnyInputPressed() const
	{
		if (bRight || bLeft || bUp || bDown)
			return true;
		return false;
	}
};

UCLASS()
class HELLTECH_API APROVISIONAL_HelltechCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region InheritedFunctions

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
#pragma endregion InheritedFunctions

#pragma region Dash
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash", meta=(UIMin=0, UIMax=100, Units="Percent"))
	float ForwardMovementCameraTolerance = 20.f;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* MovementInputAction;

	UPROPERTY(VisibleAnywhere)
	FMovementKeys2D_PROVISIONAL MovementKeys;
	
	UFUNCTION(BlueprintCallable, Category="Dash")
	void Dash();

	void ResetDash();
#pragma endregion Dash

#pragma region WallRun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallRunSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallRunGravityScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallRunCameraTilt = 15.f;
#pragma endregion WallRun

#pragma region Utilities
protected:
	bool IsWidgetClassInViewport(UWorld* World, TSubclassOf<UUserWidget> WidgetClass);
	void DetectMovement(const FInputActionValue& Value);
#pragma endregion Utilities

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
