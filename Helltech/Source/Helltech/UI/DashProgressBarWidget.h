

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DashProgressBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class HELLTECH_API UDashProgressBarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	float GetDashPercentCooldown();

	UFUNCTION()
	void SetDashPercentCooldown(float value);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DashPercentCooldown;
};
