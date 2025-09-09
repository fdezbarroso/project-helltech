#include "Game/ZoneTrigger.h"

#include "HordManager.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"

AZoneTrigger::AZoneTrigger()
{
	OnActorBeginOverlap.AddDynamic(this, &AZoneTrigger::HandleOverlap);
}

void AZoneTrigger::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AHordManager> It(GetWorld()); It; ++It)
	{
		OnZoneActivated.AddDynamic(*It, &AHordManager::OnZoneActivated);
	}
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