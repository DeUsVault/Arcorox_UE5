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
class AItem;
class AWeapon;

UCLASS()
class ARCOROX_API AArcoroxCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AArcoroxCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const; 

	void IncrementOverlappedItemCount(int8 Amount);

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE bool IsAiming() const { return bAiming; }
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
protected:
	virtual void BeginPlay() override;

	/* Input callback functions */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void FireWeapon();
	void FireButtonPressed();
	void FireButtonReleased();
	void AimButtonPressed();
	void AimButtonReleased();
	void InteractButtonPressed();
	void InteractButtonReleased();

	bool GetBeamEndLocation(const FVector& BarrelSocketLocation, FVector& OutBeamLocation);
	/* Line trace for items behind the crosshairs*/
	bool CrosshairLineTrace(FHitResult& OutHit, FVector& OutHitLocation);
	void CalculateCrosshairSpread(float DeltaTime);

	/* Trace for Items if OverlappedItemCount > 0 */
	void ItemTrace();

	void StartCrosshairShootTimer();
	void StartAutoFireTimer();

	/* Spawn default weapon for character in BeginPlay */
	AWeapon* SpawnDefaultWeapon();

	/* Attach weapon to character's weapon socket */
	void EquipWeapon(AWeapon* Weapon);

	/* Detach weapon and have it fall to ground */
	void DropWeapon();

	UFUNCTION()
	void AutoFireReset();

	UFUNCTION()
	void FinishedCrosshairShootTimer();

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;

private:	
	void PlayFireSound();
	void SpawnMuzzleFlash(const FTransform& SocketTransform);
	void SpawnImpactParticles(const FVector& BeamEnd);
	void SpawnBeamParticles(const FTransform& SocketTransform, const FVector& BeamEnd);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	void PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);
	void CameraZoomInterpolation(float DeltaTime);
	void SetupEnhancedInput();
	void SetLookScale();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float LookScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float HipLookScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float AimingLookScale;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	/* Currently equipped weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	/* Default weapon class to spawn for character */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	//Camera field of view
	float CameraDefaultFOV;
	float CameraZoomedFOV;
	float CameraCurrentFOV;

	//Crosshair spread
	float ShootTimeDuration;
	bool bFiringWeapon;
	FTimerHandle CrosshairShootTimer;

	//Automatic Weapon Fire
	bool bFireButtonPressed;
	bool bShouldFire;
	float FireRate;
	FTimerHandle AutoFireTimer;

	bool bShouldTraceForItems;
	int8 OverlappedItemCount;
};
