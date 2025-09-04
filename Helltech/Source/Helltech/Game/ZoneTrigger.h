#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "ZoneTrigger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneActivated, FName, ZoneTag, bool, bStartHordes);

UCLASS()
class HELLTECH_API AZoneTrigger : public ATriggerBox
{
	GENERATED_BODY()

public:
	AZoneTrigger();

	// Tag declaring which zone it triggers
	UPROPERTY(EditAnywhere, Category = "Zone")
	FName ZoneTag;

	// If true, starts hordes in this zone
	UPROPERTY(EditAnywhere, Category = "Zone")
	bool bStartHordes;

	// Global event for when a zone is triggered.
	UPROPERTY(EditAnywhere, Category = "Zone")
	FOnZoneActivated OnZoneActivated;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
