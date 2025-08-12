#include "Characters/Helltech/PROVISIONAL_HelltechCharacter.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/PanelWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APROVISIONAL_HelltechCharacter::APROVISIONAL_HelltechCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APROVISIONAL_HelltechCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerCamera = GetComponentByClass<UCameraComponent>();
	IsWidgetClassInViewport(GetWorld(), UDashProgressBarWidget::StaticClass());
}

// Called every frame
void APROVISIONAL_HelltechCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDashing)
	{
		DashElapsedTime += DeltaTime;
		float Alpha = DashElapsedTime / CODE_DashTime;

		if (Alpha >= 1.f)
		{
			bIsDashing = false;

			GetCharacterMovement()->Velocity = DashCurrentVelocity * CODE_FinalInertiaMultiplicator;
			
			GetCharacterMovement()->BrakingFrictionFactor = OriginalBrakingFrictionFactor;
			bTestGroundTouchedAfterDash = true;
			// Si está en el aire, mantener inercia en caida
			if (!GetCharacterMovement()->IsMovingOnGround())
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			}
			else
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}
			if (CODE_DashDebug)
			{
				DebugPostDashUntilTouchGround = true;
				DebugLastPlayerPositionUntilTouchGround = GetActorLocation();
			}
		}
		else
		{
			FVector Move = DashDirection * (CODE_DashDistance / CODE_DashTime) * DeltaTime;
			AddActorWorldOffset(Move, true);
		}
	}

	if (bTestGroundTouchedAfterDash && GetCharacterMovement()->IsMovingOnGround())
	{
		bTestGroundTouchedAfterDash = false;
		GetCharacterMovement()->AirControl = OriginalAirControl;
	}

	// DashProgressBar
	if (bCooldownStarted)
	{
		DashCooldownElapsedTime += DeltaTime;
		float Alpha = DashCooldownElapsedTime / CODE_DashCooldown;
		if (Alpha >= 1.f)
		{
			bCooldownStarted = false;
			DashCooldownElapsedTime = 0.f;
			if (DashProgressBar)
			{
				DashProgressBar->SetDashPercentCooldown(0);
			}
		}
		else
		{
			if (DashProgressBar)
			{
				DashProgressBar->SetDashPercentCooldown(1 - Alpha);
			}
		}
	}
	
#pragma region DEBUG_ZONE
	/* ---------------------- DEBUG ZONE ---------------------- */
	if (DebugPostDashUntilTouchGround && !GetCharacterMovement()->IsMovingOnGround())
	{
		ElapsedTimePostDashUntilTouchGround += DeltaTime;
		if (ElapsedTimePostDashUntilTouchGround > CooldownDebugUntilTouchGround)
		{
			ElapsedTimePostDashUntilTouchGround = 0;
			DrawDebugLine(GetWorld(), DebugLastPlayerPositionUntilTouchGround, GetActorLocation(), CODE_InertiaDebugColor, false, 20);
			DebugLastPlayerPositionUntilTouchGround = GetActorLocation();
		}
	}
	else if (DebugPostDashUntilTouchGround)
	{
		ElapsedTimePostDashUntilTouchGround = 0;
		DebugPostDashUntilTouchGround = false;
		DrawDebugLine(GetWorld(), DebugLastPlayerPositionUntilTouchGround, GetActorLocation(), CODE_InertiaDebugColor, false, 20);
	}
#pragma endregion DEBUG_ZONE
}

// Called to bind functionality to input
void APROVISIONAL_HelltechCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APROVISIONAL_HelltechCharacter::Dash()
{
	if (!bCanDash || bIsDashing) return;

	if (CODE_DashWithMovement && (GetCharacterMovement()->Velocity.X != 0 || GetCharacterMovement()->Velocity.Y != 0))
	{
		DashDirection = GetCharacterMovement()->Velocity.GetSafeNormal();
		if (GetCharacterMovement()->IsMovingOnGround() && PlayerCamera->GetComponentRotation().Pitch < 0.f)
		{
			DashDirection.Z = 0;
		}
		else
		{
			DashDirection.Z = PlayerCamera->GetForwardVector().Z;
		}
	}
	else if (GetCharacterMovement()->IsMovingOnGround() && PlayerCamera->GetComponentRotation().Pitch < 0.f)
	{
		DashDirection = GetActorForwardVector().GetSafeNormal();
	}
	else
	{
		DashDirection = PlayerCamera ? PlayerCamera->GetForwardVector().GetSafeNormal() : GetActorForwardVector().GetSafeNormal();
	}
	if (CODE_DashDebug)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + DashDirection*CODE_DashDistance, CODE_DashDebugColor, false, 20);
	}

	bIsDashing = true;
	bCanDash = false;
	bCooldownStarted = true;
	DashElapsedTime = 0.f;

	OriginalBrakingFrictionFactor = GetCharacterMovement()->BrakingFrictionFactor;
	OriginalAirControl = GetCharacterMovement()->AirControl;
	
	GetCharacterMovement()->BrakingFrictionFactor = 0;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->AirControl = 1.0f;

	DashCurrentVelocity = DashDirection * (CODE_DashDistance / CODE_DashTime);

	// Cooldown
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		this,
		&APROVISIONAL_HelltechCharacter::ResetDash,
		CODE_DashCooldown,
		false
	);
}

void APROVISIONAL_HelltechCharacter::ResetDash()
{
	bCanDash = true;
}

bool APROVISIONAL_HelltechCharacter::IsWidgetClassInViewport(UWorld* World, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!World || !*WidgetClass)
	{
		return false;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, WidgetClass, false);

	if (FoundWidgets.Num() > 0)
	{
		DashProgressBar = Cast<UDashProgressBarWidget>(FoundWidgets[0]);
		return true;
	}
	
	return false;
}