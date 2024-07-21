// Copyright Epic Games, Inc. All Rights Reserved.

#include "Skateboard_Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Skateboard_Obstacle.h"



// Sets default values
ASkateboard_Character::ASkateboard_Character()
{



	// Initialize variables
	bIsChargingJump = false;
	JumpChargeTime = 0.0f;
	MaxJumpChargeTime = 2.0f; // Max time to charge jump (seconds)
	BaseJumpStrength = 500.0f;
	MaxJumpStrength = 1500.0f;
	DefaultMaxWalkSpeed = 600.0f; // Set the default max walk speed
	CurrentSpeed = 0.0f;
	MaxSpeed = 1200.0f; // Max speed of the character
	AccelerationRate = 400.0f; // Acceleration rate
	DecelerationRate = 200.0f; // Deceleration rate when no input is given
	BrakingRate = 600.0f; // Braking rate
	bIsAccelerating = false;
	bIsSteering = false;
	SteeringIntensity = 0.0f; // Degrees of rotation per second
	SteeringRate = 10.0f; // Rate at which steering intensity is applied



	// Set Size for collision Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);


	//Set Our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;


	//Dont Rotate when the controller rotates, just camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GravityScale = 2.0f; // Increase gravity for more realistic feel
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 0.0f; // Disable automatic braking


	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Register OnHit function for collision
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ASkateboard_Character::OnHit);

	//Score
	Score = 0;
	ScoredObstacles.Empty();

}



// Called when the game starts or when spawned
void ASkateboard_Character::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}



// Called every frame
void ASkateboard_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (bIsChargingJump)
	{
		JumpChargeTime += DeltaTime;
		JumpChargeTime = FMath::Clamp(JumpChargeTime, 0.0f, MaxJumpChargeTime);
		BPEvent_OnChargeJump(JumpChargeTime);

	}
	// Handle acceleration
	if (!bIsAccelerating)
	{
		// Handle deceleration when no input is provided
		CurrentSpeed = FMath::Clamp(CurrentSpeed - (DecelerationRate * DeltaTime), 0.0f, MaxSpeed);

	}

	if (GetCharacterMovement()->IsFalling())
	{
			PerformRaycast();
	}
	else if (GetCharacterMovement()->IsMovingOnGround())  // Clear the set when the character lands
	{
		ResetScoredObstacles();
	}

	// Apply the current speed
	AddMovementInput(GetActorForwardVector(), CurrentSpeed / MaxSpeed);

}

// Called to bind functionality to input
void ASkateboard_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASkateboard_Character::StartChargingJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASkateboard_Character::ReleaseJump);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateboard_Character::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASkateboard_Character::MoveStart);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateboard_Character::Look);

	}
}

void ASkateboard_Character::Move(const FInputActionValue& Value)
{
	FVector2D ConstantPower = Value.Get<FVector2d>();
	Steer(ConstantPower.X);
	Accelerate(ConstantPower.Y);


}

void ASkateboard_Character::MoveStart(const FInputActionValue& Value)
{
	ResetSpeeding();
}

void ASkateboard_Character::Accelerate(float Value)
{
	if (Value > 0.0f)
	{
		StartSpeeding();
		bIsAccelerating = true;
		CurrentSpeed = FMath::Clamp(CurrentSpeed + (AccelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
	}
	else
	{
		Brake(Value);
	}
	if(Value == 0.0f)
	{
		StopAccelerate(Value);
	}
	AddMovementInput(GetActorForwardVector(), CurrentSpeed / MaxSpeed);
}

void ASkateboard_Character::StopAccelerate(float Value)
{
	bIsAccelerating = false;
	CurrentSpeed = FMath::Clamp(CurrentSpeed - (DecelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
}

void ASkateboard_Character::Brake(float Value)
{
	// Gradually decrease speed
	CurrentSpeed = FMath::Clamp(CurrentSpeed - (BrakingRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
	bIsAccelerating = false;
}


void ASkateboard_Character::Steer(float Value)
{
	SteeringIntensity = Value * 45.0f;
	// Smoothly rotate to the target rotation
	FRotator CurrentRotation = GetActorRotation();

	if (CurrentSpeed == 0.f)
	{
		bIsSteering = false;
		CurrentRotation.Yaw += SteeringIntensity * SteeringRate * GetWorld()->GetDeltaSeconds();
		UE_LOG(LogTemp, Warning, TEXT("Rotate %f"), SteeringIntensity);
		
	}
	else
	{
		bIsSteering = true;
		CurrentRotation.Yaw += SteeringIntensity * SteeringRate * GetWorld()->GetDeltaSeconds();
	}

	SetActorRotation(CurrentRotation);
	// Apply steering effect

}

void ASkateboard_Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASkateboard_Character::StartChargingJump(const FInputActionValue& Value)
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bIsChargingJump = true;
		JumpChargeTime = 0.0f;
	}
}

void ASkateboard_Character::ReleaseJump(const FInputActionValue& Value)
{
	if (bIsChargingJump)
	{
		bIsChargingJump = false;

		float ChargeRatio = JumpChargeTime / MaxJumpChargeTime;
		float JumpStrength = FMath::Lerp(BaseJumpStrength, MaxJumpStrength, ChargeRatio);

		FVector JumpForce = FVector(0, 0, 1) * JumpStrength;
		GetCharacterMovement()->AddImpulse(JumpForce, true);

		JumpChargeTime = 0.0f; // Reset charge time
	}
	BPEvent_OnChargeJump(JumpChargeTime);
}


//Blueprint Events for montage
void ASkateboard_Character::ResetSpeeding_Implementation()
{
}

void ASkateboard_Character::StartSpeeding_Implementation()
{
}







void ASkateboard_Character::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Reset speed on collision
	CurrentSpeed = DefaultMaxWalkSpeed;

	// Additional collision handling logic can be added here
	UE_LOG(LogTemp, Warning, TEXT("Hit detected, speed reset to default."));
}


//Obstacles

void ASkateboard_Character::PerformRaycast()
{
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 300);  // Raycast down 300 units

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor && !ScoredObstacles.Contains(HitActor) && HitActor->IsA(ASkateboard_Obstacle::StaticClass()))
		{
			ScoredObstacles.Add(HitActor);
			IncrementScore(HitActor);
		}
	}

	// Optional: Draw the debug line for visual feedback (in editor)
//	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
}

void ASkateboard_Character::IncrementScore(AActor* ActorHit)
{
	Score++;
	BPEvent_OnSpawnScore(ActorHit->GetActorLocation(), Score);
	UE_LOG(LogTemp, Warning, TEXT("Score: %d"), Score);

}












void ASkateboard_Character::ResetScoredObstacles()
{
//	ScoredObstacles.Empty();
}



