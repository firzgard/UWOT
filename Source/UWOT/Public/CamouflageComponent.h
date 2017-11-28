// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CamouflageComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCamouflageChangeDelegate, float, alpha);

UCLASS( ClassGroup=(UWOT), meta=(BlueprintSpawnableComponent) )
class UWOT_API UCamouflageComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Camouflage")
		float EnCamouflageTime = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Camouflage")
		float DeCamouflageTime = .3f;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Camouflage")
		float CamouflageRemainDuration = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Camouflage")
		float CamouflageDuration = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Camouflage")
		float CamouflageFullRecoverTime = 15;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Camouflage")
		bool bCamouflage = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Camouflage")
		float CamouflageVisibility = 1;
	UPROPERTY(BlueprintAssignable, Category = "Components|Camouflage")
		FCamouflageChangeDelegate OnChangeCamouflage;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

	UFUNCTION(BlueprintCallable)
		bool TrySetCamouflage(bool bEnable);

public:	
	UCamouflageComponent();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Components|Camouflage"))
		void ReceiveChangeCamouflage(float visibility);
	
};
