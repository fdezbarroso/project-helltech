#include "Game/ZoneActivableComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "AIController.h"

void UZoneActivableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bStartDisabled)
	{
		SetEnabled(false);
	}
}

void UZoneActivableComponent::SetEnabled(bool bEnabled)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	Owner->SetActorHiddenInGame(!bEnabled);
	Owner->SetActorEnableCollision(bEnabled);
	Owner->SetActorTickEnabled(bEnabled);

	if (ACharacter* Char = Cast<ACharacter>(Owner))
	{
		if (bEnabled)
		{
			if (!Char->GetController())
			{
				Char->SpawnDefaultController();
			}
		}
		else
		{
			if (APawn* Pawn = Cast<APawn>(Owner))
			{
				if (AController* Controller = Pawn->GetController())
				{
					Controller->UnPossess();		
				}
			}
		}
	}
}

void UZoneActivableComponent::ActivateForZone(const FName& ActiveZone)
{
	if (ActiveZone.IsNone()) return;
	if (ActiveZone == ZoneTag)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enabling enemy!"));
		SetEnabled(true);
	}
}