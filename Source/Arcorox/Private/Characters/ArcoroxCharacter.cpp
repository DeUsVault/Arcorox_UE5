// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ArcoroxCharacter.h"
#include "Items/Item.h"
#include "Items/Weapon.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"

AArcoroxCharacter::AArcoroxCharacter() :
	//Is Aiming
	bAiming(false),
	//Camera field of view
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpolationSpeed(25.f),
	//Look sensitivity
	LookScale(1.f),
	HipLookScale(1.f),
	AimingLookScale(0.25f),
	//Crosshair spread
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	ShootTimeDuration(0.05f),
	bFiringWeapon(false),
	//Automatic weapon fire
	FireRate(0.1f),
	bShouldFire(true),
	bFireButtonPressed(false),
	bShouldTraceForItems(false)
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//Make Character Mesh move according to movement direction
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AArcoroxCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SetupEnhancedInput();

	if (GetCamera())
	{
		CameraDefaultFOV = GetCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	EquipWeapon(SpawnDefaultWeapon());
}

void AArcoroxCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraZoomInterpolation(DeltaTime);
	SetLookScale();
	CalculateCrosshairSpread(DeltaTime);

	ItemTrace();
}

void AArcoroxCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bind functions to specific Input Actions 
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AArcoroxCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AArcoroxCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AArcoroxCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &AArcoroxCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &AArcoroxCharacter::FireButtonReleased);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AArcoroxCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AArcoroxCharacter::AimButtonReleased);
	}
}

void AArcoroxCharacter::Jump()
{
	Super::Jump();
}

float AArcoroxCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AArcoroxCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	OverlappedItemCount += Amount;
	bShouldTraceForItems = OverlappedItemCount > 0;
}

void AArcoroxCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (GetController())
	{
		//Get Controller Rotation and use Rotation Matrix to get Controller Forward and Right Vectors
		const FRotator ControllerRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControllerRotation.Yaw, 0.f);
		const FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardVector, MovementVector.Y);
		AddMovementInput(RightVector, MovementVector.X);
	}
}

void AArcoroxCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerPitchInput(LookScale * LookAxisVector.Y);
		AddControllerYawInput(LookScale * LookAxisVector.X);
	}
}

void AArcoroxCharacter::FireWeapon()
{
	PlayFireSound();
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		FVector BeamEnd;
		SpawnMuzzleFlash(SocketTransform);
		if (GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd))
		{
			SpawnImpactParticles(BeamEnd);
			SpawnBeamParticles(SocketTransform, BeamEnd);
		}
	}
	PlayRandomMontageSection(HipFireMontage, HipFireMontageSections);
	StartCrosshairShootTimer();
}

void AArcoroxCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartAutoFireTimer();
}

void AArcoroxCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AArcoroxCharacter::AimButtonPressed()
{
	bAiming = true;
}

void AArcoroxCharacter::AimButtonReleased()
{
	bAiming = false;
}

bool AArcoroxCharacter::GetBeamEndLocation(const FVector& BarrelSocketLocation, FVector& OutBeamLocation)
{
	//Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	if (CrosshairLineTrace(CrosshairHitResult, OutBeamLocation))
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}
	//Perform line trace from weapon barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ BarrelSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - BarrelSocketLocation };
	const FVector WeaponTraceEnd{ BarrelSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (WeaponTraceHit.bBlockingHit) //Object between weapon barrel and beam end point?
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}

bool AArcoroxCharacter::CrosshairLineTrace(FHitResult& OutHit, FVector& OutHitLocation)
{
	//Get size of viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	//Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition, CrosshairWorldDirection;
	//Get crosshairs world position and direction
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld) //was deprojection successful
	{
		//Trace outward from crosshair location
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50000 };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (OutHit.bBlockingHit)
		{
			OutHitLocation = OutHit.Location;
			return true;
		}
	}
	return false;
}

void AArcoroxCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	//Calculate CrosshairVelocityFactor
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	//Calculate CrosshairInAirFactor
	if (GetCharacterMovement()->IsFalling()) CrosshairInAirFactor = FMath::FInterpTo<float>(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	else CrosshairInAirFactor = FMath::FInterpTo<float>(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	//Calculate CrosshairAimFactor
	if (bAiming) CrosshairAimFactor = FMath::FInterpTo<float>(CrosshairAimFactor, -0.6f, DeltaTime, 30.f);
	else CrosshairAimFactor = FMath::FInterpTo<float>(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	//Calculate CrosshairShootFactor
	if (bFiringWeapon) CrosshairShootingFactor = FMath::FInterpTo<float>(CrosshairShootingFactor, 0.4f, DeltaTime, 60.f);
	else CrosshairShootingFactor = FMath::FInterpTo<float>(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CrosshairShootingFactor;
}

void AArcoroxCharacter::ItemTrace()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		if (CrosshairLineTrace(ItemTraceResult, HitLocation))
		{
			AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (HitItem)
			{
				HitItem->ShowPickupWidget();
			}

			if (TraceHitItem)
			{
				if (HitItem != TraceHitItem) TraceHitItem->HidePickupWidget();
			}
			TraceHitItem = HitItem;
		}
	}
	else if (TraceHitItem)
	{
		TraceHitItem->HidePickupWidget();
		TraceHitItem = nullptr;
	}
}

void AArcoroxCharacter::StartCrosshairShootTimer()
{
	bFiringWeapon = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AArcoroxCharacter::FinishedCrosshairShootTimer, ShootTimeDuration);
}

void AArcoroxCharacter::StartAutoFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AArcoroxCharacter::AutoFireReset, FireRate);
	}
}

AWeapon* AArcoroxCharacter::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && DefaultWeaponClass) return World->SpawnActor<AWeapon>(DefaultWeaponClass);
	return nullptr;
}

void AArcoroxCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		Weapon->DisableSphereCollision();
		Weapon->DisableBoxCollision();
		const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(FName("WeaponSocket"));
		if (WeaponSocket)
		{
			WeaponSocket->AttachActor(Weapon, GetMesh());
			EquippedWeapon = Weapon;
		}
	}
}

void AArcoroxCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed) StartAutoFireTimer();
}

void AArcoroxCharacter::FinishedCrosshairShootTimer()
{
	bFiringWeapon = false;
}

void AArcoroxCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AArcoroxCharacter::SpawnMuzzleFlash(const FTransform& SocketTransform)
{
	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
	}
}

void AArcoroxCharacter::SpawnImpactParticles(const FVector& BeamEnd)
{
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
	}
}

void AArcoroxCharacter::SpawnBeamParticles(const FTransform& SocketTransform, const FVector& BeamEnd)
{
	if (BeamParticles)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
		if (Beam)
		{
			Beam->SetVectorParameter(FName("Target"), BeamEnd);
		}
	}
}

void AArcoroxCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

void AArcoroxCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return;
	const int32 Section = FMath::RandRange(0, SectionNames.Num() - 1);
	PlayMontageSection(Montage, SectionNames[Section]);
}

void AArcoroxCharacter::CameraZoomInterpolation(float DeltaTime)
{
	//Smoothly transition the current camera field of view
	if (bAiming)
	{
		//Interpolate to zoomed field of view
		CameraCurrentFOV = FMath::FInterpTo<float>(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpolationSpeed);
	}
	else
	{
		//Interpolate to default field of view
		CameraCurrentFOV = FMath::FInterpTo<float>(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpolationSpeed);
	}
	if (GetCamera()) GetCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AArcoroxCharacter::SetupEnhancedInput()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		//Link Input Mapping Context to Character Player Controller
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ArcoroxContext, 0);
		}
	}
}

void AArcoroxCharacter::SetLookScale()
{
	if (bAiming) LookScale = AimingLookScale;
	else LookScale = HipLookScale;
}