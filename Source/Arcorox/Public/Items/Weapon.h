// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Items/AmmoType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class ARCOROX_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;

	/* Adds impulse force to weapon */
	void ThrowWeapon();

	/* Decrements ammo for the weapon */
	void DecrementAmmo();

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

protected:
	virtual void BeginPlay() override;

	void StopFalling();

private:
	/* Ammo count for the Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	/* Specific type of weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bIsFalling;

};
