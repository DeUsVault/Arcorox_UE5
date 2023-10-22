// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon.h"
#include "Items/Weapon.h"

AWeapon::AWeapon():
	ThrowWeaponTime(0.7f),
	bIsFalling(false),
	Ammo(30),
	MagazineCapacity(30),
	WeaponType(EWeaponType::EWT_SubmachineGun),
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSection(FName(TEXT("Reload SMG"))),
	ClipBoneName(FName(TEXT("smg_clip")))
{
	PrimaryActorTick.bCanEverTick = true;


}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetItemState() == EItemState::EIS_Falling && bIsFalling)
	{
		const FRotator Rotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
		GetItemMesh()->SetWorldRotation(Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	SetItemType(EItemType::EIT_Weapon);
}

void AWeapon::ThrowWeapon()
{
	FRotator Rotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	GetItemMesh()->SetWorldRotation(Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	const FVector ForwardVector{ GetItemMesh()->GetForwardVector() };
	const FVector RightVector{ GetItemMesh()->GetRightVector() };
	FVector ImpulseVector = RightVector.RotateAngleAxis(-20.f, ForwardVector);
	float RandomRotation{ FMath::FRandRange(10.f, 60.f) };
	ImpulseVector = ImpulseVector.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseVector *= 10000.f;
	GetItemMesh()->AddImpulse(ImpulseVector);
	bIsFalling = true;
	EnableGlowMaterial();
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::DecrementAmmo()
{
	if (Ammo - 1 < 0) Ammo = 0;
	else --Ammo;
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("Reload with more than magazine capacity"));
	Ammo += Amount;
}

bool AWeapon::FullMagazine()
{
	return Ammo == MagazineCapacity;
}

void AWeapon::StopFalling()
{
	bIsFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}
