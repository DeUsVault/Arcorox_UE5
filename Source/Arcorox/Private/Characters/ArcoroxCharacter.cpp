// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ArcoroxCharacter.h"
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

AArcoroxCharacter::AArcoroxCharacter()
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
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AArcoroxCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		//Link Input Mapping Context to Character Player Controller
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) 
		{
			Subsystem->AddMappingContext(ArcoroxContext, 0);
		}
	}
}

void AArcoroxCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &AArcoroxCharacter::FireWeapon);
	}
}

void AArcoroxCharacter::Jump()
{
	Super::Jump();
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
		AddControllerPitchInput(LookAxisVector.Y);
		AddControllerYawInput(LookAxisVector.X);
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
}

bool AArcoroxCharacter::GetBeamEndLocation(const FVector& BarrelSocketLocation, FVector& OutBeamLocation)
{
	//Get size of viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	//Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition, CrosshairWorldDirection;
	//Get crosshairs world position and direction
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld) //was deprojection successful
	{
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50000 };

		//Set beam end point to line trace end point
		OutBeamLocation = End;

		//Trace outward from crosshairs location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit) //was there a trace hit
		{
			//Set beam end point to trace hit location
			OutBeamLocation = ScreenTraceHit.Location;
		}

		//Perform line trace from weapon barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ BarrelSocketLocation };
		const FVector WeaponTraceEnd{ OutBeamLocation };
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
		if (WeaponTraceHit.bBlockingHit) //Object between weapon barrel and beam end point?
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
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
