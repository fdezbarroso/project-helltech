

#pragma once

//HACK:: ELIMINATE THIS TWO INCLUDES
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "HelltechBar.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FHelltechModeBoosts
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CameraFOVBoost = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerSpeedBoost = 1000;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHelltechModeActivation, bool, bActivate, FHelltechModeBoosts, HelltechModeBoosts);

UCLASS()
class HELLTECH_API UHelltechBar : public UUserWidget
{
	GENERATED_BODY()
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;

public:	
	UFUNCTION(BlueprintCallable, meta=(ToolTip="Set the widget progress bar as the target"))
	void SetHelltechBar(UProgressBar* HelltechBar);
	
	UFUNCTION(BlueprintCallable, meta=(ToolTip="Slowly increase the helltech bar"), Category = "HelltechUI|HELLTECH_Mode")
	void PassiveBarMovement(float Velocity);

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Stops Helltech Bar Movement"), Category = "HelltechUI|HELLTECH_Mode")
	void StopHelltechBarMovement();

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Resumes Helltech Bar Movement"), Category = "HelltechUI|HELLTECH_Mode")
	void ResumeHelltechBarMovement();
	
	UFUNCTION(BlueprintCallable, meta=(ToolTip="Boost of energy at the helltech bar"), Category = "HelltechUI|HELLTECH_Mode")
	void BoostBarMovement(float Boost);

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Activates the Helltech Mode"), Category = "HelltechUI|HELLTECH_Mode")
	void ActivateHelltechMode(bool bActivate, FHelltechModeBoosts HelltechModeBoosts = FHelltechModeBoosts());

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Gets the actual DescendBarVelocity"), Category = "HelltechUI|HELLTECH_Mode")
	float GetDescendBarVelocity() const
	{
		return DescendBarVelocity;
	}

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Set the DescendBarVelocity"), Category = "HelltechUI|HELLTECH_Mode")
	void SetDescendBarVelocity(float DescendBarVelocityTemp)
	{
		this->DescendBarVelocity = DescendBarVelocityTemp;
	}

	UFUNCTION(BlueprintCallable, meta=(ToolTip="Get the actual status of Helltech Mode"), Category = "HelltechUI|HELLTECH_Mode")
	bool IsHelltechModeActivated()
	{
		return bHelltechModeActive;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelltechUI|HELLTECH_Mode")
	UCameraComponent* HelltechCharacterCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelltechUI|HELLTECH_Mode")
	UCharacterMovementComponent* HelltechCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelltechUI|HELLTECH_Mode")
	float CameraFOVInterpSpeed = 8;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HelltechUI|HELLTECH_Mode")
	FHelltechModeBoosts HelltechModeDefaultBoosts = FHelltechModeBoosts();
private:
	UPROPERTY()
	UProgressBar* HelltechProgressBar = nullptr;

	FOnHelltechModeActivation OnHelltechModeActivation;
	float TargetCameraFOV;
	float DescendBarVelocity = 1.f;
	float HelltechPassiveProgressionVelocity = 1.f;
	float ActualHelltechProgressionVelocity = 0.0f;
	bool bBarCanPassiveMove = true;
	bool bHelltechModeActive = false;
};
