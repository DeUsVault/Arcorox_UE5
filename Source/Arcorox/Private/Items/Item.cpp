// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "Characters/ArcoroxCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

AItem::AItem():
	ItemName(FString("Item"))
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

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(GetRootComponent());
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	HidePickupWidget();

	//Setup overlap for sphere component
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AArcoroxCharacter* ArcoroxCharacter = Cast<AArcoroxCharacter>(OtherActor);
		if (ArcoroxCharacter)ArcoroxCharacter->IncrementOverlappedItemCount(1);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AArcoroxCharacter* ArcoroxCharacter = Cast<AArcoroxCharacter>(OtherActor);
		if (ArcoroxCharacter) ArcoroxCharacter->IncrementOverlappedItemCount(-1);
	}
}

void AItem::ShowPickupWidget()
{
	if(PickupWidget) PickupWidget->SetVisibility(true);
}

void AItem::HidePickupWidget()
{
	if(PickupWidget) PickupWidget->SetVisibility(false);
}