// Fill out your copyright notice in the Description page of Project Settings.

#include "TankSpottingManager.h"

#include "TankSpottingComponent.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

ATankSpottingManager* ATankSpottingManager::Instance;

void ATankSpottingManager::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	
	// Singleton checking
	if (!Instance || Instance == this)
	{
		Instance = this;

		PrimaryActorTick.bCanEverTick = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Another instance of ATankSpottingManager is already exist. This object will now be destroyed."));
		Destroy();
	}
}

void ATankSpottingManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if(Instance == this)
	{
		Instance = nullptr;
	}
}

void ATankSpottingManager::Tick(const float deltaTime)
{
	// Check timestamp to invoke spotting every frame for each spotting component registered
	const auto timeSec = GetWorld()->GetTimeSeconds();
	for (auto & teamEntry : SpottingTeamDict)
	{
		for (auto & memberEntry : teamEntry.Value.SpottingMemberDict)
		{
			// Invoke spotting
			if (timeSec > memberEntry.Value.SpottingTimeStamp)
			{
				InvokeSpottingTest(memberEntry.Key);

				// Register next timestamp for spotting invocation
				memberEntry.Value.SpottingTimeStamp = CalcRandomSpottingTimeStamp(memberEntry.Key->SpottingIntervalSec);
			}

			// Invoke despotting
			auto despottedEntries = TArray<ETankTeamEnum>();
			for (auto & deSpottingEntry : memberEntry.Value.DeSpottingTimeStampDict)
			{
				if (timeSec > deSpottingEntry.Value)
				{
					// Inform this member itself has been despotted from another team
					memberEntry.Key->OnSpottedSelf(deSpottingEntry.Key, false);

					// Inform the other team about this member being despotted
					for (auto & otherTeamMember : SpottingTeamDict[deSpottingEntry.Key].SpottingMemberDict)
					{
						otherTeamMember.Key->OnSpottedOther(memberEntry.Key, false);
					}
					
					despottedEntries.Add(deSpottingEntry.Key);
				}
			}

			// Remove despotted entry
			for (const auto key : despottedEntries)
			{
				memberEntry.Value.DeSpottingTimeStampDict.Remove(key);
			}
		}
	}

	Super::Tick(deltaTime);
}

void ATankSpottingManager::InvokeSpottingTest(UTankSpottingComponent* spotter)
{
	// Invoke spotting against all other registered members
	for (auto & otherTeamEntry : SpottingTeamDict)
	{
		// Prevent testing against teammate
		if(otherTeamEntry.Key == spotter->TeamId) continue;

		for (auto & otherMemberEntry : otherTeamEntry.Value.SpottingMemberDict)
		{
			if (DoSpottingTest(spotter, otherMemberEntry.Key))
			{
				// Register despotting timestamp for the spotted member
				otherMemberEntry.Value.DeSpottingTimeStampDict.Add(spotter->TeamId, SpottedDuration + GetWorld()->TimeSeconds);

				// Invoke on-spotting event for involved spotting members
				if (!otherMemberEntry.Key->bIsSpotted)
				{
					for (auto & teamMateEntry : SpottingTeamDict[spotter->TeamId].SpottingMemberDict)
					{
						teamMateEntry.Key->OnSpottedOther(otherMemberEntry.Key, true);
					}

					otherMemberEntry.Key->OnSpottedSelf(spotter->TeamId, true);
				}
			}
		}
	}
}

bool ATankSpottingManager::DoSpottingTest(UTankSpottingComponent* spotter, UTankSpottingComponent* target) const
{
	const auto effectiveSpottingRange = (1 - target->GetCurrentCamouflageFactor()) * spotter->BaseViewRange;
	const auto effectiveSpottingRangeSquared = effectiveSpottingRange * effectiveSpottingRange;
	const auto autoSpottingRangeSquared = spotter->AutoSpotRange * spotter->AutoSpotRange;

	for (const auto spottingPort : spotter->SpottingPorts)
	{
		const auto lineTraceStartLocation = spottingPort->GetComponentLocation();

		for (const auto detectionPort : target->DetectionPorts)
		{
			const auto distanceVector = detectionPort->GetComponentLocation() - lineTraceStartLocation;
			const auto distanceSquared = distanceVector.SizeSquared();

			// Check if within auto spotting range
			if (autoSpottingRangeSquared > distanceSquared) return true;

			// Check if within effective spotting range
			if (distanceSquared > effectiveSpottingRangeSquared) continue;

			const auto lineTraceEndLocation = lineTraceStartLocation + distanceVector.GetSafeNormal() * effectiveSpottingRange;

			FHitResult outHitresult;
			GetWorld()->LineTraceSingleByChannel(outHitresult
				, lineTraceStartLocation
				, lineTraceEndLocation
				, ECC_WorldDynamic);

			// Spotted!!
			if (outHitresult.GetActor() == target->GetOwner()) return true;
		}
	}

	return false;
}


ATankSpottingManager* ATankSpottingManager::GetInstance()
{
	if(!Instance)
	{
		Instance = GEngine->GameViewport->GetWorld()->SpawnActor<ATankSpottingManager>();
	}

	return Instance;
}

void ATankSpottingManager::SubscribeSpottingComponenent(UTankSpottingComponent * member)
{
	SpottingTeamDict.FindOrAdd(member->TeamId).SpottingMemberDict.Emplace(member, FSpottingEntry(CalcRandomSpottingTimeStamp(member->SpottingIntervalSec)));
}

void ATankSpottingManager::UnsubscribeSpottingComponenent(UTankSpottingComponent * member)
{
	if(auto team = SpottingTeamDict.Find(member->TeamId))
	{
		team->SpottingMemberDict.Remove(member);
	}
}
