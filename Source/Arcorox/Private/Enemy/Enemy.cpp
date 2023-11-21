// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::PlayImpactSound()
{
	if (ImpactSound) UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
}

void AEnemy::SpawnImpactParticles(FHitResult& HitResult)
{
	if (ImpactParticles) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location);
}

void AEnemy::Hit_Implementation(FHitResult HitResult)
{
	PlayImpactSound();
	SpawnImpactParticles(HitResult);
}

