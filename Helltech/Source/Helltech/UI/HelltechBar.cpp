


#include "UI/HelltechBar.h"
#include "Blueprint/WidgetTree.h"
#include "Characters/Helltech/PROVISIONAL_HelltechCharacter.h"

#define VELOCITY_BOOST_DIVIDER 1000

void UHelltechBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (HelltechProgressBar)
	{
		if (bBarCanPassiveMove)
		{
			HelltechProgressBar->SetPercent(FMath::Clamp(HelltechProgressBar->GetPercent() + ActualHelltechProgressionVelocity / VELOCITY_BOOST_DIVIDER, 0, 1));
		}
		
		if (HelltechProgressBar->GetPercent() >= 1 && !IsHelltechModeActivated())
		{
			OnHelltechModeActivation.Broadcast(true, HelltechModeDefaultBoosts);
		}
		else if (HelltechProgressBar->GetPercent() <= 0 && IsHelltechModeActivated())
		{
			OnHelltechModeActivation.Broadcast(false, HelltechModeDefaultBoosts);
		}
	}

	//Smooth camera FOV
	if (HelltechCharacterCamera)
	{
		HelltechCharacterCamera->FieldOfView = FMath::FInterpTo(HelltechCharacterCamera->FieldOfView, TargetCameraFOV, InDeltaTime, CameraFOVInterpSpeed);
	}

	if (bHelltechModeActive)
	{
		ActualHelltechProgressionVelocity = -DescendBarVelocity;
	}
	else
	{
		ActualHelltechProgressionVelocity = HelltechPassiveProgressionVelocity;
	}
}

void UHelltechBar::NativeConstruct()
{
	Super::NativeConstruct();

	OnHelltechModeActivation.AddDynamic(this, &UHelltechBar::ActivateHelltechMode);
	
	if (HelltechCharacterCamera)
	{
		TargetCameraFOV = HelltechCharacterCamera->FieldOfView;
	}
	SetHelltechBar(nullptr);
}

void UHelltechBar::SetHelltechBar(UProgressBar* HelltechBar)
{
	if (HelltechBar)
	{
		this->HelltechProgressBar = HelltechBar;
	}
	else if (UWidget* FoundWidget = WidgetTree->FindWidget("HelltechModeProgressBar"))
	{
		if (UProgressBar* HelltechProgressBarTemp = dynamic_cast<UProgressBar*>(FoundWidget))
		{
			this->HelltechProgressBar = HelltechProgressBarTemp;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR. HelltechBar.cpp: No se ha podido encontrar la barra de progreso Helltech"));
	}
}

void UHelltechBar::PassiveBarMovement(float Velocity)
{
	HelltechPassiveProgressionVelocity = Velocity;
}

void UHelltechBar::StopHelltechBarMovement()
{
	bBarCanPassiveMove = false;
}

void UHelltechBar::ResumeHelltechBarMovement()
{
	bBarCanPassiveMove = true;
}

void UHelltechBar::BoostBarMovement(float Boost)
{
	HelltechProgressBar->SetPercent(HelltechProgressBar->GetPercent() + Boost / 100);
}

void UHelltechBar::ActivateHelltechMode(bool bActivate, FHelltechModeBoosts HelltechModeBoosts)
{
	if (GEngine)
	{
		FString Message = bActivate ? TEXT("HelltechMode: ACTIVATED") : TEXT("HelltechMode: DEACTIVATED");
		FColor MessageColor = bActivate ? FColor::Green : FColor::Red;

		GEngine->AddOnScreenDebugMessage(-1, 5.f, MessageColor, Message);
	}
	if (bActivate)
	{
		ActualHelltechProgressionVelocity = -DescendBarVelocity;
		bHelltechModeActive = true;
	}
	else
	{
		ActualHelltechProgressionVelocity = HelltechPassiveProgressionVelocity;
		bHelltechModeActive = false;
	}

	///TODO: Implementar el uso de HelltechModeBoosts para el cambio de FOV y velocidad en el character
	if (HelltechCharacterCamera)
	{
		if (bActivate)
		{
			TargetCameraFOV = HelltechCharacterCamera->FieldOfView + HelltechModeBoosts.CameraFOVBoost;
		}
		else
		{
			TargetCameraFOV = HelltechCharacterCamera->FieldOfView - HelltechModeBoosts.CameraFOVBoost;
		}
	}
	if (HelltechCharacter)
	{
		if (bActivate)
		{
			HelltechCharacter->MaxWalkSpeed += HelltechModeBoosts.PlayerSpeedBoost;
		}
		else
		{
			HelltechCharacter->MaxWalkSpeed -= HelltechModeBoosts.PlayerSpeedBoost;
		}
	}
}

void UHelltechBar::BindToPlayer(AActor* PlayerActor)
{
	if (!PlayerActor) return;

	BoundPlayer = PlayerActor;

	PlayerActor->OnTakeAnyDamage.AddDynamic(this, &UHelltechBar::HandleTakeAnyDamage);
	Cast<APROVISIONAL_HelltechCharacter>(PlayerActor)->OnDamageDealt.AddDynamic(this, &UHelltechBar::OnDamageApplied);
}

void UHelltechBar::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (DamagedActor && DamagedActor == BoundPlayer)
	{
		OnDamageTaken(Damage);
	}
}
