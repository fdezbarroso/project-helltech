#include "Game/SpawnPoint.h"
#include "Components/BillboardComponent.h"

ASpawnPoint::ASpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	EditorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorIcon"));
	RootComponent = EditorIcon;
	EditorIcon->SetHiddenInGame(true);
#endif
}

