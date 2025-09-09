#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZoneActivableComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HELLTECH_API UZoneActivableComponent : public UActorComponent
{
	GENERATED_BODY()
public:	

	// Indicates Zone
	UPROPERTY(EditAnywhere, Category = "Zone")
	FName ZoneTag;

	// If true actor is deactivated on start
	UPROPERTY(EditAnywhere, Category = "Zone")
	bool bStartDisabled = true;

	// HordManager calls it when zone is triggered
	UFUNCTION(Category = "Zone")
	void ActivateForZone(const FName& ActiveZone);
	
protected:
	virtual void BeginPlay() override;
	void SetEnabled(bool bEnabled);
};
