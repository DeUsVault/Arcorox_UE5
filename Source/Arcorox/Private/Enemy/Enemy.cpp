// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Enemy/EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Characters/ArcoroxCharacter.h"

AEnemy::AEnemy() :
	Health(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(5.f),
	bCanHitReact(true),
	MinHitReactTime(0.5f),
	MaxHitReactTime(0.8f),
	HitDamageDestroyTime(1.5f),
	bStunned(false),
	StunChance(0.5f)
{
	PrimaryActorTick.bCanEverTick = true;

	AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphere"));
	AggroSphere->SetupAttachment(GetRootComponent());
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (AggroSphere) AggroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AggroSphereOverlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	EnemyController = Cast<AEnemyController>(GetController());
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
	if (EnemyController && EnemyController->GetBlackboardComponent() && BehaviorTree)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
		EnemyController->RunBehaviorTree(BehaviorTree);
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitDamages();
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::StoreHitDamage(UUserWidget* Widget, FVector Location)
{
	HitDamages.Add(Widget, Location);
	FTimerHandle HitDamageTimer;
	FTimerDelegate HitDamageDelegate;
	HitDamageDelegate.BindUFunction(this, FName("DestroyHitDamage"), Widget);
	GetWorldTimerManager().SetTimer(HitDamageTimer, HitDamageDelegate, HitDamageDestroyTime, false);
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if (EnemyController && EnemyController->GetBlackboardComponent())
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), bStunned);
	}
}

void AEnemy::DestroyHitDamage(UUserWidget* HitDamage)
{
	HitDamages.Remove(HitDamage);
	HitDamage->RemoveFromParent();
}

void AEnemy::AggroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	AArcoroxCharacter* ArcoroxCharacter = Cast<AArcoroxCharacter>(OtherActor);
	if (ArcoroxCharacter && EnemyController && EnemyController->GetBlackboardComponent())
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), ArcoroxCharacter);
	}
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarDisplayTimer);
	GetWorldTimerManager().SetTimer(HealthBarDisplayTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
	HideHealthBar();
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::UpdateHitDamages()
{
	for (auto& HitPair : HitDamages)
	{
		UUserWidget* HitDamage = HitPair.Key;
		const FVector HitLocation = HitPair.Value;
		FVector2D ScreenPosition;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), HitLocation, ScreenPosition);
		HitDamage->SetPositionInViewport(ScreenPosition);
	}
}

void AEnemy::PlayImpactSound()
{
	if (ImpactSound) UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
}

void AEnemy::SpawnImpactParticles(FHitResult& HitResult)
{
	if (ImpactParticles) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location);
}

void AEnemy::PlayHitMontage(FHitResult& HitResult, float PlayRate)
{
	if (!bCanHitReact) return;
	const FVector Forward = GetActorForwardVector();
	const FVector Right = GetActorRightVector();
	FVector Diff = HitResult.Location - GetActorLocation();
	Diff.Normalize();
	const FVector HitLocation = Diff;
	const float ForwardDotProduct = FVector::DotProduct(Forward, HitLocation);
	const float RightDotProduct = FVector::DotProduct(Right, HitLocation);
	FName SectionName = FName("HitReactFront");
	if (ForwardDotProduct >= 0.5f && ForwardDotProduct <= 1.f) SectionName = FName("HitReactFront");
	else if (ForwardDotProduct >= -1.f && ForwardDotProduct <= -0.5f) SectionName = FName("HitReactBack");
	else if (RightDotProduct >= 0.f && RightDotProduct <= 1.f) SectionName = FName("HitReactRight");
	else if (RightDotProduct >= -1.f && RightDotProduct <= -0.f) SectionName = FName("HitReactLeft");
	PlayMontageSection(HitMontage, SectionName, PlayRate);
	bCanHitReact = false;
	GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, FMath::FRandRange(MinHitReactTime, MaxHitReactTime));
}

void AEnemy::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage, PlayRate);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

void AEnemy::Hit_Implementation(FHitResult HitResult)
{
	PlayImpactSound();
	SpawnImpactParticles(HitResult);
	ShowHealthBar();
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		PlayHitMontage(HitResult);
		SetStunned(true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else Health -= DamageAmount;
	return DamageAmount;
}

