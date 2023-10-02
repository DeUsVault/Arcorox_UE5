// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ArcoroxPlayerController.h"
#include "Blueprint/UserWidget.h"

AArcoroxPlayerController::AArcoroxPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;


}

void AArcoroxPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void AArcoroxPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}
