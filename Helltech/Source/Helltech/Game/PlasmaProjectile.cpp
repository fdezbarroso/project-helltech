#include "Game/PlasmaProjectile.h"

#include "SceneRenderTargetParameters.h"
#include "Characters/Helltech/EnemyBase.h"
#include "Characters/Helltech/PROVISIONAL_HelltechCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

APlasmaProjectile::APlasmaProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComp->InitSphereRadius(8.0f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComp->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	SphereComp->OnComponentHit.AddDynamic(this, &APlasmaProjectile::OnHit);
	RootComponent = SphereComp;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(SphereComp);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetRelativeScale3D(FVector(0.2f));

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	MovementComp->UpdatedComponent = SphereComp;
	MovementComp->InitialSpeed = InitialSpeed;
	MovementComp->MaxSpeed = InitialSpeed;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 3.0f;
}

void APlasmaProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlasmaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlasmaProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);
		APROVISIONAL_HelltechCharacter* Player = Cast<APROVISIONAL_HelltechCharacter>(OtherActor);
		if (Player)
		{
			Player->OnDamageDealt.Broadcast(Damage);
		}
		AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
		if (Enemy)
		{
			Enemy->GetDamage(Damage);
		}
		if (ImpactSound) UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.ImpactPoint);
	}
	
	Destroy();
}