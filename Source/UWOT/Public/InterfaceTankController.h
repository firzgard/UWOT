#pragma once

#include "Tank.h"

#include "CoreMinimal.h"
#include "Interface.h"

#include "InterfaceTankController.generated.h"

UENUM(BlueprintType)
enum class ETankTeamEnum : uint8
{
	NONE,
	TEAM_1,
	TEAM_2,
	TEAM_3,
	TEAM_4
};

UINTERFACE(BlueprintType)
class UWOT_API UTankController : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class UWOT_API ITankController
{
	GENERATED_IINTERFACE_BODY()

public:
	virtual ETankTeamEnum GetTeamId() = 0;
	virtual ATank * GetControlledTank() = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spotting")
		bool OnSpottedSelf(bool bSpotted);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spotting")
		bool OnSpottedOther(bool bSpotted, ATank * other);
};