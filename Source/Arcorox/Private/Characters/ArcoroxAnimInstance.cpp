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
	TIPCharacterRotationYaw(0.f),
	TIPCharacterRotationYawLastFrame(0.f),
	RootYawOffset(0.f),
	RotationCurve(0.f),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	RotationCurveLastFrame(0.f),
	Pitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	DeltaYaw(0.f)
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

		bReloading = ArcoroxCharacter->GetCombatState() == ECombatState::ECS_Reloading;
		bAiming = ArcoroxCharacter->IsAiming();

		if (bReloading) OffsetState = EOffsetState::EOS_Reloading;
		else if (bIsFalling) OffsetState = EOffsetState::EOS_InAir;
		else if (bAiming) OffsetState = EOffsetState::EOS_Aiming;
		else OffsetState = EOffsetState::EOS_Hip;

	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UArcoroxAnimInstance::TurnInPlace()
{
	if (ArcoroxCharacter == nullptr || ArcoroxCharacterMovement == nullptr) return;
	Pitch = ArcoroxCharacter->GetBaseAimRotation().Pitch;
	if (Speed > 0 || bIsFalling)
	{
		RootYawOffset = 0.f;
		TIPCharacterRotationYaw = ArcoroxCharacter->GetActorRotation().Yaw;
		TIPCharacterRotationYawLastFrame = TIPCharacterRotationYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		TIPCharacterRotationYawLastFrame = TIPCharacterRotationYaw;
		TIPCharacterRotationYaw = ArcoroxCharacter->GetActorRotation().Yaw;
		const float TIPDeltaYaw{ TIPCharacterRotationYaw - TIPCharacterRotationYawLastFrame };
		//Clamp RootYawOffset between [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPDeltaYaw);
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
}

void UArcoroxAnimInstance::Lean(float DeltaTime)
{
	if (ArcoroxCharacter == nullptr || ArcoroxCharacterMovement == nullptr) return;
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ArcoroxCharacter->GetActorRotation();
	const FRotator DeltaRotation{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };
	const float Target = DeltaRotation.Yaw / DeltaTime;
	const float Interpolation{ FMath::FInterpTo<float>(DeltaYaw, Target, DeltaTime, 1.f) };
	DeltaYaw = FMath::Clamp(Interpolation, -85.f, 85.f);
}
