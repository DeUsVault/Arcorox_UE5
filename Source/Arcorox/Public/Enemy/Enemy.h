// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Enemy.generated.h"

class UParticleSystem;
class UAnimMontage;

UCLASS()
class ARCOROX_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Hit_Implementation(FHitResult HitResult) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitDamage(int32 Damage, FVector HitLocation, bool bHeadshot);

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	UFUNCTION(BlueprintCallable)
	void StoreHitDamage(UUserWidget* Widget, FVector Location);

	UFUNCTION()
	void DestroyHitDamage(UUserWidget* HitDamage);

	void ShowHealthBar_Implementation();

	/* Called when Health reaches 0 */
	void Die();

	void ResetHitReactTimer();

	void UpdateHitDamages();

private:	
	void PlayImpactSound();
	void SpawnImpactParticles(FHitResult& HitResult);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName, float PlayRate);
	void PlayHitMontage(FHitResult& HitResult, float PlayRate = 1.f);

	/* Current health of enemy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	/* Maximum health value of enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	/* Particles spawned when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	/* Sound played when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USoundBase* ImpactSound;

	/* Name of head bone on skeleton for headshot damage functionality */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FString HeadBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	/* How long Health Bar should be displayed when enemy is hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HealthBarDisplayTime;

	/* Minimum hit react delay time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MinHitReactTime;

	/* Maximum hit react delay time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHitReactTime;

	/* Map of Hit Damage widgets and their locations */
	UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> HitDamages;

	/* How long Hit Damage widgets will persist on screen */
	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitDamageDestroyTime;

	/* FTimerHandle for displaying the Health Bar */
	FTimerHandle HealthBarDisplayTimer;

	/* FTimerHandle for hit react delay */
	FTimerHandle HitReactTimer;

	/* Can the Hit React montage be played */
	bool bCanHitReact;
};
