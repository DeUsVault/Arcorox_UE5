// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ArcoroxCharacter.generated.h"

//Forward declarations to avoid including unnecessary header files
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UParticleSystem;
class UAnimMontage;

UCLASS()
class ARCOROX_API AArcoroxCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AArcoroxCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE bool IsAiming() const { return bAiming; }
protected:
	virtual void BeginPlay() override;

	/* Input callback functions */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void FireWeapon();
	void AimButtonPressed();
	void AimButtonReleased();

	bool GetBeamEndLocation(const FVector& BarrelSocketLocation, FVector& OutBeamLocation);
	void WeaponLineTrace(const FVector& BarrelSocketLocation, FVector& OutBeamLocation);
	void CrosshairLineTrace(FVector& CrosshairWorldPosition, FVector& CrosshairWorldDirection, FVector& OutBeamLocation);

	/* Enhanced Input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* ArcoroxContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* FireWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimAction;

private:	
	void PlayFireSound();
	void SpawnMuzzleFlash(const FTransform& SocketTransform);
	void SpawnImpactParticles(const FVector& BeamEnd);
	void SpawnBeamParticles(const FTransform& SocketTransform, const FVector& BeamEnd);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	void PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);
	void CameraZoomInterpolation(float DeltaTime);
	void SetupEnhancedInput();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> HipFireMontageSections;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpolationSpeed;

	float CameraDefaultFOV;
	float CameraZoomedFOV;
	float CameraCurrentFOV;

};
