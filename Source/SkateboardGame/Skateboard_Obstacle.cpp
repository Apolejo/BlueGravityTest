// Fill out your copyright notice in the Description page of Project Settings.


#include "Skateboard_Obstacle.h"
#include "Skateboard_Character.h"
#include "Components/BoxComponent.h"

// Sets default values
ASkateboard_Obstacle::ASkateboard_Obstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASkateboard_Obstacle::OnOverlapBegin);

}

// Called when the game starts or when spawned
void ASkateboard_Obstacle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASkateboard_Obstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASkateboard_Obstacle::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASkateboard_Character* Character = Cast<ASkateboard_Character>(OtherActor);
	if (Character)
	{
//		Character->IncrementScore(GetActorLocation());
	}
}

