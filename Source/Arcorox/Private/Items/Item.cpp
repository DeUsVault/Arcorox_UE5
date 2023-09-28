// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "Characters/ArcoroxCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

AItem::AItem():
	ItemName(FString("Item")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup)
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetEnableGravity(false);
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
	SetActiveStars();

	//Setup overlap for sphere component
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	SetItemProperties(ItemState);
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

void AItem::SetActiveStars()
{
	for (int32 i = 0; i < 6; i++) ActiveStars.Add(false);
	switch (ItemRarity)
	{
		case EItemRarity::EIR_Damaged:
			ActiveStars[1] = true;
			break;
		case EItemRarity::EIR_Common:
			for (int32 i = 1; i < 3; i++) ActiveStars[i] = true;
			break;
		case EItemRarity::EIR_Uncommon:
			for (int32 i = 1; i < 4; i++) ActiveStars[i] = true;
			break;
		case EItemRarity::EIR_Rare:
			for (int32 i = 1; i < 5; i++) ActiveStars[i] = true;
			break;
		case EItemRarity::EIR_Legendary:
			for (int32 i = 1; i < 6; i++) ActiveStars[i] = true;
			break;
		default:
			for (int32 i = 1; i < 3; i++) ActiveStars[i] = true;
			break;
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
		case EItemState::EIS_Pickup:
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			DisableMeshCollision();
			OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
			OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			DisableBoxCollision();
			CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			break;
		case EItemState::EIS_Equipped:
			HidePickupWidget();
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			DisableMeshCollision();
			DisableSphereCollision();
			DisableBoxCollision();
			break;
		case EItemState::EIS_Falling:
			ItemMesh->SetSimulatePhysics(true);
			ItemMesh->SetEnableGravity(true);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
			DisableSphereCollision();
			DisableBoxCollision();
			break;
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

void AItem::DisableSphereCollision()
{
	if (OverlapSphere)
	{
		OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AItem::DisableBoxCollision()
{
	if (CollisionBox)
	{
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AItem::DisableMeshCollision()
{
	if (ItemMesh)
	{
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(ItemState);
}
