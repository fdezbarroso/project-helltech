#include "Game/ZoneTrigger.h"
#include "GameFramework/Character.h"

AZoneTrigger::AZoneTrigger()
{
	OnActorBeginOverlap.AddDynamic(this, &AZoneTrigger::HandleOverlap);
}

void AZoneTrigger::BeginPlay()
{
	Super::BeginPlay();
}

void AZoneTrigger::HandleOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor) {return;}

	if (Cast<ACharacter>(OtherActor) && OtherActor->ActorHasTag(FName("Player")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player %s entered the area! : %s and active hordes are %hhd"), *(OtherActor->GetName()), *ZoneTag.ToString(), bStartHordes);
		OnZoneActivated.Broadcast(ZoneTag, bStartHordes);
	}
}