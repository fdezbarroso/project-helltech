#include "Characters/Helltech/PROVISIONAL_HelltechCharacter.h"

#include "EnhancedInputComponent.h"
#include "HelltechCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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

	previousWallRunHorizontalSpeed = WallRunSpeedHorizontal;
	previousWallRunVerticalSpeed = WallRunSpeedVertical;
	previousGravityScale = GetCharacterMovement()->GravityScale;
	
	if (!PlayerCamera)
	{
		PlayerCamera = FindComponentByClass<UCameraComponent>();
	}
	for (UActorComponent* Component : this->GetComponents())
	{
		if (Component->GetName() == TEXT("DetectWallCapsule"))
		{
			UCapsuleComponent* CapsuleComponentVar = Cast<UCapsuleComponent>(Component);
			if (CapsuleComponentVar != nullptr)
			{
				WallCapsuleDetector = CapsuleComponentVar;
				break;
			}
			
		}
	}
	if (!WallCapsuleDetector || WallCapsuleDetector->GetName() != "DetectWallCapsule")
	{
		WallCapsuleDetector = CreateDefaultSubobject<UCapsuleComponent>(TEXT("DetectWallCapsule"));
		WallCapsuleDetector->SetCapsuleHalfHeight(WallDetectionCapsuleHalfHeight);
		WallCapsuleDetector->SetCapsuleRadius(WallDetectionCapsuleRadius);
		WallCapsuleDetector->SetupAttachment(RootComponent);
	}
	WallCapsuleDetector->OnComponentBeginOverlap.AddDynamic(this, &APROVISIONAL_HelltechCharacter::OnCollisionBeginDetectWallrunCapsule);
	WallCapsuleDetector->OnComponentEndOverlap.AddDynamic(this, &APROVISIONAL_HelltechCharacter::OnCollisionEndDetectWallrunCapsule);
	IsWidgetClassInViewport(GetWorld(), UDashProgressBarWidget::StaticClass());
}

// Called every frame
void APROVISIONAL_HelltechCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//--------------------WALLRUN-------------------

	if (previousWallRunHorizontalSpeed == 0.f)
	{
		WallRunSpeedHorizontal = GetCharacterMovement()->GetMaxSpeed();
	}
	if (previousWallRunVerticalSpeed == 0.f)
	{
		WallRunSpeedVertical = GetCharacterMovement()->GetMaxSpeed();
	}
	
	if (bIsTouchingWall && bIsWallRunning)
	{
		bCanDoWallRunning = false;
	}
	else if (!bIsTouchingWall && !bIsWallRunning)
	{
		bCanDoWallRunning = true;
	}

	// CheckForWall();

	if (bIsWallRunning)
	{
		CheckWallRunCollision();
		PerformWallRunMovement();
	}
	UpdateCameraTilt(DeltaTime);

	//---------------------DASH---------------------
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
	ForwardAxisValue = result.Y;
	RightAxisValue = result.X;
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

// void APROVISIONAL_HelltechCharacter::CheckForWall()
// {
// 	if (GetCharacterMovement()->IsFalling())
//     {
//         FVector Start = GetActorLocation();
//         FVector Right = GetActorRightVector();
// 		FVector End = Start + GetVelocity().GetSafeNormal() * WallDetectionDistance;
//
//         TArray<FHitResult> Hits;
//         FCollisionQueryParams Params;
//         Params.AddIgnoredActor(this);
//
// 		FCollisionShape Capsule = FCollisionShape::MakeCapsule(WallDetectionCapsuleRadius, WallDetectionCapsuleHalfHeight);
// 		
//         if (WallrunDebug)
//         {
//         	DrawDebugCapsule(GetWorld(), End, Capsule.GetCapsuleHalfHeight(), Capsule.GetCapsuleRadius(), FQuat::Identity, FColor::Cyan, false, 0.1);
//         }
//
//         bool bHit = GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, WallRunTraceChannel, Capsule, Params);
//
// 		FHitResult Hit = Hits[0];
// 		if (WallRunClass)
// 		{
// 			for (int i = 0; i < Hits.Num(); i++)
// 			{
// 				if (Hits[i].GetActor()->IsA(WallRunClass))
// 				{
// 					Hit = Hits[i];
// 					break;
// 				}
// 			}
// 		}
// 		
// 		if (bHit && CanSurfaceBeWallrun(Hit))
// 		{
// 			float DotRight = FVector::DotProduct(Hit.ImpactNormal, Right);
//
// 			if (DotRight >= 0.f)
// 			{
// 				StartWallRun(EWallRunSide::Right, Hit.ImpactNormal, Hit.GetActor());
// 			}
// 			else if (DotRight < 0.f)
// 			{
// 				StartWallRun(EWallRunSide::Left, Hit.ImpactNormal, Hit.GetActor());
// 			}
// 		}
//     }
//     else if (bIsWallRunning)
//     {
//         // Si no estamos cayendo pero seguimos en wallrun, pararlo
//         StopWallRun();
//     }
// }

void APROVISIONAL_HelltechCharacter::CheckWallRunCollision()
{
	if (!bIsWallRunning) return;

	FVector Start = GetActorLocation();
	FVector CheckDirection = (CurrentWallRunSide == EWallRunSide::Right) ? GetActorRightVector() : -GetActorRightVector();

	if (WallrunDebug)
	{
		DrawDebugCapsule(GetWorld(), Start - CheckDirection * WallDetectionDistance, WallDetectionCapsuleHalfHeight, WallDetectionCapsuleRadius, FQuat::Identity, FColor::Blue, false, 0.1f);
	}
}

// Si tiene tag, no está poco empinado y el player está mirando lo suficiente a la pared
bool APROVISIONAL_HelltechCharacter::CanSurfaceBeWallrun(const FHitResult& Hit) const
{
	AActor* HitActor = Hit.GetActor();
	// Verificar que el actor tenga el tag correcto
	if (WallRunTag.IsNone() || !Hit.GetActor() || !Hit.GetActor()->ActorHasTag(WallRunTag))
	{
		return false;
	}

	// Verificar que la superficie no esté demasiado horizontal (poco empinada)
	if (FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector) > 0.2f)
	{
		return false;
	}

	if (bIsWallRunning)
	{
		return true;
	}

	// Verificar el ángulo de visión respecto a la pared
	FVector Forward = GetActorForwardVector();
	FVector WallDir = FVector::CrossProduct(Hit.ImpactNormal, FVector::UpVector).GetSafeNormal();

	// MEJORADO: Usar tanto la dirección hacia adelante como hacia atrás de la pared
	float AngleForward = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, WallDir)));
	float AngleBackward = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, -WallDir)));
    
	float MinAngle = FMath::Min(AngleForward, AngleBackward);
    
	return MinAngle < MaxViewAngleFromWall;
}

void APROVISIONAL_HelltechCharacter::StartWallRun(EWallRunSide Side, const FVector& WallNormal, AActor* Wall)
{
	WallNormalVar = WallNormal;
	CurrentWallRunSide = Side;
	WallRunWall = Wall;

	if (!bCanDoWallRunning)
	{
		return;
	}
	bIsWallRunning = true;
	// Guardar y cambiar la gravedad
	GetCharacterMovement()->GravityScale = WallRunGravityScale;

	// Configurar timer de duración
	if (WallRunDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(WallRunTimerHandle, this, &APROVISIONAL_HelltechCharacter::StopWallRun, WallRunDuration, false);
	}

	UE_LOG(LogTemp, Log, TEXT("Started Wallrun on %s side"), Side == EWallRunSide::Right ? TEXT("Right") : TEXT("Left"));
}

void APROVISIONAL_HelltechCharacter::StopWallRun()
{
	bIsWallRunning = false;
	WallRunWall = nullptr;
	WallNormalVar = FVector::ZeroVector;
	CurrentWallRunSide = EWallRunSide::None;
    
	// Restaurar gravedad
	GetCharacterMovement()->GravityScale = previousGravityScale;

	// Limpiar timer
	GetWorldTimerManager().ClearTimer(WallRunTimerHandle);

	UE_LOG(LogTemp, Log, TEXT("Stopped Wallrun"));
}

void APROVISIONAL_HelltechCharacter::PerformWallRunMovement()
{
	if (!bIsWallRunning) return;

	// Obtener dirección del input del controlador

	FVector InputVector;
    
	// Calcular dirección de movimiento a lo largo de la pared
	FVector WallDir;
	float WallRunSpeedChosen = 0.0f;
	if (WallRunWall)
	{
		if (bIsWallrunningUp)
		{
			InputVector = (FVector::UpVector * ForwardAxisValue).GetSafeNormal();
			WallDir = FVector::UpVector;
			WallRunSpeedChosen = WallRunSpeedVertical;
		}
		else
		{
			FRotator ControlRot = Controller->GetControlRotation();
			FRotator YawRot(0, ControlRot.Yaw, 0);

			FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
			FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
			InputVector = (ForwardDir * ForwardAxisValue + RightDir * RightAxisValue).GetSafeNormal();
			WallDir = WallRunWall->GetActorForwardVector();
			WallRunSpeedChosen = WallRunSpeedHorizontal;
		}
	}
	else
	{
		if (bIsWallrunningUp)
		{
			InputVector = (FVector::UpVector * ForwardAxisValue).GetSafeNormal();
			WallDir = FVector::UpVector;
			WallRunSpeedChosen = WallRunSpeedVertical;
		}
		else
		{
			
			FRotator ControlRot = Controller->GetControlRotation();
			FRotator YawRot(0, ControlRot.Yaw, 0);

			FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
			FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
			InputVector = (ForwardDir * ForwardAxisValue + RightDir * RightAxisValue).GetSafeNormal();
			WallDir = FVector::CrossProduct(WallNormalVar, FVector::UpVector).GetSafeNormal();
			WallRunSpeedChosen = WallRunSpeedHorizontal;
		}
	}
    
	// Proyectar el input del jugador sobre la dirección de la pared
	FVector WallMovement = FVector::DotProduct(InputVector, WallDir) * WallDir;
    
	// También permitir movimiento en la dirección opuesta
	FVector OppositeWallDir = -WallDir;
	FVector OppositeWallMovement = FVector::DotProduct(InputVector, OppositeWallDir) * OppositeWallDir;
    
	// Usar el movimiento con mayor magnitud
	if (OppositeWallMovement.Size() > WallMovement.Size())
	{
		WallMovement = OppositeWallMovement;
	}

	// Aplicar velocidad de wallrun
	FVector WallRunVelocity = WallMovement * WallRunSpeedChosen;
    
	GetCharacterMovement()->Velocity = FVector(WallRunVelocity.X, WallRunVelocity.Y, WallRunVelocity.Z);
}

void APROVISIONAL_HelltechCharacter::OnCollisionBeginDetectWallrunCapsule(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bIsWallRunning)
	{
		if (OtherActor->ActorHasTag(WallRunTag))
		{
			bIsTouchingWall = true;
		}
	}
	FVector Start = GetActorLocation();
	FVector Right = GetActorRightVector();
	FVector End = Start + GetVelocity().GetSafeNormal() * WallDetectionDistance;

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FCollisionShape Capsule = FCollisionShape::MakeCapsule(WallDetectionCapsuleRadius, WallDetectionCapsuleHalfHeight);
	
	if (WallrunDebug)
	{
		DrawDebugCapsule(GetWorld(), End, Capsule.GetCapsuleHalfHeight(), Capsule.GetCapsuleRadius(), FQuat::Identity, FColor::Cyan, false, 0.1);
	}

	bool bHit = GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, WallRunTraceChannel, Capsule, Params);

	FHitResult Hit = Hits[0];
	if (WallRunClass)
	{
		for (int i = 0; i < Hits.Num(); i++)
		{
			if (Hits[i].GetActor() && Hits[i].GetActor()->IsA(WallRunClass))
			{
				Hit = Hits[i];
				break;
			}
		}
	}
	
	if (bHit && CanSurfaceBeWallrun(Hit))
	{
		float DotRight = FVector::DotProduct(Hit.ImpactNormal, Right);

		if (DotRight >= 0.f)
		{
			StartWallRun(EWallRunSide::Right, Hit.ImpactNormal, Hit.GetActor());
		}
		else if (DotRight < 0.f)
		{
			StartWallRun(EWallRunSide::Left, Hit.ImpactNormal, Hit.GetActor());
		}
	}
}

void APROVISIONAL_HelltechCharacter::OnCollisionEndDetectWallrunCapsule(UPrimitiveComponent* OverlappedComp,
                                                                        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag(WallRunTag))
	{
		StopWallRun();
		bIsTouchingWall = false;
	}
}

void APROVISIONAL_HelltechCharacter::UpdateCameraTilt(float DeltaTime)
{
	UCameraComponent* Camera = nullptr;
    
	if (PlayerCamera){
		Camera = PlayerCamera;
	}
	else
	{
		TArray<UCameraComponent*> CameraComponents;
		GetComponents<UCameraComponent>(CameraComponents);
        
		if (CameraComponents.Num() > 0)
		{
			Camera = CameraComponents[0];
		}
	}
    
	if (Camera && Controller)
	{
		FVector ForwardCamera = Camera->GetForwardVector().GetSafeNormal();
		FVector RightCamera = Camera->GetRightVector().GetSafeNormal();
		FVector WallNormal = WallNormalVar.GetSafeNormal();

		float Dot = FVector::DotProduct(ForwardCamera, WallNormal);
		float WallSide = FVector::DotProduct(RightCamera, WallNormal);
		Dot = FMath::Clamp(Dot, -1.f, 1.f);
       
		float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Abs(Dot))); // Usar Abs aquí

		FVector Cross = FVector::CrossProduct(WallNormal, ForwardCamera);
		if (Cross.Z < ProyectedZAngleToStartWallrunUp && Cross.Z > -ProyectedZAngleToStartWallrunUp && Dot < 0.f)
		{
			bIsWallrunningUp = true;
		}
		else
		{
			bIsWallrunningUp = false;
		}
		float Sign = FMath::Sign(Cross.Z);

		float Alpha = FMath::Clamp(Angle / 90.f, 0.f, 1.f);
		float TargetRoll = Alpha * WallRunCameraTilt * -Sign;
		
		// En lugar de SetControlRotation, usar AddControllerRollInput
		float CurrentRoll = Controller->GetControlRotation().Roll;
		float RollDifference = FMath::FindDeltaAngleDegrees(CurrentRoll, TargetRoll);

		float RollInput = 0.0f;
       
		// Solo aplicar input si hay una diferencia significativa
		if (FMath::Abs(RollDifference) > 0.1f)
		{
			RollInput = RollDifference * DeltaTime * CameraAlignInterpSpeed;
			AddControllerRollInput(RollInput);
		}
		
		// DEBUG: Imprimir valores
		if (GEngine && WallrunDebug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, 
				FString::Printf(TEXT("WallNormal: %s"), *WallNormal.ToString()));
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, 
				FString::Printf(TEXT("CameraForward: %s"), *ForwardCamera.ToString()));
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Blue, 
				FString::Printf(TEXT("Cross: %s | Sign: %.2f"), *Cross.ToString(), Sign));
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Blue, 
				FString::Printf(TEXT("Dot: %.2f"), Dot));
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, 
				FString::Printf(TEXT("Angle: %.1f | Alpha: %.2f | TargetRoll: %.2f"), 
				Angle, Alpha, TargetRoll));
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, 
			 FString::Printf(TEXT("WallSide: %.2f | Roll: %.2f"), 
			 WallSide, TargetRoll));
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Black,
				FString::Printf(TEXT("ControllerRotation: X: %.2f | Y: %.2f | Z: %.2f"),
					Controller->GetControlRotation().GetComponentForAxis(EAxis::X),
					Controller->GetControlRotation().GetComponentForAxis(EAxis::Y),
					Controller->GetControlRotation().GetComponentForAxis(EAxis::Z)));
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Magenta,
				FString::Printf(TEXT("TargetRoll: %.2f | CurrentRoll: %.2f | RollInput: %.2f | RollDiference: %.2f"), TargetRoll, CurrentRoll, RollInput, RollDifference));
		}
	}
}

void APROVISIONAL_HelltechCharacter::JumpPressed()
{
	if (bIsWallRunning)
	{
		// Obtener dirección de la cámara
		FRotator CameraRotation = Controller->GetControlRotation();
		FVector CameraForward = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::X);
        
		// Verificar si la cámara está apuntando hacia la pared
		float DotWithWallNormal = FVector::DotProduct(CameraForward, -WallNormalVar);
        
		FVector JumpDir;
        
		if (DotWithWallNormal > 0.3f) // Si está mirando hacia la pared
		{
			// Salto direccional alejándose de la pared
			JumpDir = FVector::UpVector;
			FVector AwayFromWall = (CurrentWallRunSide == EWallRunSide::Right) ? -GetActorRightVector() : GetActorRightVector();
			JumpDir += AwayFromWall * 0.5f;
			JumpDir.Normalize();
		}
		else
		{
			// Salto hacia donde apunta la cámara
			JumpDir = CameraForward;
			JumpDir.Z = FMath::Max(JumpDir.Z, 0.5f); // Asegurar componente vertical mínima
			JumpDir.Normalize();
		}
        
		LaunchCharacter(JumpDir * WallRunJumpPower, false, true);
		StopWallRun();
	}
	else
	{
		// Salto normal
		Super::Jump();
	}
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
