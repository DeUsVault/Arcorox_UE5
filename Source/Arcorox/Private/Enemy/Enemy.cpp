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
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
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
	StunChance(0.5f),
	bInAttackRange(false),
	WeaponDamage(20.f),
	LeftWeaponSocket(TEXT("FX_Trail_L_02")),
	RightWeaponSocket(TEXT("FX_Trail_R_02"))
{
	PrimaryActorTick.bCanEverTick = true;

	AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphere"));
	AggroSphere->SetupAttachment(GetRootComponent());

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(GetRootComponent());

	LeftWeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponBox"));
	LeftWeaponBox->SetupAttachment(GetMesh(), FName("LeftWeapon"));
	RightWeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponBox"));
	RightWeaponBox->SetupAttachment(GetMesh(), FName("RightWeapon"));
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (AggroSphere) AggroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AggroSphereOverlap);
	if (AttackRangeSphere)
	{
		AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AttackRangeSphereOverlap);
		AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AttackRangeSphereEndOverlap);
	}
	if (LeftWeaponBox)
	{
		LeftWeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::LeftWeaponOverlap);
		LeftWeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LeftWeaponBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		LeftWeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		LeftWeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	if (RightWeaponBox)
	{
		RightWeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::RightWeaponOverlap);
		RightWeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RightWeaponBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		RightWeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		RightWeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	}
	if (GetCapsuleComponent()) GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

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

void AEnemy::SetInAttackRange(bool InRange)
{
	bInAttackRange = InRange;
	if (EnemyController && EnemyController->GetBlackboardComponent())
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
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

void AEnemy::AttackRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	AArcoroxCharacter* ArcoroxCharacter = Cast<AArcoroxCharacter>(OtherActor);
	if (ArcoroxCharacter)
	{
		SetInAttackRange(true);
	}
}

void AEnemy::AttackRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;
	AArcoroxCharacter* ArcoroxCharacter = Cast<AArcoroxCharacter>(OtherActor);
	if (ArcoroxCharacter)
	{
		SetInAttackRange(false);
	}
}

void AEnemy::InflictDamage(AArcoroxCharacter* ArcoroxCharacter, const FName& WeaponSocket)
{
	if (ArcoroxCharacter == nullptr) return;
	UGameplayStatics::ApplyDamage(ArcoroxCharacter, WeaponDamage, EnemyController, this, UDamageType::StaticClass());
	ArcoroxCharacter->PlayMeleeImpactSound();
	ArcoroxCharacter->SpawnBloodParticles(GetMesh()->GetSocketTransform(WeaponSocket));
}

void AEnemy::LeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	AArcoroxCharacter* ArcoroxCharacter = Cast<AArcoroxCharacter>(OtherActor);
	if(ArcoroxCharacter) InflictDamage(ArcoroxCharacter, LeftWeaponSocket);
}

void AEnemy::RightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	AArcoroxCharacter* ArcoroxCharacter = Cast<AArcoroxCharacter>(OtherActor);
	if (ArcoroxCharacter) InflictDamage(ArcoroxCharacter, RightWeaponSocket);
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

void AEnemy::PlayAttackMontage(float PlayRate)
{
	PlayRandomMontageSection(AttackMontage, AttackMontageSections, PlayRate);
}

void AEnemy::ActivateLeftWeapon()
{
	if (LeftWeaponBox) LeftWeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
	if (LeftWeaponBox) LeftWeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	if (RightWeaponBox) RightWeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
	if (RightWeaponBox) RightWeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

void AEnemy::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames, float PlayRate)
{
	if (SectionNames.Num() <= 0) return;
	const int32 Section = FMath::FRandRange(0.f, SectionNames.Num() - 1);
	PlayMontageSection(Montage, SectionNames[Section], PlayRate);
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

