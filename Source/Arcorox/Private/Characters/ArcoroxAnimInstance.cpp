// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ArcoroxAnimInstance.h"
#include "Characters/ArcoroxCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UArcoroxAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ArcoroxCharacter = Cast<AArcoroxCharacter>(TryGetPawnOwner());
	if (ArcoroxCharacter)
	{
		ArcoroxCharacterMovement = ArcoroxCharacter->GetCharacterMovement();
	}
}

void UArcoroxAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ArcoroxCharacterMovement)
	{
		Speed = UKismetMathLibrary::VSizeXY(ArcoroxCharacterMovement->Velocity);
		bIsFalling = ArcoroxCharacterMovement->IsFalling();
		bIsAccelerating = ArcoroxCharacterMovement->GetCurrentAcceleration().Size() > 0.f;
	}
}
