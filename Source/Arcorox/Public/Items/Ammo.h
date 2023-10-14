// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Items/AmmoType.h"
#include "Ammo.generated.h"

class UStaticMeshComponent;

UCLASS()
class ARCOROX_API AAmmo : public AItem
{
	GENERATED_BODY()
	
public:
	AAmmo();
	virtual void Tick(float DeltaTime);

	void DisableAmmoMeshCollision();

	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

protected:
	virtual void BeginPlay();

	/* Override of AItem::SetItemProperties(EItemState State) */
	virtual void SetItemProperties(EItemState State) override;

private:
	/* Ammo static mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;

	/* Ammo type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

};
