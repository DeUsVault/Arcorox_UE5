// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	HidePickupWidget();
}

void AItem::ShowPickupWidget()
{
	if(PickupWidget) PickupWidget->SetVisibility(true);
}

void AItem::HidePickupWidget()
{
	if(PickupWidget) PickupWidget->SetVisibility(false);
}