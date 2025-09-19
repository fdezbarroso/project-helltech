#include "Characters/Helltech/PROVISIONAL_HelltechCharacter.h"

#include "EnhancedInputComponent.h"
#include "HelltechCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "Game/PlasmaRifle.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/UnrealTypePrivate.h"

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

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Cast<APawn>(this);
	FVector SpawnLoc = GetActorLocation();
	FRotator SpawnRot = GetActorRotation();
	
	APlasmaRifle* CurWeapon = GetWorld()->SpawnActor<APlasmaRifle>(DefaultWeaponClass, SpawnLoc, SpawnRot, SpawnParams);

	if (!CurWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn weapon"));
		return;
	}
	
	CurrentWeapon = CurWeapon;

	const FName SocketName(TEXT("GripPoint"));
	USkeletalMeshComponent* ArmsMesh = nullptr;

	TArray<USkeletalMeshComponent*> Components;
	GetComponents<USkeletalMeshComponent>(Components);
	
	for (UActorComponent* Comp : Components)
	{
		USkeletalMeshComponent* Sk = Cast<USkeletalMeshComponent>(Comp);
		if (Sk && Sk->DoesSocketExist(SocketName))
		{
			ArmsMesh = Sk;
			break;
		}
	}
	
	if (!ArmsMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("No skeletal mesh with socket %s found on character. Attach weapon to actor root instead."), *SocketName.ToString());
		CurWeapon->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
	else
	{
		CurWeapon->AttachToComponent(ArmsMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
		CurWeapon->SetActorRelativeRotation(FRotator::ZeroRotator); 

		CurWeapon->SetActorRelativeRotation(WeaponRotation);

		CurWeapon->SetActorRelativeLocation(FVector(1.14, 13.54, -10));
		
		FName PropName = TEXT("HasRifle");
		SetHasRifleOnTrue();	
	}

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

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MovementInputAction)
		{
			EnhancedInput->BindAction(MovementInputAction, ETriggerEvent::Triggered, this, &APROVISIONAL_HelltechCharacter::DetectMovement);
			EnhancedInput->BindAction(MovementInputAction, ETriggerEvent::Completed, this, &APROVISIONAL_HelltechCharacter::DetectMovement);
		}
		if (FireInputAction)
		{
			UE_LOG(LogTemp, Warning, TEXT("FIRE"));
			EnhancedInput->BindAction(FireInputAction, ETriggerEvent::Triggered, this, &APROVISIONAL_HelltechCharacter::StartFire);
			EnhancedInput->BindAction(FireInputAction, ETriggerEvent::Completed, this, &APROVISIONAL_HelltechCharacter::StopFire);
		}
	}
}

void APROVISIONAL_HelltechCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void APROVISIONAL_HelltechCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void APROVISIONAL_HelltechCharacter::DetectMovement(const FInputActionValue& Value)
{
	FVector2D result = Value.Get<FVector2D>();
	if (FMath::IsNearlyEqual(result.Y, 1.f, ForwardMovementCameraTolerance / 100.f))
	{
		MovementKeys.bUp = true;
	}
	else
	{
		MovementKeys.bUp = false;
	}
	if (FMath::IsNearlyEqual(result.Y, -1.f, ForwardMovementCameraTolerance / 100.f))
	{
		MovementKeys.bDown = true;
	}
	else
	{
		MovementKeys.bDown = false;
	}
	if (FMath::IsNearlyEqual(result.X, 1.f, ForwardMovementCameraTolerance / 100.f))
	{
		MovementKeys.bRight = true;
	}
	else
	{
		MovementKeys.bRight = false;
	}
	if (FMath::IsNearlyEqual(result.X, -1.f, ForwardMovementCameraTolerance / 100.f))
	{
		MovementKeys.bLeft = true;
	}
	else
	{
		MovementKeys.bLeft = false;
	}
}

void APROVISIONAL_HelltechCharacter::Dash()
{
	if (!bCanDash || bIsDashing) return;

	//Si tiene DashWithMovement y está en movimiento
	if (CODE_DashWithMovement && (GetCharacterMovement()->Velocity.X != 0 || GetCharacterMovement()->Velocity.Y != 0))
	{
		if (MovementInputAction && MovementKeys.IsAnyInputPressed())
		{
			DashDirection = GetActorForwardVector() * (MovementKeys.bUp ? 1 : 0 + MovementKeys.bDown ? -1 : 0) +
				GetActorRightVector() * (MovementKeys.bRight ? 1.f : 0.f + MovementKeys.bLeft ? -1.f : 0.f);
		}
		else if (MovementInputAction && !MovementKeys.IsAnyInputPressed())
		{
			DashDirection = PlayerCamera ? PlayerCamera->GetForwardVector().GetSafeNormal() : GetActorForwardVector().GetSafeNormal();
		}
		else
		{
			DashDirection = GetCharacterMovement()->Velocity.GetSafeNormal();
		}
		//Si está mirando hacia abajo y está en el suelo
		if (GetCharacterMovement()->IsMovingOnGround() && PlayerCamera->GetComponentRotation().Pitch < 0.f)
		{
			//Hacia delante (Z)
			DashDirection.Z = 0;
		}
		else
		{
			//Si se está moviendo hacia delante
			if (!MovementKeys.bDown && !MovementKeys.bRight && !MovementKeys.bLeft)
			{
				//Hacia donde esté mirando (Z)
				DashDirection.Z = PlayerCamera->GetForwardVector().Z;
			}
			//Si se está moviendo hacia otra dirección (esquivar)
			else
			{
				DashDirection.Z = 0;
			}
		}
	}
	//Si está en el suelo y mirando hacia abajo y no se está moviendo
	else if (GetCharacterMovement()->IsMovingOnGround() && PlayerCamera->GetComponentRotation().Pitch < 0.f && GetCharacterMovement()->Velocity.IsNearlyZero())
	{
		//Dash hacia delante
		DashDirection = GetActorForwardVector().GetSafeNormal();
	}
	//Si no está en el suelo
	else
	{
		//Si tiene camara te mueves hacia donde apunte la camara, sino hacia delante
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
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
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
