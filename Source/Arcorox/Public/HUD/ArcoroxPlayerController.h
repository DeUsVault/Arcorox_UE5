// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArcoroxPlayerController.generated.h"

UCLASS()
class ARCOROX_API AArcoroxPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AArcoroxPlayerController();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:


};
