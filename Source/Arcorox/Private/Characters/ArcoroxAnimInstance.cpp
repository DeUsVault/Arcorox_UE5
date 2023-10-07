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
	RootYawOffset(0.f),
	RotationCurve(0.f),
	RotationCurveLastFrame(0.f)
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
		//Clamp RootYawOffset between [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - DeltaYaw);
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };
			//RootYawOffset > 0  Turning Left, otherwise  Turning Right
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;
			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (ABSRootYawOffset > 90.f)
			{
				const float ExcessYaw{ ABSRootYawOffset - 90.f };
				RootYawOffset > 0 ? RootYawOffset -= ExcessYaw : RootYawOffset += ExcessYaw;
			}
		}
	}
	else
	{
		RootYawOffset = 0.f;
		CharacterRotationYaw = ArcoroxCharacter->GetActorRotation().Yaw;
		CharacterRotationYawLastFrame = CharacterRotationYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
}
