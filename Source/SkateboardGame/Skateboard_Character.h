// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Skateboard_Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(config = Game)
class SKATEBOARDGAME_API ASkateboard_Character : public ACharacter
{
	GENERATED_BODY()

	//Input Actions

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;




public:
	// Sets default values for this character's properties
	ASkateboard_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	/** Called for movement input */
	UFUNCTION()
	void Steer(float Value);
	UFUNCTION()
	void Accelerate(float Value);
	UFUNCTION()
	void StopAccelerate(float Value);
	UFUNCTION()
	void Brake(float Value);

	//Movement
	void StartChargingJump(const FInputActionValue& Value);
	void ReleaseJump(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void MoveStart(const FInputActionValue& Value);

	bool bIsChargingJump;
	float JumpChargeTime;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Skate, meta = (AllowPrivateAccess = "true"))
	float BaseJumpStrength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Skate, meta = (AllowPrivateAccess = "true"))
	float MaxJumpChargeTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Skate, meta = (AllowPrivateAccess = "true"))
	float MaxJumpStrength;
//Acceleration Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate Movement", meta = (AllowPrivateAccess = "true"))
	float AccelerationRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate Movement", meta = (AllowPrivateAccess = "true"))
	float DecelerationRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate Movement", meta = (AllowPrivateAccess = "true"))
	float BrakingRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate Movement", meta = (AllowPrivateAccess = "true"))
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate Movement", meta = (AllowPrivateAccess = "true"))
	bool StartMovement;

	// Steering management
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate Movement", meta = (AllowPrivateAccess = "true"))
	float SteeringRate;


private:


	// Speed management
	float DefaultMaxWalkSpeed;
	float CurrentSpeed;
	bool bIsAccelerating;
	bool bIsSteering;


	float BaseTurnRate = 45.f;
	float BaseLookUpRate = 45.f;

	float SteeringIntensity;


public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	//Call animation in blueprint when starts moving
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animations")
	void StartSpeeding();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animations")
	void ResetSpeeding();

private: 
	
	TSet<AActor*> ScoredObstacles;
	void PerformRaycast();
	void ResetScoredObstacles();
	void IncrementScore(AActor* ActorHit);


public:

	int32 Score;

public:
	//UI
	UFUNCTION(BlueprintImplementableEvent, Category = "Scoring", meta = (DisplayName = "SpawnScore"))
	void BPEvent_OnSpawnScore(FVector HitLocation, int32 ScoreTotal);
	UFUNCTION(BlueprintImplementableEvent, Category = "Charge", meta = (DisplayName = "ChargeJump"))
	void BPEvent_OnChargeJump(float Charge);





};
