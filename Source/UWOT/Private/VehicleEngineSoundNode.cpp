// Fill out your copyright notice in the Description page of Project Settings.

#include "VehicleEngineSoundNode.h"

#include "ActiveSound.h"


TMap<uint32, UVehicleEngineSoundNode::FVehicleDesiredRPM> UVehicleEngineSoundNode::DesiredRPMDict;

UVehicleEngineSoundNode::UVehicleEngineSoundNode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

void UVehicleEngineSoundNode::ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances)
{
	auto updatedParams = ParseParams;
	StoreCurrentRPM(AudioDevice, ActiveSound, ParseParams, CurrentMaxRPM);

	for (auto childNodeId = 0; childNodeId < ChildNodes.Num(); childNodeId++)
	{
		if (ChildNodes[childNodeId])
		{
			const auto fadeInRpmMin = EngineSoundSamples[childNodeId].FadeInRPMStart;
			const auto fadeInRpmMax = EngineSoundSamples[childNodeId].FadeInRPMEnd;
			const auto fadeOutRpmMin = EngineSoundSamples[childNodeId].FadeOutRPMStart;
			const auto fadeOutRpmMax = EngineSoundSamples[childNodeId].FadeOutRPMEnd;

			CurrentMaxRPM = FMath::Max(CurrentMaxRPM, fadeOutRpmMax);

			const auto rpmAlpha = (CurrentRPM - fadeInRpmMin) / (fadeOutRpmMax - fadeInRpmMin);

			auto pitchToSet = FMath::Lerp(1.0f, EngineSoundSamples[childNodeId].MaxPitchMultiplier, rpmAlpha);
			auto volumeToSet = 1.0f;

			if (CurrentRPM >= fadeInRpmMin && CurrentRPM <= fadeInRpmMax && fadeInRpmMin != fadeInRpmMax)
			{
				volumeToSet = (CurrentRPM - fadeInRpmMin) / (fadeInRpmMax - fadeInRpmMin);
			}
			else if (CurrentRPM >= fadeOutRpmMin && CurrentRPM <= fadeOutRpmMax && fadeOutRpmMin != fadeOutRpmMax)
			{
				volumeToSet = 1.0f - (CurrentRPM - fadeOutRpmMin) / (fadeOutRpmMax - fadeOutRpmMin);
			}
			else if (CurrentRPM < fadeInRpmMax || CurrentRPM > fadeOutRpmMax)
			{
				volumeToSet = 0.0f;
				pitchToSet = 1.0f;
			}

			updatedParams.Volume = ParseParams.Volume * volumeToSet;
			updatedParams.Pitch = ParseParams.Pitch * pitchToSet;

			// "play" the rest of the tree
			ChildNodes[childNodeId]->ParseNodes(AudioDevice, GetNodeWaveInstanceHash(NodeWaveInstanceHash, ChildNodes[childNodeId], childNodeId), ActiveSound, updatedParams, WaveInstances);
		}
	}
}

void UVehicleEngineSoundNode::CreateStartingConnectors()
{
	InsertChildNode(ChildNodes.Num());
//	InsertChildNode(ChildNodes.Num());
}

void UVehicleEngineSoundNode::InsertChildNode(int32 Index)
{
	Super::InsertChildNode(Index);

	EngineSoundSamples.InsertZeroed(Index);
	EngineSoundSamples[Index].MaxPitchMultiplier = 1.0f;
}


void UVehicleEngineSoundNode::RemoveChildNode(int32 Index)
{
	Super::RemoveChildNode(Index);
	EngineSoundSamples.RemoveAt(Index);
}

#if WITH_EDITOR
	void UVehicleEngineSoundNode::SetChildNodes(TArray<USoundNode*>& InChildNodes)
	{
		Super::SetChildNodes(InChildNodes);

		// Keep the total sample count constant, add/remove extra when needed
		if (EngineSoundSamples.Num() < ChildNodes.Num())
		{
			const auto oldSize = EngineSoundSamples.Num();
			const auto numToAdd = ChildNodes.Num() - oldSize;
			EngineSoundSamples.AddZeroed(numToAdd);
			for (auto newIndex = oldSize; newIndex < EngineSoundSamples.Num(); ++newIndex)
			{
				EngineSoundSamples[newIndex].MaxPitchMultiplier = 1.0f;
			}
		}
		else if (EngineSoundSamples.Num() > ChildNodes.Num())
		{
			const auto numToRemove = EngineSoundSamples.Num() - ChildNodes.Num();
			EngineSoundSamples.RemoveAt(EngineSoundSamples.Num() - numToRemove, numToRemove);
		}
	}

#endif //WITH_EDITOR

void UVehicleEngineSoundNode::SetDesiredRPM(int32 playerId, FVehicleDesiredRPM desiredRPM)
{
	FAudioThread::RunCommandOnAudioThread([playerId, desiredRPM]()
	{
		DesiredRPMDict.Add(playerId, desiredRPM);
	});
}

void UVehicleEngineSoundNode::RemoveDesiredRPM(int32 playerId)
{
	FAudioThread::RunCommandOnAudioThread([playerId]()
	{
		DesiredRPMDict.Remove(playerId);
	});
}

void UVehicleEngineSoundNode::StoreCurrentRPM(FAudioDevice* audioDevice, FActiveSound& activeSound, const FSoundParseParameters& parseParams, float maxRPM)
{
	if (auto desiredRpm = DesiredRPMDict.Find(activeSound.GetOwnerID()))
	{
		CurrentRPMStoreTime = desiredRpm->TimeStamp;
		const auto deltaTime = desiredRpm->TimeStamp - CurrentRPMStoreTime;

		CurrentRPM = FMath::FInterpTo(CurrentRPM, FMath::Min(desiredRpm->DesiredRPM, maxRPM), deltaTime, 10.0f);
	}
	else
	{
		UE_LOG(LogAudio, Warning, TEXT("SoundNodeVehicleEngine node being used for Owner '%s' which has not cached desired RPM."), *activeSound.GetOwnerName());

		CurrentRPMStoreTime = 0.f;
	}
}