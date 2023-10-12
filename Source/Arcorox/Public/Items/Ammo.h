// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
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

protected:
	virtual void BeginPlay();

	/* Override of AItem::SetItemProperties(EItemState State) */
	virtual void SetItemProperties(EItemState State) override;

private:
	/* Ammo static mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo Properties", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;


};
