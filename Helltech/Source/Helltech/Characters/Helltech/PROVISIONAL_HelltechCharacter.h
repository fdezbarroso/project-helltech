

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
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Speed")
	float WallRunSpeedHorizontal = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Speed")
	float WallRunSpeedVertical = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallDetectionDistance = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallRunDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallRunJumpPower = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallDetectionCapsuleRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float WallDetectionCapsuleHalfHeight = 50.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="WallRun")
	UCapsuleComponent* WallCapsuleDetector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun")
	float ProyectedZAngleToStartWallrunUp = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Camera")
	float WallRunCameraTilt = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Camera")
	float CameraAlignInterpSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Camera")
	float WallRunViewOffset = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Surface")
	TSubclassOf<AActor> WallRunClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Surface")
	TEnumAsByte<ECollisionChannel> WallRunTraceChannel = ECC_WorldStatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Surface")
	FName WallRunTag = "WallRun";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|Conditions")
	float MaxViewAngleFromWall = 150.f;

private:
	EWallRunSide CurrentWallRunSide = EWallRunSide::None;
	FTimerHandle WallRunTimerHandle;
	bool bIsWallRunning = false;
	bool bCanDoWallRunning = true;
	bool bIsTouchingWall = false;
	float previousGravityScale = 0.f;
	FVector WallNormalVar;
	float ForwardAxisValue = 0.f;
	float RightAxisValue = 0.f;
	float WallRunGravityScale = 0.0f;
	UPROPERTY()
	AActor* WallRunWall;
	float CurrentWallrunTilt = 0.f;
	float TargetWallrunTilt = 0.f;
	bool bIsWallrunningUp = false;
	float previousWallRunHorizontalSpeed = 0.f;
	float previousWallRunVerticalSpeed = 0.f;

	// Main functions
	void CheckForWall();
	void CheckWallRunCollision();
	bool CanSurfaceBeWallrun(const FHitResult& Hit) const;
	void StartWallRun(EWallRunSide Side, const FVector& WallNormal, AActor* Wall);
	void StopWallRun();
	void PerformWallRunMovement();
	UFUNCTION()
	void OnCollisionBeginDetectWallrunCapsule(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	UFUNCTION()
	void OnCollisionEndDetectWallrunCapsule(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Camera
	void UpdateCameraTilt(float DeltaTime);

	// Input
	UFUNCTION(BlueprintCallable)
	void JumpPressed();
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|DEBUG")
	bool WallrunDebug = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WallRun|DEBUG")
	FColor WallrunDetectorColor = FColor::Cyan;
	bool DebugPostDashUntilTouchGround = false;
	float ElapsedTimePostDashUntilTouchGround = 0.f;
	float CooldownDebugUntilTouchGround = 0.2f;
	FVector DebugLastPlayerPositionUntilTouchGround = FVector::ZeroVector;

#pragma endregion
};
