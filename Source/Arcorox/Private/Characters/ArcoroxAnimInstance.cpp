// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ArcoroxAnimInstance.h"
#include "Characters/ArcoroxCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UArcoroxAnimInstance::UArcoroxAnimInstance() :
	Speed(0.f),
	bIsFalling(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	bAiming(false),
	CharacterRotationYaw(0.f),
	CharacterRotationYawLastFrame(0.f),
	RootYawOffset(0.f)
{

}

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

	if (ArcoroxCharacter && ArcoroxCharacterMovement)
	{
		Speed = UKismetMathLibrary::VSizeXY(ArcoroxCharacterMovement->Velocity);
		bIsFalling = ArcoroxCharacterMovement->IsFalling();
		bIsAccelerating = ArcoroxCharacterMovement->GetCurrentAcceleration().Size() > 0.f;

		FRotator AimRotation = ArcoroxCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ArcoroxCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		if (ArcoroxCharacter->GetVelocity().Size() > 0) LastMovementOffsetYaw = MovementOffsetYaw;

		bAiming = ArcoroxCharacter->IsAiming();
	}
	TurnInPlace();
}

void UArcoroxAnimInstance::TurnInPlace()
{
	if (ArcoroxCharacter == nullptr || ArcoroxCharacterMovement == nullptr) return;
	if (Speed <= 0)
	{
		CharacterRotationYawLastFrame = CharacterRotationYaw;
		CharacterRotationYaw = ArcoroxCharacter->GetActorRotation().Yaw;
		const float DeltaYaw{ CharacterRotationYaw - CharacterRotationYawLastFrame };
		RootYawOffset -= DeltaYaw;
	}
}
