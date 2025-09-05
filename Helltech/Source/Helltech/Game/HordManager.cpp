#include "Game/HordManager.h"

#include "AbilitySystemComponent.h"
#include "ZoneActivableComponent.h"
#include "ZoneTrigger.h"
#include "Characters//Helltech/EnemyBase.h"
#include "Abilities/HelltechAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

AHordManager::AHordManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHordManager::BeginPlay()
{
	Super::BeginPlay();

	RefreshSpawnPoints();
	
	// TArray<AActor*> Triggers;
	// UGameplayStatics::GetAllActorsOfClass(GetWorld(),AZoneTrigger::StaticClass(), Triggers);
	//
	// for (AActor* A : Triggers)
	// {
	// 	if (AZoneTrigger* Trig = Cast<AZoneTrigger>(A))
	// 	{
	// 		Trig->OnZoneActivated.AddDynamic(this, &AHordManager::OnZoneActivated);
	// 	}
	// }
}

void AHordManager::RefreshSpawnPoints()
{
	ActiveSpawnPoints.Reset();
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),ASpawnPoint::StaticClass(),Found);
	for (AActor* A : Found)
	{
		if (ASpawnPoint* sp = Cast<ASpawnPoint>(A))
		{
			ActiveSpawnPoints.Add(sp);
		}
	}
}

void AHordManager::OnZoneActivated(FName ZoneTag, bool bStartHordes)
{
	UE_LOG(LogTemp, Display, TEXT("Zone %s is activated on HordManager"), *ZoneTag.ToString());
	SetActiveZone(ZoneTag);
	ActivateStaticEnemis(ZoneTag);
	if (bStartHordes)
	{
		BeginZone();
	}
}

void AHordManager::SetActiveZone(const FName& ZoneTag)
{
	CurrentZone = ZoneTag;
	ActiveSpawnPoints.Reset();
	for (ASpawnPoint* sp : ActiveSpawnPoints)
	{
		if (sp && sp->SpawnZoneTag == ZoneTag)
		{
			ActiveSpawnPoints.Add(sp);
		}
	}
}

void AHordManager::ActivateStaticEnemis(const FName& ZoneTag)
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),AEnemyBase::StaticClass(),Actors);
	for (AActor* A : Actors)
	{
		if (!A) continue;
		UZoneActivableComponent* Comp = A->FindComponentByClass<UZoneActivableComponent>();
		if (Comp)
		{
			Comp->ActivateForZone(ZoneTag);
		}
	}
}

void AHordManager::BeginZone()
{
	CurrentWave = -1;
	StartNextWave();
}

void AHordManager::StartNextWave()
{
	if (!WavesByZone.Contains(CurrentZone)) return;

	const TArray<FHordeWave>& Waves = WavesByZone[CurrentZone].Waves;
	CurrentWave++;	

	if (!Waves.IsValidIndex(CurrentWave))
	{
		return;
	}

	ActiveWave = Waves[CurrentWave];
	RemainingToSpawn = ActiveWave.Count;

	GetWorldTimerManager().SetTimer(WaveTimer, [this]()
	{
		GetWorldTimerManager().SetTimer(SpawnTimer, this, &AHordManager::SpawnOne, ActiveWave.SpawnInterval, true, 0.f);
	}, ActiveWave.DelayBeforeWave, false);
}

void AHordManager::OnEnemyDestroyed(AActor* DestroyedActor)
{
	EnemiesAlive--;

	if (EnemiesAlive <= 0)
	{
		StartNextWave();
	}
}


void AHordManager::SpawnOne()
{
	if (RemainingToSpawn <= 0) {
		GetWorldTimerManager().ClearTimer(SpawnTimer);

		if (bWaitAllDead) {
			FTimerHandle WaitHandle;
			GetWorldTimerManager().SetTimer(WaitHandle, [this]()
			{
				if (EnemiesAlive <= 0)
				{
					StartNextWave();
				}
			}, 0.5f, true);
		}
		else {
			GetWorldTimerManager().SetTimer(WaveTimer, this, &AHordManager::StartNextWave, TimeBetweenWaves, false);
		}
		return;
	}
	
	ASpawnPoint* SpawnP = PickRandomSpawn();
	if (!SpawnP ||!ActiveWave.EnemyClass) {
		RemainingToSpawn = 0;
		return;
	}

	FVector Location = SpawnP->GetActorLocation();
	FRotator Rotation = FRotator::ZeroRotator;
	AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(ActiveWave.EnemyClass, Location, Rotation);

	if (SpawnedEnemy)
	{
		SpawnedEnemy->IncreaseDifficulty(ActiveWave.HealthMultiplier, ActiveWave.DamageMultiplier);
		EnemiesAlive++;
		SpawnedEnemy->OnDestroyed.AddDynamic(this, &AHordManager::OnEnemyDestroyed);
	}
	RemainingToSpawn--;
}

ASpawnPoint* AHordManager::PickRandomSpawn() const
{
	if (ActiveSpawnPoints.Num() == 0) return nullptr;

	int32 Weigth = 0;
	for (ASpawnPoint* sp : ActiveSpawnPoints)
	{
		Weigth += FMath::Max(1, sp->Weight);
	}
	int32 Pick = FMath::RandRange(1, Weigth);
	int32 i = 0;
	for (ASpawnPoint* SP : ActiveSpawnPoints)
	{
		i += FMath::Max(1, SP->Weight);
		if (Pick <= i)
		{
			return SP;
		}
	}
	return ActiveSpawnPoints.Last();
}

