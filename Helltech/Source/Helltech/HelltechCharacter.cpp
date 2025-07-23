// Fill out your copyright notice in the Description page of Project Settings.


#include "HelltechCharacter.h"

// Sets default values
AHelltechCharacter::AHelltechCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHelltechCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHelltechCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDashing)
	{
		//DashUpdate(DeltaTime);
	}
}

// Called to bind functionality to input
void AHelltechCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AHelltechCharacter::Dash()
{
	// If dash is not available then don't do it
	if (!bCanDash || bIsDashing)
	{
		return;
	}

	///TODO: Seguir desde aquí
}
