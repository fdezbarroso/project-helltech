#include "Game/PlasmaRifle.h"

#include "PlasmaProjectile.h"
#include "Kismet/GameplayStatics.h"

APlasmaRifle::APlasmaRifle()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetGenerateOverlapEvents(false);

	RootLocation = CreateDefaultSubobject<USceneComponent>(TEXT("RootLocation"));
	RootLocation->SetupAttachment(WeaponMesh);
	RootLocation->SetRelativeLocation(WeaponLocation);
}

void APlasmaRifle::BeginPlay()
{
	Super::BeginPlay();

	InitialDamage = WeaponDamage;
}

FActorSpawnParameters APlasmaRifle::MakeSpawnParameters() const
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = (GetOwner() ? Cast<APawn>(GetOwner()) : nullptr);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	return SpawnParameters;
}

void APlasmaRifle::StartFire()
{
	if (!GetWorldTimerManager().IsTimerActive(FireTimer))
	{
		GetWorldTimerManager().SetTimer(FireTimer, this, &APlasmaRifle::Fire, FireRate, true, 0.0f);
	}
}

void APlasmaRifle::StopFire()
{
	GetWorldTimerManager().ClearTimer(FireTimer);
}

void APlasmaRifle::Fire()
{
	if (!ProjectileClass) return;

	FVector WepLocation = RootLocation->GetComponentLocation();

	FVector CamLoc;
	FRotator CamRot;
	
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (AController* Controller = OwnerPawn->GetController())
		{
			Controller->GetPlayerViewPoint(CamLoc, CamRot);
		}
	}

	FVector TraceStart = CamLoc;
	FVector TraceEnd = CamLoc + (CamRot.Vector() * 10000.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	FVector AimPoint = TraceEnd;
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		AimPoint = Hit.ImpactPoint;
	}

	FVector AimDir = (AimPoint - WepLocation).GetSafeNormal();
	FRotator ShootRot = AimDir.Rotation();
	
	FActorSpawnParameters SpawnParameters = MakeSpawnParameters();
	APlasmaProjectile* Proj = GetWorld()->SpawnActor<APlasmaProjectile>(ProjectileClass, WepLocation, ShootRot, SpawnParameters);
	if (Proj)
	{
		Proj->Damage = WeaponDamage;
	}
	
	if (ShotParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShotParticle, WepLocation, ShootRot);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, WepLocation, ShootRot);
	}
}

float APlasmaRifle::MultiplyDamage(float DamageMultiplier)
{
	WeaponDamage = InitialDamage * DamageMultiplier;

	return WeaponDamage;
}


void APlasmaRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}